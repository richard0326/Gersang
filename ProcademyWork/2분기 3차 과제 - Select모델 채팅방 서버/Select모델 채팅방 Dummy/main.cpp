#include "stdafx.h"
#include "UserMgr.h"
#include "RoomMgr.h"
#include "Packet.h"

bool Init();
void Release();
bool ConnectSession(const wchar_t* ipWstr, int portNum);
void DisconnectSession(stSession* pSession);
bool SendPacket(stSession* pSession, int messageType, void* infoPtr = nullptr);
bool RecvPacket(stSession* pSession);

list<stSession*> m_sessoinList;

int main()
{
    WSADATA wsa;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (iResult != NO_ERROR) {
        return 0;
    }

    if (Init() == false)
    {
        return 0;
    }

    while (true)
    {
        for (auto outerLoopIter = m_sessoinList.begin(); outerLoopIter != m_sessoinList.end();)
        {
            // 소켓 셋 초기화
            FD_SET rset, wset;
            FD_ZERO(&rset);
            FD_ZERO(&wset);

            int nSession = 0;
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
                        DisconnectSession(pSession);
                        innerLoopIter = m_sessoinList.erase(innerLoopIter);
                        continue;
                    }
                    else if (retval == 0) {
                        DisconnectSession(pSession);
                        innerLoopIter = m_sessoinList.erase(innerLoopIter);
                        continue;
                    }

                    if (pSession->_pRecvBuffer->MoveRear(retval) == false)
                    {
                        printf("Move Rear Fail\n");
                        DisconnectSession(pSession);
                        innerLoopIter = m_sessoinList.erase(innerLoopIter);
                        continue;
                    }

                    // recvQ의 값에서 패킷으로 처리 가능한 부분 처리하기
                    if (RecvPacket(pSession) == false)
                    {
                        printf("RecvFunc Error()\n");
                        DisconnectSession(pSession);
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
                            DisconnectSession(pSession);
                            innerLoopIter = m_sessoinList.erase(innerLoopIter);
                            continue;
                        }

                        if (pSession->_pSendBuffer->MoveFront(retval) == false)
                        {
                            printf("MoveFront Error()\n");
                            DisconnectSession(pSession);
                            innerLoopIter = m_sessoinList.erase(innerLoopIter);
                            continue;
                        }
                    }
                }

                ++innerLoopIter;
            }
            outerLoopIter = innerLoopIter;
        }

        // 루프 마다 1초 대기
        Sleep(1000);
    }

    Release();

    // 윈속 종료
    WSACleanup();

	return 0;
}

bool Init()
{
    for (int i = 0; i < INIT_SESSION; ++i)
    {
        if (ConnectSession(L"127.0.0.1", dfNETWORK_PORT) == false)
        {
            return false;
        }
    }

    // 첫번째 세션에 테스트하기 위해 SendQueue에 값을 넣어보자.
    stSession* pSession = (*m_sessoinList.begin());

    wchar_t testID[15] = L"testID";
    if (SendPacket(pSession, df_REQ_LOGIN, testID) == false)
        return false;

    if (SendPacket(pSession, df_REQ_ROOM_LIST) == false)
        return false;

    CRoom* pRoom = SINGLETON(CRoomManager)->CreateRoom(1, (wchar_t*)L"sibal", wcslen(L"sibal") * sizeof(wchar_t));
    if (SendPacket(pSession, df_REQ_ROOM_CREATE, pRoom) == false)
        return false;

    if (SendPacket(pSession, df_REQ_ROOM_ENTER, pRoom) == false)
        return false;

    stUser_Chat chatInfo = { (wchar_t*)L"sibal", wcslen(L"sibal") * sizeof(wchar_t) };
    if (SendPacket(pSession, df_REQ_CHAT, &chatInfo) == false)
        return false;

    if (SendPacket(pSession, df_REQ_ROOM_LEAVE) == false)
        return false;

    return true;
}

void Release()
{
    // Session 정리
    for (auto iter = m_sessoinList.begin(); iter != m_sessoinList.end(); ++iter)
    {
        closesocket((*iter)->_socket);
        delete (*iter);
    }
}

bool ConnectSession(const wchar_t* ipWstr, int portNum)
{
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET)
    {
        return false;
    }

    sockaddr_in clientInfo;
    ZeroMemory(&clientInfo, sizeof(clientInfo));
    clientInfo.sin_family = AF_INET;
    InetPton(AF_INET, ipWstr, &clientInfo.sin_addr);
    clientInfo.sin_port = htons(portNum);

    int conRet = connect(clientSocket, (sockaddr*)&clientInfo, sizeof(clientInfo));
    if (conRet == SOCKET_ERROR)
    {
        return false;
    }

    u_long on = 1;
    int nonBlockRet = ioctlsocket(clientSocket, FIONBIO, &on);
    if (nonBlockRet == SOCKET_ERROR)
    {
        return false;
    }

    stSession* pSession = new stSession(clientSocket, &clientInfo);
    m_sessoinList.push_back(pSession);
    SINGLETON(CUserManager)->CreateUser(pSession);

    printf("연결 성공\n");

    return true;
}

