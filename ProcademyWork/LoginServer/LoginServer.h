#pragma once

struct st_PLAYER;
class CSerializeBuffer;
class CTlsDBConnector;
class CLoginServer : public CNetServer
{
public:
	CLoginServer();
	~CLoginServer();

	bool Start(const wchar_t* ipWstr, int portNum, int workerCreateCnt, int workerRunningCnt,
		bool bNoDelayOpt, int serverMaxUser, bool bRSTOpt, bool bKeepAliveOpt, bool bOverlappedSend, int SendQSize, int RingBufferSize);
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
		long long sendBytePerSec, long long recvBytePerSec);

	static unsigned int __stdcall RedisThread(void* pParameter);

	bool SendPlayerMessage(st_PLAYER* pPlayer, BYTE Status);

private:
	CLockFreeTlsPoolA<st_PLAYER>* m_playerPool;
	CLockFreeQueue<st_PLAYER*>* m_redisQueue;
	HANDLE m_hRedisEvent;
	HANDLE m_hRedisThread;

	DWORD m_tlsDatabaseIndex = 0;

	WCHAR m_GameServerIP[16];
	USHORT m_GameServerPort = 0;
	WCHAR m_ChattingServerIP[3][16];
	USHORT m_ChattingServerPort[3];

	WCHAR m_RedisIP[16];
	USHORT m_RedisPort = 0;
	int m_RedisExistTime;
	CTlsDBConnector* m_pTlsDBConnecter;

	long long m_successCount = 0;
	long long m_successPrevCount = 0;
};