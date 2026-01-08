#include "stdafx.h"
#include "Session.h"

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