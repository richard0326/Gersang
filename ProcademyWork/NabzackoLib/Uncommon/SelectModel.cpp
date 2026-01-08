#include "stdafx.h"
#include "SelectModel.h"

stSession::stSession(SOCKET sock, sockaddr_in* pSockInfo)
{
    _socket = sock;
    memcpy(&_socketInfo, pSockInfo, sizeof(sockaddr_in));
    _pRecvBuffer = new CRingBuffer();
    _pSendBuffer = new CRingBuffer();
    _pUser = nullptr;
}

stSession::~stSession()
{
    delete _pRecvBuffer;
    delete _pSendBuffer;
}

CSelectServer::CSelectServer()
{

}

CSelectServer::~CSelectServer()
{

}

bool CSelectServer::Init(const wchar_t* ipWstr, int portNum)
{
    WSADATA wsa;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (iResult != NO_ERROR) {
        return false;
    }
        
    m_listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_listen_sock == INVALID_SOCKET) return false;
    
    ZeroMemory(&m_listeninfo, sizeof(m_listeninfo));
    m_listeninfo.sin_family = AF_INET;
    InetPton(AF_INET, ipWstr, &m_listeninfo.sin_addr);
    m_listeninfo.sin_port = htons(portNum);

    iResult = bind(m_listen_sock, (sockaddr*)&m_listeninfo, sizeof(m_listeninfo));
    if (iResult == SOCKET_ERROR) return false;

    iResult = listen(m_listen_sock, SOMAXCONN);
    if (iResult == SOCKET_ERROR) return false;

    u_long on = 1;
    int retval = ioctlsocket(m_listen_sock, FIONBIO, &on);
    if (retval == SOCKET_ERROR) return false;

    printf("Server Open...\n");

    return true;
}

void CSelectServer::Release()
{
    for (auto eraseIter = m_sessoinList.begin(); eraseIter != m_sessoinList.end();)
    {
        closesocket((*eraseIter)->_socket);
        delete (*eraseIter);
        eraseIter = m_sessoinList.erase(eraseIter);
    }

    if (m_addedSession != nullptr)
    {
        delete m_addedSession;
    }

    closesocket(m_listen_sock);

    // 윈속 종료
    WSACleanup();
}