void DisconnectSession(stSession* pSession)
{
    if (pSession != nullptr)
    {
        CUser* pUser = pSession->_pUser;
        if (pUser->GetRoom() != nullptr)
        {
            SINGLETON(CRoomManager)->LeaveRoom(pUser);
        }

        SINGLETON(CRoomManager)->ExitLobby(pUser);
        SINGLETON(CUserManager)->DeleteUser(pUser);

        closesocket(pSession->_socket);
        delete (pSession);
    }
}

bool SendPacket(stSession* pSession, int messageType, void* infoPtr)
{
    CPacket packetBuffer;
    try {
        switch (messageType)
        {
        case df_REQ_LOGIN:
            SPackingLogin(&packetBuffer, infoPtr);
            break;
        case df_REQ_ROOM_LIST:
            SPackingRoomList(&packetBuffer, infoPtr);
            break;
        case df_REQ_ROOM_CREATE:
            SPackingRoomCreate(&packetBuffer, infoPtr);
            break;
        case df_REQ_ROOM_ENTER:
            SPackingRoomEnter(&packetBuffer, infoPtr);
            break;
        case df_REQ_CHAT:
            SPackingChat(&packetBuffer, infoPtr);
            break;
        case df_REQ_ROOM_LEAVE:
            SPackingRoomLeave(&packetBuffer, infoPtr);
            break;
        }
    }
    catch (exception e)
    {
        wchar_t* wstrPtr = (wchar_t*)e.what();
        wprintf_s(L"%s\n", wstrPtr);
        return false;
    };

    if (pSession->_pSendBuffer->GetFreeSize() < packetBuffer.GetHeaderSize() + packetBuffer.GetDataSize())
        return false;

    pSession->_pSendBuffer->Enqueue(packetBuffer.GetHeaderPtr(), packetBuffer.GetHeaderSize());
    pSession->_pSendBuffer->Enqueue((char*)packetBuffer.GetBufferPtr(), packetBuffer.GetDataSize());
    return true;
}

bool RecvPacket(stSession* pSession)
{
    bool retVal = true;
    CRingBuffer* pRingBuffer = pSession->_pRecvBuffer;
    try {
        while (true)
        {
            CPacket packetBuffer;
            int packetHeaderSize = packetBuffer.GetHeaderSize();
            int usedSize = pRingBuffer->GetUseSize();
            if (usedSize < packetHeaderSize)
                break;

            if (pRingBuffer->Peek(packetBuffer.GetHeaderPtr(), packetHeaderSize) != packetHeaderSize)
                break;

            short payloadSize = packetBuffer.GetPayloadSize();
            if (usedSize < packetHeaderSize + payloadSize)
                break;

            if (pRingBuffer->MoveFront(packetHeaderSize) == false)
                break;

            if (payloadSize != 0)
            {
                if (pRingBuffer->GetUseSize() < payloadSize)
                    break;

                if (pRingBuffer->Dequeue(packetBuffer.GetBufferPtr(), payloadSize) != payloadSize)
                    break;

                packetBuffer.MoveWritePos(payloadSize);
            }


            short messageType = packetBuffer.GetMsgType();

            //wprintf_s(L"PacketRecv [UserNO:%d][Type:%d]\n", pSession->_pUser->GetUserID(), messageType);

            switch (messageType)
            {
            case df_RES_LOGIN:
                retVal = netPacketProc_Login(pSession, &packetBuffer);
                break;
            case df_RES_ROOM_LIST:
                retVal = netPacketProc_RoomList(pSession, &packetBuffer);
                break;
            case df_RES_ROOM_CREATE:
                retVal = netPacketProc_RoomCreate(pSession, &packetBuffer);
                break;
            case df_RES_ROOM_ENTER:
                retVal = netPacketProc_RoomEnter(pSession, &packetBuffer);
                break;
            case df_RES_CHAT:
                retVal = netPacketProc_Chat(pSession, &packetBuffer);
                break;
            case df_RES_ROOM_LEAVE:
                retVal = netPacketProc_RoomLeave(pSession, &packetBuffer);
                break;
            case df_RES_ROOM_DELETE:
                retVal = netPacketProc_RoomDelete(pSession, &packetBuffer);
                break;
            case df_RES_USER_ENTER:
                retVal = netPacketProc_UserEnter(pSession, &packetBuffer);
                break;
            }

            if (retVal == false)
            {
                return false;
            }
        }
    }
    catch (exception e)
    {
        wchar_t* wstrPtr = (wchar_t*)e.what();
        wprintf_s(L"%s\n", wstrPtr);
        return false;
    };

    return retVal;
}