#pragma once

#define INIT_SESSION 1

class CUser;
struct stSession
{
    SOCKET _socket = INVALID_SOCKET;
    sockaddr_in _socketInfo;
    CRingBuffer* _pRecvBuffer = nullptr;
    CRingBuffer* _pSendBuffer = nullptr;

    stSession(SOCKET sock, sockaddr_in* pSockInfo);
    ~stSession();

    CUser* _pUser = nullptr;
};

struct stUser_Chat
{
	wchar_t* _wchpChat;
	short _chatLen;
};