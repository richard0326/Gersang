#include "stdafx.h"
#include "ContentContext.h"
#include "RoomMgr.h"
#include "UserMgr.h"
#include "Packet.h"

bool AcceptSession(stSession* pSession, SOCKET socket, sockaddr_in* pSocketInfo)
{
    wchar_t wstrBuf[20];
    InetNtop(AF_INET, (void*)&pSocketInfo->sin_addr, wstrBuf, sizeof(wstrBuf));
    int userNo = SINGLETON(CUserManager)->CreateUser(pSession);

    wprintf_s(L"Accept - %s:%d [UserNO:%d]\n", wstrBuf, ntohs(pSocketInfo->sin_port), userNo);

    return true;
}

void DeleteSession(stSession* pSession)
{
    CUser* pUser = pSession->_pUser;

    wchar_t wstrBuf[20];
    InetNtop(AF_INET, (void*)&pSession->_socketInfo.sin_addr, wstrBuf, sizeof(wstrBuf));
    int userNo = pUser->GetUserID();
    wprintf_s(L"Disconnect - %s:%d [UserNO:%d]\n", wstrBuf, ntohs(pSession->_socketInfo.sin_port), userNo);

    if (pUser->GetRoom() != nullptr)
    {
        SINGLETON(CRoomManager)->LeaveRoom(pUser);
    }

    SINGLETON(CRoomManager)->ExitLobby(pUser);
    SINGLETON(CUserManager)->DeleteUser(pUser);
}

bool SendPacket(stSession* pSession, int messageType, void* infoPtr)
{
    CPacket packetBuffer;
    try {
        switch (messageType)
        {
        case df_RES_LOGIN:
            SPackingLogin(&packetBuffer, infoPtr);
            break;
        case df_RES_ROOM_LIST:
            SPackingRoomList(&packetBuffer, infoPtr);
            break;
        case df_RES_ROOM_CREATE:
            SPackingRoomCreate(&packetBuffer, infoPtr);
            break;
        case df_RES_ROOM_ENTER:
            SPackingRoomEnter(&packetBuffer, infoPtr);
            break;
        case df_RES_CHAT:
            SPackingChat(&packetBuffer, infoPtr);
            break;
        case df_RES_ROOM_LEAVE:
            SPackingRoomLeave(&packetBuffer, infoPtr);
            break;
        case df_RES_ROOM_DELETE:
            SPackingRoomDelete(&packetBuffer, infoPtr);
            break;
        case df_RES_USER_ENTER:
            SPackingUserEnter(&packetBuffer, infoPtr);
            break;
        default:
            return false;
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

            switch (messageType)
            {
            case df_REQ_LOGIN:
                retVal = netPacketProc_Login(pSession, &packetBuffer);
                break;
            case df_REQ_ROOM_LIST:
                retVal = netPacketProc_RoomList(pSession, &packetBuffer);
                break;
            case df_REQ_ROOM_CREATE:
                retVal = netPacketProc_RoomCreate(pSession, &packetBuffer);
                break;
            case df_REQ_ROOM_ENTER:
                retVal = netPacketProc_RoomEnter(pSession, &packetBuffer);
                break;
            case df_REQ_CHAT:
                retVal = netPacketProc_Chat(pSession, &packetBuffer);
                break;
            case df_REQ_ROOM_LEAVE:
                retVal = netPacketProc_RoomLeave(pSession, &packetBuffer);
                break;
            default:
                return false;
            }

            if (retVal == false)
            {
                return false;
            }

            wprintf_s(L"PacketRecv [UserNO:%d][Type:%d]\n", pSession->_pUser->GetUserID(), messageType);
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

bool BroadcastRoom(CRoom* pRoom, stSession* pSession, int messageType, void* infoPtr)
{
    while (true)
    {
        CUser* pUser = pRoom->GetNextUser();
        if (pUser == nullptr)
            break;

        if (pUser == pSession->_pUser)
            continue;

        if (SendPacket(pUser->GetSession(), messageType, infoPtr) == false)
            return false;
    }

    return true;
}

bool BroadcastAll(stSession* pSession, int messageType, void* infoPtr)
{
    while (true)
    {
        CUser* pUser = SINGLETON(CRoomManager)->GetNextUser();
        if (pUser == nullptr)
            break;

        if (pUser == pSession->_pUser)
            continue;

        if (SendPacket(pUser->GetSession(), messageType, infoPtr) == false)
            return false;
    }

    while (true)
    {
        CRoom* pRoom = SINGLETON(CRoomManager)->GetNextRoom();
        if (pRoom == nullptr)
            break;

        if (BroadcastRoom(pRoom, pSession, messageType, infoPtr) == false)
            return false;
    }

    return true;
}