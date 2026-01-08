#pragma once

enum {
	LANHEADER = 2,
};

struct st_SESSION;
class CSerializeBuffer;
class CRingBuffer;
template<typename T> class CLockFreeQueue;
class CLanClient
{
protected:
	CLanClient();
	~CLanClient();

public:
	bool Init();
	void Release();
	// 오픈 IP / 포트 / 워커스레드 수(생성수, 러닝수) / 나글옵션 / 최대접속자 수
	bool Start(const wchar_t* ipWstr, int portNum, int workerCreateCnt, int workerRunningCnt,
		bool bNoDelayOpt, int serverMaxUser, bool bRSTOpt, bool bKeepAliveOpt, bool bOverlappedSend, int sendQsize, int RingBufferSize);
	void Stop();

	// Accept 후 접속처리 완료 후 호출.
	virtual void OnClientJoin() = 0;
	// Release 후 호출
	virtual void OnClientLeave() = 0;

	// 패킷 수신 완료 후
	virtual int OnRecv(CSerializeBuffer* pPacket) = 0;

	virtual void OnError(const wchar_t* ipWstr, int portNum, int errorCode, const wchar_t* errorMsg) = 0;

private:
	static unsigned __stdcall netIOThread(void* pParameter);

	int netWSARecv();
	int netWSARecvPost();
protected:
	int netWSASendPacket(CSerializeBuffer* pPacket);
	int netWSASendEnq(CSerializeBuffer* pPacket);
private:
	int netWSASendPost();
	// 세션 생성
	st_SESSION* CreateSession(SOCKET clientSocket);

protected:
	bool GetIP_Port(wchar_t* outIpStr, unsigned short* outPortNum);
	void ClientDisconnectSession();
private:
#ifdef MY_DEBUG
	// 세션 연결 종료 처리
	void DisconnectSession();

	void DecreaseIOCount();
	void PrintError(const wchar_t* errStr, int errorCode);

	st_SESSION* AcquireLock();
	void ReleaseLock();
#else
	__forceinline void DisconnectSession();

	__forceinline void DecreaseIOCount();
	__forceinline void PrintError(const wchar_t* errStr, int errorCode);

	__forceinline st_SESSION* AcquireLock();
	__forceinline void ReleaseLock();
#endif	

private:
	HANDLE		m_hIOCP;
	HANDLE		m_hIOThread;

	wchar_t m_ipWstr[20];
	int m_portNum;
	bool m_bNoDelayOpt;
	bool m_bRSTOpt;
	bool m_bKeepAliveOpt;
	bool m_bOverlappedSend;

	st_SESSION* m_Session;
	CLockFreeTlsPoolA<CSerializeBuffer>* m_packetTlsPool;
	DWORD m_tlsIndex = 0;
#ifdef MY_DEBUG
	DWORD m_tlsDebugIndex = 0;
#endif
	long long m_SendTpsCount = 0;
	long long m_RecvTpsCount = 0;

	// 값이 지속적으로 변하는 변수들
	int m_tlsCount = 0;
};