#pragma once

struct st_PLAYER;
class CSerializeBuffer;
class CLoginServer : public CNetServer
{
public:
	CLoginServer();
	~CLoginServer();

	bool Start(const wchar_t* ipWstr, int portNum, int workerCreateCnt, int workerRunningCnt,
		bool bNoDelayOpt, int serverMaxUser, bool bRSTOpt, bool bKeepAliveOpt, bool bOverlappedSend, int SendQSize);
	void Stop();

	// accept 직후
	virtual bool OnConnectionRequest(const wchar_t* ipWstr, int portNum);

	// Accept 후 접속처리 완료 후 호출.
	virtual void OnClientJoin(unsigned long long SessionID);
	// Release 후 호출
	virtual void OnClientLeave(unsigned long long SessionID);

	// 패킷/메시지 수신 완료 후
	virtual int OnRecv(unsigned long long SessionID, CSerializeBuffer* pPacket);

	virtual void OnError(const wchar_t* ipWstr, int portNum, int errorCode, const wchar_t* errorMsg);

	void GetMonitoringInfo(long long AcceptTps, long long RecvTps, long long SendTps,
		long long acceptCount, long long disconnectCount, int sessionCount,
		int chunkCount, int chunkNodeCount,
		int workerCreatCnt, int workerRunningCnt,
		long long sendBytePerSec, long long recvBytePerSec);

	static unsigned int __stdcall DatabaseThread(void* pParameter);

	bool SendPlayerMessage(st_PLAYER* pPlayer, BYTE Status);

private:
	CLockFreeTlsPoolA<st_PLAYER>* m_playerPool;
	CLockFreeQueue<st_PLAYER*>* m_databaseQueue;
	HANDLE m_hDBEvent;

	HANDLE m_hDBThread;

	DWORD m_tlsDatabaseIndex = 0;
	DWORD m_tlsRedisIndex = 0;

	WCHAR m_GameServerIP[16];
	USHORT m_GameServerPort = 0;
	WCHAR m_ChattingServerIP[3][16];
	USHORT m_ChattingServerPort[3];

	WCHAR m_DatabaseIP[16];
	USHORT m_DatabasePort = 0;
	WCHAR m_DatabaseUser[20];
	WCHAR m_DatabasePassword[20];
	WCHAR m_DatabaseName[20];

	WCHAR m_RedisIP[16];
	USHORT m_RedisPort = 0;
	int m_RedisExistTime;
};