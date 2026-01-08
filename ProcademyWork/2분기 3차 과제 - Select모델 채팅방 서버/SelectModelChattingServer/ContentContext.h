#pragma once

struct stResult_ID
{
	char _result;
	int _ID;
};

class CRoom;
struct stResult_Room
{
	char _result;
	CRoom* _pRoom;
};

class CUser;
struct stUser_Chat
{
	CUser* _pUser;
	wchar_t* _wchpChat;
	short _chatLen;
};

struct stSession;
bool AcceptSession(stSession* pSession, SOCKET socket, sockaddr_in* pSocketInfo);
void DeleteSession(stSession* pSession);
bool SendPacket(stSession* pSession, int messageType, void* infoPtr = nullptr);
bool RecvPacket(stSession* pSession);
bool BroadcastRoom(CRoom* pRoom, stSession* pSession, int messageType, void* infoPtr);
bool BroadcastAll(stSession* pSession, int messageType, void* infoPtr);