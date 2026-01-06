#include "stdafx.h"
#include "test.h"

#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include <atomic>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

// =========================
// Minimal IOCP TCP Server (Single-file)
// - accept() thread + IOCP workers
// - per-session recv overlapped
// - echo back
// =========================

static constexpr int RECV_BUF_SIZE = 8192;

enum class OpType : uint8_t { Recv, Send, Shutdown };

struct OverlappedEx {
    OVERLAPPED ol{};
    OpType type{};
    WSABUF wsaBuf{};
    char* buf = nullptr;      // points to owned buffer
    uint32_t cap = 0;         // buffer size
};

struct Session {
    uint64_t id = 0;
    SOCKET sock = INVALID_SOCKET;
    std::atomic<bool> closing{ false };

    // recv
    char recvBuf[RECV_BUF_SIZE];
    OverlappedEx recvOp{};

    // send (single in-flight only for simplicity)
    std::mutex sendMu;
    bool sendInFlight = false;
    OverlappedEx sendOp{};
    // send buffer owned here for simplicity
    std::vector<char> sendStorage;

    Session() {
        // recv overlapped init
        recvOp.type = OpType::Recv;
        recvOp.buf = recvBuf;
        recvOp.cap = RECV_BUF_SIZE;
        recvOp.wsaBuf.buf = recvOp.buf;
        recvOp.wsaBuf.len = recvOp.cap;

        // send overlapped init (buffer set at send time)
        sendOp.type = OpType::Send;
    }
};

struct Server {
    // sockets / iocp
    WSADATA wsa{};
    SOCKET listenSock = INVALID_SOCKET;
    HANDLE iocp = NULL;

    // threads
    std::vector<std::thread> workers;
    std::thread acceptThread;

    // state
    std::atomic<bool> running{ false };
    std::atomic<uint64_t> nextSessionId{ 1 };

    // sessions
    std::mutex sessionsMu;
    std::unordered_map<uint64_t, Session*> sessions;

    bool Start(const char* ip, uint16_t port, uint32_t workerCount) {
        if (running.load()) return true;

        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            std::printf("WSAStartup failed\n");
            return false;
        }

