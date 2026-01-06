#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include <cstdio>
#include <cstring>
#include <string>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

static bool SendAll(SOCKET s, const char* data, int len) {
    int sentTotal = 0;
    while (sentTotal < len) {
        int n = send(s, data + sentTotal, len - sentTotal, 0);
        if (n == SOCKET_ERROR || n == 0) return false;
        sentTotal += n;
    }
    return true;
}

static bool RecvSome(SOCKET s, std::string& out) {
    char buf[4096];
    int n = recv(s, buf, (int)sizeof(buf), 0);
    if (n == SOCKET_ERROR || n == 0) return false;
    out.assign(buf, buf + n);
    return true;
}

int main(int argc, char** argv) {
    const char* ip = "127.0.0.1";
    uint16_t port = 13000;

    if (argc >= 2) ip = argv[1];
    if (argc >= 3) port = (uint16_t)atoi(argv[2]);

    WSADATA wsa{};
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::printf("WSAStartup failed\n");
        return 1;
    }

    SOCKET sock = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, nullptr, 0, 0);
    if (sock == INVALID_SOCKET) {
        std::printf("socket failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &addr.sin_addr) != 1) {
        std::printf("inet_pton failed for ip=%s\n", ip);
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::printf("Connecting to %s:%u ...\n", ip, port);
    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) != 0) {
        std::printf("connect failed: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::printf("Connected. Type messages. Type 'quit' to exit.\n");

    while (true) {
        std::printf("> ");
        std::string line;
        if (!std::getline(std::cin, line)) break;

        if (line == "quit") break;

        // 서버에서 줄 단위로 보기 쉽게 \n 붙여서 보냄
        line.push_back('\n');

        if (!SendAll(sock, line.data(), (int)line.size())) {
            std::printf("send failed: %d\n", WSAGetLastError());
            break;
        }

        std::string echo;
        if (!RecvSome(sock, echo)) {
            std::printf("recv failed: %d\n", WSAGetLastError());
            break;
        }

        std::printf("echo: %s", echo.c_str());
        if (!echo.empty() && echo.back() != '\n') std::printf("\n");
    }

    closesocket(sock);
    WSACleanup();
    std::printf("Bye.\n");
    return 0;
}
