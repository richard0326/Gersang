#pragma once

//#define FD_SETSIZE 1024

class CRingBuffer;
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

class CSelectServer
{
public:

	CSelectServer();
	~CSelectServer();

	bool Init(const wchar_t* ipWstr, int portNum);
	void Release();

	bool SelectLoop(
		bool (*AcceptFunc)(stSession* pSession, SOCKET socket, sockaddr_in* pSocketInfo), 
		void (*DeleteFunc)(stSession* pSession), 
		bool (*RecvFunc)(stSession* pSession));
	
	bool AddSession(
		bool (*AcceptFunc)(stSession* pSession, SOCKET socket, sockaddr_in* pSocketInfo), 
		SOCKET socket, sockaddr_in* psocketInfo);
	
	void DeleteSession(
		void (*DeleteFunc)(stSession* pSession), 
		stSession* pSession);

private:
	SOCKET m_listen_sock = INVALID_SOCKET;
	sockaddr_in m_listeninfo;
	list<stSession*> m_sessoinList;
	stSession* m_addedSession = nullptr;
};