        listenSock = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, WSA_FLAG_OVERLAPPED);
        if (listenSock == INVALID_SOCKET) {
            std::printf("WSASocket listen failed: %lu\n", GetLastError());
            return false;
        }

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);

        if (std::strcmp(ip, "0.0.0.0") == 0) addr.sin_addr.s_addr = INADDR_ANY;
        else {
            if (inet_pton(AF_INET, ip, &addr.sin_addr) != 1) {
                std::printf("inet_pton failed\n");
                return false;
            }
        }

        int yes = 1;
        setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes));

        if (bind(listenSock, (sockaddr*)&addr, sizeof(addr)) != 0) {
            std::printf("bind failed: %lu\n", GetLastError());
            return false;
        }

        if (listen(listenSock, SOMAXCONN) != 0) {
            std::printf("listen failed: %lu\n", GetLastError());
            return false;
        }

        iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
        if (!iocp) {
            std::printf("CreateIoCompletionPort failed: %lu\n", GetLastError());
            return false;
        }

        running.store(true);

        // start workers
        if (workerCount == 0) {
            SYSTEM_INFO si{};
            GetSystemInfo(&si);
            workerCount = (si.dwNumberOfProcessors > 0) ? si.dwNumberOfProcessors : 4;
        }

        workers.reserve(workerCount);
        for (uint32_t i = 0; i < workerCount; ++i) {
            workers.emplace_back([this] { WorkerLoop(); });
        }

        // start accept thread
        acceptThread = std::thread([this] { AcceptLoop(); });

        std::printf("Server started: %s:%u  workers=%u\n", ip, port, workerCount);
        return true;
    }

    void Stop() {
        bool was = running.exchange(false);
        if (!was) return;

        // close listen to break accept()
        if (listenSock != INVALID_SOCKET) {
            closesocket(listenSock);
            listenSock = INVALID_SOCKET;
        }

        if (acceptThread.joinable()) acceptThread.join();

        // ask workers to shutdown
        for (size_t i = 0; i < workers.size(); ++i) {
            auto* op = new OverlappedEx();
            op->type = OpType::Shutdown;
            PostQueuedCompletionStatus(iocp, 0, 0, &op->ol);
        }

        for (auto& t : workers) {
            if (t.joinable()) t.join();
        }
        workers.clear();

        // cleanup sessions
        {
            std::lock_guard<std::mutex> lk(sessionsMu);
            for (auto& kv : sessions) {
                auto id = kv.first;
                auto* s = kv.second;
                if (s && s->sock != INVALID_SOCKET) closesocket(s->sock);
                delete s;
            }
            sessions.clear();
        }

        if (iocp) {
            CloseHandle(iocp);
            iocp = NULL;
        }

        WSACleanup();
        std::printf("Server stopped.\n");
    }

    // -------- internals --------

    void AcceptLoop() {
        while (running.load()) {
            sockaddr_in caddr{};
            int clen = sizeof(caddr);
            SOCKET cs = accept(listenSock, (sockaddr*)&caddr, &clen);
            if (cs == INVALID_SOCKET) {
                int err = WSAGetLastError();
                if (!running.load()) break; // stop path
                std::printf("accept failed: %d\n", err);
                continue;
            }

            // make socket overlapped-friendly (already OK), but ensure non-block not required
            auto* s = new Session();
            s->id = nextSessionId.fetch_add(1);
            s->sock = cs;

            // bind to IOCP with completion key = Session*
            if (!CreateIoCompletionPort((HANDLE)cs, iocp, (ULONG_PTR)s, 0)) {
                std::printf("Bind IOCP failed: %lu\n", GetLastError());
                closesocket(cs);
                delete s;
                continue;
            }

            {
                std::lock_guard<std::mutex> lk(sessionsMu);
                sessions[s->id] = s;
            }

            char ipbuf[64]{};
            inet_ntop(AF_INET, &caddr.sin_addr, ipbuf, sizeof(ipbuf));
            std::printf("Connect id=%llu from %s:%u\n",
                (unsigned long long)s->id, ipbuf, ntohs(caddr.sin_port));

            PostRecv(s);
        }
    }

    bool PostRecv(Session* s) {
        if (!s || s->closing.load()) return false;

        std::memset(&s->recvOp.ol, 0, sizeof(s->recvOp.ol));
        DWORD flags = 0;
        DWORD bytes = 0;

        int r = WSARecv(s->sock, &s->recvOp.wsaBuf, 1, &bytes, &flags, &s->recvOp.ol, nullptr);
        if (r == SOCKET_ERROR) {
            int err = WSAGetLastError();
            if (err != WSA_IO_PENDING) {
                CloseSession(s, err);
                return false;
            }
        }
        return true;
    }

    bool PostSend(Session* s, const char* data, int len) {
        if (!s || s->closing.load()) return false;
        if (len <= 0) return true;

        std::lock_guard<std::mutex> lk(s->sendMu);
        if (s->sendInFlight) {
            // 초단순 스켈레톤: 동시에 보내려는 요청은 버림(나중 단계에서 sendQueue로 개선)
            return false;
        }
        s->sendInFlight = true;

        // copy into owned storage
        s->sendStorage.assign(data, data + len);

        s->sendOp.buf = s->sendStorage.data();
        s->sendOp.cap = (uint32_t)s->sendStorage.size();
        s->sendOp.wsaBuf.buf = s->sendOp.buf;
        s->sendOp.wsaBuf.len = s->sendOp.cap;

        std::memset(&s->sendOp.ol, 0, sizeof(s->sendOp.ol));
        DWORD sent = 0;

        int r = WSASend(s->sock, &s->sendOp.wsaBuf, 1, &sent, 0, &s->sendOp.ol, nullptr);
        if (r == SOCKET_ERROR) {
            int err = WSAGetLastError();
            if (err != WSA_IO_PENDING) {
                s->sendInFlight = false;
                CloseSession(s, err);
                return false;
            }
        }

        return true;
    }

    void WorkerLoop() {
        while (true) {
            DWORD bytes = 0;
            ULONG_PTR key = 0;
            OVERLAPPED* pol = nullptr;

            BOOL ok = GetQueuedCompletionStatus(iocp, &bytes, &key, &pol, INFINITE);
            if (!pol) continue;

            auto* op = reinterpret_cast<OverlappedEx*>(pol);
            if (op->type == OpType::Shutdown) {
                delete op;
                break;
            }

            Session* s = reinterpret_cast<Session*>(key);
            if (!s) continue;

            if (op->type == OpType::Recv) {
                if (!ok || bytes == 0) {
                    CloseSession(s, ok ? 0 : WSAGetLastError());
                    continue;
                }

                // echo
                PostSend(s, s->recvBuf, (int)bytes);

                // next recv
                PostRecv(s);
            }
            else if (op->type == OpType::Send) {
                // send complete
                {
                    std::lock_guard<std::mutex> lk(s->sendMu);
                    s->sendInFlight = false;
                    s->sendStorage.clear();
                }

                if (!ok) {
                    CloseSession(s, WSAGetLastError());
                    continue;
                }
            }
        }
    }

    void CloseSession(Session* s, int reason) {
        if (!s) return;

        bool expected = false;
        if (!s->closing.compare_exchange_strong(expected, true)) return;

        uint64_t id = s->id;

        // remove from map first (avoid double-close)
        {
            std::lock_guard<std::mutex> lk(sessionsMu);
            auto it = sessions.find(id);
            if (it != sessions.end() && it->second == s) {
                sessions.erase(it);
            }
        }

        if (s->sock != INVALID_SOCKET) {
            closesocket(s->sock);
            s->sock = INVALID_SOCKET;
        }

        std::printf("Disconnect id=%llu reason=%d\n", (unsigned long long)id, reason);
        delete s;
    }
};

int main()
{
    Server srv;
    // NOTE: workerCount=0이면 CPU 코어 수로 자동
    if (!srv.Start("127.0.0.1", 13000, 0)) {
        std::printf("Start failed\n");
        return 1;
    }

    std::printf("Press Enter to stop...\n");
    (void)getchar();

    srv.Stop();
    return 0;
}