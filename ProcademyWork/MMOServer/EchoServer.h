#pragma once

class CEchoServer : public CMMOServer
{
	struct st_PLAYER : public CMMOServer::st_SESSION
	{
	public:
		st_PLAYER();
		virtual ~st_PLAYER();

		virtual void OnAuth_ClientJoin(void);
		virtual void OnAuth_ClientLeave(void);
		virtual bool OnAuth_Packet(CSerializeBuffer* pRecvPacket);
		virtual void OnGame_ClientJoin(void);
		virtual void OnGame_ClientLeave(void);
		virtual bool OnGame_Packet(CSerializeBuffer* pRecvPacket);
		virtual void OnGame_Release(void);

	private:
		INT64 AccountNo;
		char SessionKey[64];
		WCHAR ID[20];
		WCHAR Nickname[20];
		DWORD timeout;
	};

public:
	CEchoServer();
	~CEchoServer();

	bool Start(const wchar_t* ipWstr, int portNum, int workerCreateCnt, int workerRunningCnt,
		bool bNoDelayOpt, int serverMaxUser, bool bRSTOpt, bool bKeepAliveOpt, bool bOverlappedSend, 
		int SendQSize, int RingBufferSize, int CompleteRecvQSize, int SocketQSize);
	void Stop();

	// accept Á÷ÈÄ
	virtual bool OnConnectionRequest(const wchar_t* ipWstr, int portNum);

	virtual void OnError(const wchar_t* ipWstr, int portNum, int errorCode, const wchar_t* errorMsg);

	virtual void GetMonitoringInfo(long long AcceptTps, long long RecvTps, long long SendTps,
		long long AuthTps, long long GameTps,
		long long acceptCount, long long disconnectCount, int sessionCount,
		int chunkCount, int chunkNodeCount,
		long long sendBytePerSec, long long recvBytePerSec);

private:
	CLockFreeTlsPoolA<st_PLAYER>* m_pPlayerPool;

	bool m_shutdown;
	int m_timeOut;
};