bool CSelectServer::SelectLoop(
    bool (*AcceptFunc)(stSession* pSession, SOCKET socket, sockaddr_in* pSocketInfo), 
    void (*DeleteFunc)(stSession* pSession), 
    bool (*RecvFunc)(stSession* pSession))
{
    if (m_addedSession != nullptr)
    {
        m_sessoinList.push_back(m_addedSession);
        m_addedSession = nullptr;
    }

    if (m_sessoinList.empty())
    {
        FD_SET rset;
        FD_ZERO(&rset);
        FD_SET(m_listen_sock, &rset);

        timeval time = { 0,0 };
        int retval = select(0, &rset, NULL, NULL, &time);
        if (retval == SOCKET_ERROR)
        {
            printf("select() %d\n", WSAGetLastError());
            return false;
        }

        if (FD_ISSET(m_listen_sock, &rset))
        {
            SOCKET client_sock;
            sockaddr_in clientaddr;
            int addrlen = sizeof(clientaddr);
            client_sock = accept(m_listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
            if (client_sock == INVALID_SOCKET) {
                printf("accept() %d\n", WSAGetLastError());
                return false;
            }
            else {
                AddSession(AcceptFunc, client_sock, &clientaddr);
            }
        }
    }
    else
    {
        bool isListen = true;

        for (auto outerLoopIter = m_sessoinList.begin(); outerLoopIter != m_sessoinList.end();)
        {
            // 소켓 셋 초기화
            FD_SET rset, wset;
            FD_ZERO(&rset);
            FD_ZERO(&wset);

            int nSession = 0;
            if (isListen)
            {
                FD_SET(m_listen_sock, &rset);
                ++nSession;
            }

            for (auto innerLoopIter = outerLoopIter; innerLoopIter != m_sessoinList.end(); ++innerLoopIter)
            {
                if (nSession + 1 >= FD_SETSIZE)
                    break;

                stSession* session = (*innerLoopIter);
                FD_SET(session->_socket, &rset);
                FD_SET(session->_socket, &wset);
                ++nSession;
            }

            timeval time = { 0,0 };
            int retval = select(0, &rset, &wset, NULL, &time);
            if (retval == 0)
            {
                continue;
            }
            else if (retval == SOCKET_ERROR)
            {
                printf("select() %d\n", WSAGetLastError());
                return false;
            }

            // 소켓 셋 검사(1): 클라이언트 접속 수용
            if (isListen)
            {
                if (FD_ISSET(m_listen_sock, &rset))
                {
                    SOCKET client_sock;
                    sockaddr_in clientaddr;
                    int addrlen = sizeof(clientaddr);
                    client_sock = accept(m_listen_sock, (SOCKADDR*)&clientaddr, &addrlen);
                    if (client_sock == INVALID_SOCKET) {
                        printf("accept() %d\n", WSAGetLastError());
                        return false;
                    }
                    else {
                        AddSession(AcceptFunc, client_sock, &clientaddr);
                    }
                }
                --nSession;
            }

            // 소켓 셋 검사(2): 데이터 통신
            auto innerLoopIter = outerLoopIter;
            for (; innerLoopIter != m_sessoinList.end(); )
            {
                if (nSession < 0)
                    break;

                --nSession;
                stSession* pSession = (*innerLoopIter);
                if (FD_ISSET(pSession->_socket, &rset))
                {
                    // recvQ에 바로 받아버리기
                    retval = recv(pSession->_socket, pSession->_pRecvBuffer->GetRearBufferPtr(), pSession->_pRecvBuffer->DirectEnqueueSize(), 0);
                    if (retval == SOCKET_ERROR) {
                        DeleteSession(DeleteFunc, pSession);
                        innerLoopIter = m_sessoinList.erase(innerLoopIter);
                        continue;
                    }
                    else if (retval == 0) {
                        DeleteSession(DeleteFunc, pSession);
                        innerLoopIter = m_sessoinList.erase(innerLoopIter);
                        continue;
                    }

                    if (pSession->_pRecvBuffer->MoveRear(retval) == false)
                    {
                        printf("Move Rear Fail\n");
                        DeleteSession(DeleteFunc, pSession);
                        innerLoopIter = m_sessoinList.erase(innerLoopIter);
                        continue;
                    }

                    // recvQ의 값에서 패킷으로 처리 가능한 부분 처리하기
                    if (RecvFunc(pSession) == false)
                    {
                        printf("RecvFunc Error()\n");
                        DeleteSession(DeleteFunc, pSession);
                        innerLoopIter = m_sessoinList.erase(innerLoopIter);
                        continue;
                    }
                }

                if (FD_ISSET(pSession->_socket, &wset))
                {
                    if (pSession->_pSendBuffer->DirectDequeueSize() != 0)
                    {
                        // 데이터 보내기
                        retval = send(pSession->_socket, pSession->_pSendBuffer->GetFrontBufferPtr(), pSession->_pSendBuffer->DirectDequeueSize(), 0);
                        if (retval == SOCKET_ERROR) {
                            printf("send Error() %d\n", WSAGetLastError());
                            DeleteSession(DeleteFunc, pSession);
                            innerLoopIter = m_sessoinList.erase(innerLoopIter);
                            continue;
                        }

                        if (pSession->_pSendBuffer->MoveFront(retval) == false)
                        {
                            printf("MoveFront Error()\n");
                            DeleteSession(DeleteFunc, pSession);
                            innerLoopIter = m_sessoinList.erase(innerLoopIter);
                            continue;
                        }
                    }                    
                }

                ++innerLoopIter;
            }
            outerLoopIter = innerLoopIter;
        }
    }

    return true;
}

bool CSelectServer::AddSession(bool (*AcceptFunc)(stSession* pSession, SOCKET socket, sockaddr_in* pSocketInfo), SOCKET socket, sockaddr_in* psocketInfo)
{
    // 소켓 정보 추가
    m_addedSession = new stSession(socket, psocketInfo);
    
    // 난블락 소켓으로 전환
    u_long on = 1;
    int retval = ioctlsocket(socket, FIONBIO, &on);
    if (retval == SOCKET_ERROR)
    {
        delete m_addedSession;
        m_addedSession = nullptr;
    }
    else
    {
        return AcceptFunc(m_addedSession, socket, psocketInfo);
    }
    return false;
}

void CSelectServer::DeleteSession(void (*DeleteFunc)(stSession* pSession), stSession* pSession)
{
    if (pSession != nullptr)
    {
        DeleteFunc(pSession);

        closesocket(pSession->_socket);
        delete pSession;
    }
}