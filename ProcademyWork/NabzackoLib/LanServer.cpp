#include "stdafx.h"
#include "LanServer.h"

enum {
	WSABUF_SIZE = 200,
	RET_SOCKET_ERROR = 0,
	RET_SUCCESS = 1,
};

struct OVERLAPPEDEX : public OVERLAPPED
{
	bool recvFlags;
};

#ifdef MY_DEBUG

enum class DEBUG_LOCATION
{
	RECV_BEF_ONRECV,
	RECV_PAYLOAD_FAIL,
	RECV_RECVPACKET_FAIL,
	RECV_DECRYPTION_FAIL,
	RECV_AFT_ONRECV,
	RECV_AFT_RECVPOST,
	RECVPOST_MODE1_BEF_WSARECV,
	RECVPOST_MODE1_AFT_WSARECV,
	RECVPOST_MODE1_WSARECVERR,
	RECVPOST_MODE2_BEF_WSARECV,
	RECVPOST_MODE2_AFT_WSARECV,
	RECVPOST_MODE2_WSARECVERR,

	SENDPACKET_BEF_ENQ,
	SENDPACKET_AFT_ENQ,
	SENDPACKET_BEF_SENDPOST,
	SENDPACKET_AFT_SENDPOST,

	SENDPOST_USESIZE_FAIL,
	SENDPOST_MODE1_BEF_WSASEND,
	SENDPOST_MODE1_WSASENDERR,
	SENDPOST_MODE1_AFT_WSASEND,
	SENDPOST_MODE1_WSASEND_SYNC,

	ACCEPT_BEF_ONCLIENTJOIN,
	ACCEPT_AFT_ONCLIENTJOIN,
	ACCEPT_BEF_RECVPOST,
	ACCEPT_AFT_RECVPOST,

	IO_TRANS_RECV_ZERO,
	IO_TRANS_SEND_ZERO,
	IO_BEF_RECV,
	IO_AFT_RECV,
	IO_BEF_SENDPOST,
	IO_AFT_SENDPOST,

	DISCONNECT_TRY1,
	DISCONNECT_TRY2,
	DISCONNECT_CLOSESOCKET,

	RELEASE_SUCCESS,

	DECREASE_TRY,
	DECREASE_SUCCESS,

	SENDPOST_INCREASE_IOCNT,
	RECVPOST_INCREASE_IOCNT,

	ACQUIRE_ADD_MISTAKE,
	ACQUIRE_NOT_MY_SESSION,
	ACQUIRE_ALREADY_RELEASED,
	ACQUIRE_SUCCESS,
	RELEASE_BEGIN,
};

#endif

// 세션 정보 구조체.
struct st_SESSION
{
	// 읽기 전용 데이터
	CLockFreeQueue<CSerializeBuffer*>* SendQ; // 송신 큐.
	CRingBuffer* RecvQ; // 수신 큐.

	unsigned long long SessionID = 0; // 접속자의 고유 세션 ID.
	SOCKET Socket = 0; // 현 접속의 TCP 소켓.
	wchar_t IpStr[16];
	u_short usPort;
	bool isShutdown;

	alignas(64) int PacketArrLen;
	CSerializeBuffer* pPacketArr[WSABUF_SIZE];

	OVERLAPPEDEX RecvOverlappedEx;	// 수신 오버랩드 구조체
	OVERLAPPEDEX SendOverlappedEx;	// 송신 오버랩드 구조체

	__declspec(align(64)) bool IOSend = false;
	short releaseFlag = false;
	short IOCount = 0;
#ifdef MY_DEBUG
	struct st_ForDebug
	{
		DWORD threadID;
		unsigned int debugID;
		DEBUG_LOCATION location;
		short IOCount;
		short releaseFlag;
		bool IOSend;
		bool isShutdown;
		int RecvQSize;
		int SendQSize;
		SOCKET workSocket;
		DWORD workID;
	};

	__declspec(align(64)) unsigned int debugID = 0;
	st_ForDebug debugSendArr[100];
	st_ForDebug debugRecvArr[100];
	st_ForDebug debugAcceptArr[100];
	DWORD tlsIndex;
	__declspec(align(64)) unsigned int debugSendCount = 0;
	__declspec(align(64)) unsigned int debugRecvCount = 0;
	__declspec(align(64)) unsigned int debugAcceptCount = 0;
#endif
};

#ifdef MY_DEBUG
void DebugCheck(DEBUG_LOCATION location, st_SESSION* pSession);
#define DEBUG(location, pSession) DebugCheck((DEBUG_LOCATION)location, pSession)
#else
#define DEBUG(location, pSession) ;
#endif

CLanServer::CLanServer()
{
	CSerializeBuffer::Init();
}

CLanServer::~CLanServer()
{

}

bool CLanServer::Init()
{
	LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"Server Init...");
	WSADATA wsa;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsa);
	if (iResult != NO_ERROR) {
		LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"WSAStartup() Error : %d", WSAGetLastError());
		return false;
	}
	LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"WSAStartup() Success");

	CSerializeBuffer::Reset();

	m_tlsIndex = TlsAlloc();
	if (m_tlsIndex == TLS_OUT_OF_INDEXES)
		return false;

#ifdef MY_DEBUG
	m_tlsDebugIndex = TlsAlloc();
	if (m_tlsDebugIndex == TLS_OUT_OF_INDEXES)
		return false;

	m_tlsSendBPSIndex = TlsAlloc();
	if (m_tlsSendBPSIndex == TLS_OUT_OF_INDEXES)
		return false;

	m_tlsRecvBPSIndex = TlsAlloc();
	if (m_tlsRecvBPSIndex == TLS_OUT_OF_INDEXES)
		return false;
#endif // MY_DEBUG

	m_tlsSendTpsIndex = TlsAlloc();
	if (m_tlsSendTpsIndex == TLS_OUT_OF_INDEXES)
		return false;

	m_tlsRecvTpsIndex = TlsAlloc();
	if (m_tlsRecvTpsIndex == TLS_OUT_OF_INDEXES)
		return false;

	m_packetTlsPool = (CLockFreeTlsPoolA<CSerializeBuffer>*)CSerializeBuffer::GetTlsPool();

	LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"SerializeBuffer chunkSize : %d, nodeSize : %d", m_packetTlsPool->GetMaxChunkSize(), m_packetTlsPool->GetMaxNodeSize());

	return true;
}

void CLanServer::Release()
{
	TlsFree(m_tlsRecvTpsIndex);
	TlsFree(m_tlsSendTpsIndex);

#ifdef MY_DEBUG
	TlsFree(m_tlsRecvBPSIndex);
	TlsFree(m_tlsSendBPSIndex);
	TlsFree(m_tlsDebugIndex);
#endif
	TlsFree(m_tlsIndex);

	// 윈속 종료
	WSACleanup();

	LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"Server Release...");
}

// 오픈 IP / 포트 / 워커스레드 수(생성수, 러닝수) / 나글옵션 / 최대접속자 수
bool CLanServer::Start(const wchar_t* ipWstr, int portNum,
	int workerCreateCnt, int workerRunningCnt,
	bool bNoDelayOpt, int serverMaxUser, bool bRSTOpt, bool bKeepAliveOpt, bool bOverlappedSend, int SendQSize, int RingBufferSize)
{
	// 인자 저장해두기
	wcscpy_s(m_ipWstr, ipWstr);
	m_portNum = portNum;
	m_workerCreateCnt = workerCreateCnt;
	m_workerRunningCnt = workerRunningCnt;
	m_bNoDelayOpt = bNoDelayOpt;
	m_MaxSession = serverMaxUser;
	m_bRSTOpt = bRSTOpt;
	m_bKeepAliveOpt = bKeepAliveOpt;
	m_bOverlappedSend = bOverlappedSend;

	// 모니터링 정보들
	m_pTlsIndexTpsArr = new long long[m_workerCreateCnt + 5];
	m_pTlsSendTpsArr = new long long[m_workerCreateCnt + 5];
	m_pTlsRecvTpsArr = new long long[m_workerCreateCnt + 5];
	m_pSaveSendTpsArr = new long long[m_workerCreateCnt + 5];
	m_pSaveRecvTpsArr = new long long[m_workerCreateCnt + 5];

#ifdef MY_DEBUG
	m_pSendBytePerSecArr = new long long[m_workerCreateCnt + 5];
	m_pSaveSendBytePerSecArr = new long long[m_workerCreateCnt + 5];
	m_pRecvBytePerSecArr = new long long[m_workerCreateCnt + 5];
	m_pSaveRecvBytePerSecArr = new long long[m_workerCreateCnt + 5];
#endif

	for (int i = 0; i < m_workerCreateCnt + 5; ++i)
	{
		m_pTlsIndexTpsArr[i] = 0;
		m_pTlsSendTpsArr[i] = 0;
		m_pTlsRecvTpsArr[i] = 0;
		m_pSaveSendTpsArr[i] = 0;
		m_pSaveRecvTpsArr[i] = 0;
#ifdef MY_DEBUG
		m_pSendBytePerSecArr[i] = 0;
		m_pSaveSendBytePerSecArr[i] = 0;
		m_pRecvBytePerSecArr[i] = 0;
		m_pSaveRecvBytePerSecArr[i] = 0;
#endif
	}
	//

	m_ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_ListenSocket == INVALID_SOCKET) {
		LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"socket() Error : %d", WSAGetLastError());
		return false;
	}
	LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"socket() Success");

	ZeroMemory(&m_ListenInfo, sizeof(m_ListenInfo));
	m_ListenInfo.sin_family = AF_INET;
	InetPton(AF_INET, ipWstr, &m_ListenInfo.sin_addr);
	m_ListenInfo.sin_port = htons(portNum);

	int iResult = ::bind(m_ListenSocket, (sockaddr*)&m_ListenInfo, sizeof(m_ListenInfo));
	if (iResult == SOCKET_ERROR) {
		LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"bind() Error : %d", WSAGetLastError());
		return false;
	}
	LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"[ IP %s, Port %d ] bind() Success", ipWstr, portNum);

	if (m_bNoDelayOpt)
	{
		BOOL opt = TRUE;
		setsockopt(m_ListenSocket, IPPROTO_TCP, TCP_NODELAY, (const char*)&opt, sizeof(opt));

		LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"NoDelay Opt True");
	}

	if (m_bRSTOpt)
	{
		struct linger optval;
		optval.l_onoff = 1;
		optval.l_linger = 0;
		setsockopt(m_ListenSocket, SOL_SOCKET, SO_LINGER, (char*)&optval, sizeof(optval));

		LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"RST Opt True");
	}

	if (m_bKeepAliveOpt)
	{
		struct tcp_keepalive
		{
			u_long onoff;
			u_long keepalivetime;
			u_long keepaliveinterval;
		};

		tcp_keepalive keepAlive;
		// keep-alive 활성화   
		keepAlive.onoff = 1;

		// 5초, 양 소켓사이에 패킷 교환이 없는 시간 간격이 아래보다 크면 keep-alive probe 즉 검사패킷을 보낸다.   
		// 응용단과는 무관하다.   
		keepAlive.keepalivetime = 5000; // 5초   

		// 검사패킷에 대하여 상대방이 응답이 없으면 아래 시간 간격으로 검사패킷을 반복적으로 보낸다.   
		// ms tcp 는 10회까지이다. 이후에 연결이 종료되고 응용단에 종료이벤트가 감지된다.   
		keepAlive.keepaliveinterval = 1000; // 1 초   
		DWORD dwBytesReturned = 0;
		int retIOCTL = WSAIoctl(m_ListenSocket, _WSAIOW(IOC_VENDOR, 4), &keepAlive, sizeof(tcp_keepalive), 0, 0, &dwBytesReturned, NULL, NULL);
		if (retIOCTL != 0)
		{
			int WSAError = WSAGetLastError();
			if (WSAError != WSA_IO_PENDING)
			{
				LOG(L"CNetServer", en_LOG_LEVEL::LEVEL_SYSTEM, L"KeepAliveOpt Fail : %d ", WSAError);
				return false;
			}
		}
		LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"KeepAliveOpt On, waitDelay %d, waitInterval %d", keepAlive.keepalivetime, keepAlive.keepaliveinterval);
	}

	iResult = listen(m_ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"listen() Error : %d", WSAGetLastError());
		return false;
	}
	LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"listen() Success");

	m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, workerRunningCnt);
	if (m_hIOCP == INVALID_HANDLE_VALUE)
	{
		LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"CreateIoCompletionPort() Error : %d", WSAGetLastError());
		return false;
	}
	LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"CreateIoCompletionPort() Success");

	m_SessionArray = new st_SESSION[m_MaxSession];

	m_idxStack = new CLockFreeStack<int>(m_MaxSession);
	for (int i = m_MaxSession - 1; i >= 0; --i)
	{
		m_SessionArray[i].SendQ = new CLockFreeQueue<CSerializeBuffer*>(SendQSize);
		m_SessionArray[i].RecvQ = new CRingBuffer(RingBufferSize);
		m_idxStack->Push(i);
	}

	LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"WorkerCreate : %d, WorkerRunning : %d, SendQSize : %d, RingBufferSize : %d",
		m_workerCreateCnt, m_workerRunningCnt, SendQSize, RingBufferSize);

	m_hIOThread = new HANDLE[m_workerCreateCnt];
	for (int i = 0; i < m_workerCreateCnt; ++i)
	{
		m_hIOThread[i] = (HANDLE)_beginthreadex(nullptr, 0, CLanServer::netIOThread, this, 0, nullptr);
	}

	m_hAcceptThread = (HANDLE)_beginthreadex(nullptr, 0, CLanServer::netAcceptThread, this, 0, nullptr);
	m_hMonitoringThread = (HANDLE)_beginthreadex(nullptr, 0, CLanServer::monitoringThread, this, 0, nullptr);

	LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"Server Start...");

	return true;
}

void CLanServer::Stop()
{
	LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"Server Stopping... Wait 1Min...");

	TerminateThread(m_hMonitoringThread, 0);
	CloseHandle(m_hMonitoringThread);
	closesocket(m_ListenSocket);

	DWORD waitAccept = WaitForSingleObject(m_hAcceptThread, 10000);
	if (waitAccept == WAIT_TIMEOUT)
	{
		TerminateThread(m_hAcceptThread, 0);
		LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"Server Stop. Terminate Accept Thread");
	}

	for (int i = 0; i < m_workerCreateCnt; ++i)
		PostQueuedCompletionStatus(m_hIOCP, 0, 0, 0);

	// 1초 동안 기다린다...
	DWORD waitCount = WaitForMultipleObjects(m_workerCreateCnt, m_hIOThread, true, 60000);
	if (waitCount == WAIT_TIMEOUT)
	{
		for (int i = 0; i < m_workerCreateCnt; ++i)
		{
			TerminateThread(m_hIOThread[i], 0);
			LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"Server Stop. Terminate Worker Thread %d", i + 1);
		}
	}

	for (int i = 0; i < m_workerCreateCnt; ++i)
	{
		CloseHandle(m_hIOThread[i]);
	}
	CloseHandle(m_hAcceptThread);

	DWORD test = WAIT_OBJECT_0 + m_workerCreateCnt;
	CloseHandle(m_hIOCP);

	if (m_workerCreateCnt == 1)
	{
		delete m_hIOThread;
	}
	else
	{
		delete[] m_hIOThread;
	}

	delete m_idxStack;
	for (int i = 0; i < m_MaxSession; ++i)
	{
		closesocket(m_SessionArray[i].Socket);
		delete m_SessionArray[i].SendQ;
	}

	if (m_MaxSession == 1)
	{
		delete m_SessionArray;
	}
	else
	{
		delete[] m_SessionArray;
	}

	delete[] m_pTlsIndexTpsArr;
	delete[] m_pTlsSendTpsArr;
	delete[] m_pTlsRecvTpsArr;
	delete[] m_pSaveSendTpsArr;
	delete[] m_pSaveRecvTpsArr;
#ifdef MY_DEBUG
	delete[] m_pSendBytePerSecArr;
	delete[] m_pSaveSendBytePerSecArr;
	delete[] m_pRecvBytePerSecArr;
	delete[] m_pSaveRecvBytePerSecArr;
#endif
	LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"Server Stop...");
}

unsigned __stdcall CLanServer::monitoringThread(void* pParameter)
{
	CLanServer* pLanServer = (CLanServer*)pParameter;
	srand((unsigned int)GetCurrentThreadId());

	int* pSessionCount = &pLanServer->m_SessionCnt;
	long long* pAcceptCnt = &pLanServer->m_acceptCount;
	long long* pAcceptPrevCnt = &pLanServer->m_acceptPrevCount;

	int* pTlsCount = &pLanServer->m_tlsCount;
	long long* pRecvArr = pLanServer->m_pTlsRecvTpsArr;
	long long* pRecvPrevArr = pLanServer->m_pSaveRecvTpsArr;
	long long* pSendArr = pLanServer->m_pTlsSendTpsArr;
	long long* pSendPrevArr = pLanServer->m_pSaveSendTpsArr;

#ifdef MY_DEBUG
	long long* pDisconnectCnt = &pLanServer->m_DisconnectCnt;

	long long* pRecvBPS = pLanServer->m_pRecvBytePerSecArr;
	long long* pRecvPrevBPS = pLanServer->m_pSaveRecvBytePerSecArr;
	long long* pSendBPS = pLanServer->m_pSendBytePerSecArr;
	long long* pSendPrevBPS = pLanServer->m_pSaveSendBytePerSecArr;
#endif

	CLockFreeTlsPoolA<CSerializeBuffer>* pPacketTlsPool = pLanServer->m_packetTlsPool;

	long long RecvTpsSum = 0;
	long long SendTpsSum = 0;
	while (true)
	{
		long long temp = *pAcceptCnt;
		long long AcceptTps = temp - *pAcceptPrevCnt;
		*pAcceptPrevCnt = temp;

		RecvTpsSum = 0;
		SendTpsSum = 0;
		for (int i = 1; i <= *pTlsCount; ++i)
		{
			temp = pRecvArr[i];
			RecvTpsSum += temp - pRecvPrevArr[i];
			pRecvPrevArr[i] = temp;
		}

		for (int i = 1; i <= *pTlsCount; ++i)
		{
			temp = pSendArr[i];
			SendTpsSum += temp - pSendPrevArr[i];
			pSendPrevArr[i] = temp;
		}

#ifdef MY_DEBUG
		long long RecvBytePerSec = 0;
		for (int i = 1; i <= *pTlsCount; ++i)
		{
			temp = pRecvBPS[i];
			RecvBytePerSec += temp - pRecvPrevBPS[i];
			pRecvPrevBPS[i] = temp;
		}

		long long SendBytePerSec = 0;
		for (int i = 1; i <= *pTlsCount; ++i)
		{
			temp = pSendBPS[i];
			SendBytePerSec += temp - pSendPrevBPS[i];
			pSendPrevBPS[i] = temp;
		}

		pLanServer->GetMonitoringInfo(AcceptTps, RecvTpsSum, SendTpsSum,
			*pAcceptCnt, *pDisconnectCnt, *pSessionCount,
			pPacketTlsPool->GetChunkSize(), pPacketTlsPool->GetNodeSize(),
			SendBytePerSec, RecvBytePerSec);
#else
		pLanServer->GetMonitoringInfo(AcceptTps, RecvTpsSum, SendTpsSum,
			*pAcceptCnt, 0, *pSessionCount,
			pPacketTlsPool->GetChunkSize(), 0,
			0, 0);
#endif
		Sleep(1000);
	}

	return 0;
}

unsigned __stdcall CLanServer::netAcceptThread(void* pParameter)
{
	CLanServer* pLanServer = (CLanServer*)pParameter;
	srand((unsigned int)GetCurrentThreadId());

	while (true)
	{
		sockaddr_in clientaddr;
		int clientLen = sizeof(clientaddr);
		SOCKET clientSocket = accept(pLanServer->m_ListenSocket, (sockaddr*)&clientaddr, &clientLen);
		if (clientSocket == INVALID_SOCKET)
		{
			LOG(L"Error", en_LOG_LEVEL::LEVEL_ERROR, L"accept() Error : %d ", WSAGetLastError());
			return 0;
		}

		// 세션 생성
		st_SESSION* pSession = pLanServer->CreateSession(clientSocket);
		if (pSession == nullptr)
		{
			LOG(L"Error", en_LOG_LEVEL::LEVEL_ERROR, L"MaxUser Login Fail");
			closesocket(clientSocket);
			continue;
		}

		InetNtop(AF_INET, &clientaddr.sin_addr, pSession->IpStr, 16);
		pSession->usPort = ntohs(clientaddr.sin_port);

		if (pLanServer->OnConnectionRequest(pSession->IpStr, pSession->usPort) == false)
		{
			LOG(L"Error", en_LOG_LEVEL::LEVEL_ERROR, L"[ IP %s, Port %d ] Banned ", pSession->IpStr, pSession->usPort);
			pLanServer->DecreaseIOCount(pSession);
			continue;
		}

		// AcceptTps
		pLanServer->m_acceptCount++;

		// 송신버퍼 0으로 해서 비동기 방식으로 작동
		if (pLanServer->m_bOverlappedSend == true)
		{
			int sendSize = 0;
			setsockopt(clientSocket, SOL_SOCKET, SO_SNDBUF, (char*)&sendSize, sizeof(int));
		}

		CreateIoCompletionPort((HANDLE)clientSocket, pLanServer->m_hIOCP, (ULONG_PTR)pSession, 0);

		DEBUG(DEBUG_LOCATION::ACCEPT_BEF_ONCLIENTJOIN, pSession);

		// 콘텐츠 작업 파트
		pLanServer->OnClientJoin(pSession->SessionID);

		DEBUG(DEBUG_LOCATION::ACCEPT_AFT_ONCLIENTJOIN, pSession);

		DEBUG(DEBUG_LOCATION::ACCEPT_BEF_RECVPOST, pSession);

		int retRecvPost = pLanServer->netWSARecvPost(pSession);
		if (retRecvPost != RET_SUCCESS)
		{
			pLanServer->DisconnectSession(pSession);
		}

		DEBUG(DEBUG_LOCATION::ACCEPT_AFT_RECVPOST, pSession);

		pLanServer->DecreaseIOCount(pSession);
	}

	return 0;
}

unsigned __stdcall CLanServer::netIOThread(void* pParameter)
{
	CLanServer* pLanServer = (CLanServer*)pParameter;
	srand((unsigned int)GetCurrentThreadId());

	while (1)
	{
		DWORD transferred = 0;
		st_SESSION* pSession = nullptr;
		OVERLAPPEDEX* pOverlapped = nullptr;
		GetQueuedCompletionStatus(pLanServer->m_hIOCP, &transferred, (PULONG_PTR)&pSession, (LPOVERLAPPED*)&pOverlapped, INFINITE);

		if (pOverlapped == nullptr)
		{
			// 서버 종료
			LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"Overlapped IO nullptr : Exit Server ");
			return 0;
		}

#ifdef MY_DEBUG
		pSession->tlsIndex = pLanServer->m_tlsDebugIndex;
		if (TlsSetValue(pSession->tlsIndex, (LPVOID)pOverlapped) == false)
		{
			LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"TLS Fail : Exit Server ");
			CCrashDump::Crash();
			return 0;
		}
#endif

		if (transferred == 0)
		{
			pLanServer->DisconnectSession(pSession);
			// 세션 종료
			if (pOverlapped->recvFlags == false)
			{
				DEBUG(DEBUG_LOCATION::IO_TRANS_SEND_ZERO, pSession);
			}
			else
			{
				DEBUG(DEBUG_LOCATION::IO_TRANS_RECV_ZERO, pSession);
			}
		}
		else
		{
			if (pOverlapped->recvFlags == true)
			{
				if (pSession->RecvQ->MoveRear(transferred) == false)
				{
					LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"RecvQ MoveRear Error ");
					CCrashDump::Crash();
				}
				else
				{
					DEBUG(DEBUG_LOCATION::IO_BEF_RECV, pSession);

					int retRecv = pLanServer->netWSARecv(pSession);
					if (retRecv != RET_SUCCESS)
					{
						pLanServer->DisconnectSession(pSession);
					}

					DEBUG(DEBUG_LOCATION::IO_AFT_RECV, pSession);
				}
			}
			// Send Flag
			else
			{
				// SendTps
				long long* pSendTps = (long long*)TlsGetValue(pLanServer->m_tlsSendTpsIndex);
				if (pSendTps == nullptr)
				{
					long long index = (long long)TlsGetValue(pLanServer->m_tlsIndex);
					if (index == 0)
					{
						index = InterlockedIncrement((LONG*)&pLanServer->m_tlsCount);
						TlsSetValue(pLanServer->m_tlsIndex, (LPVOID)index);
					}
					pSendTps = &pLanServer->m_pTlsSendTpsArr[index];
					TlsSetValue(pLanServer->m_tlsSendTpsIndex, pSendTps);
				}
				for (int i = 0; i < pSession->PacketArrLen; i++)
				{
					CSerializeBuffer* pPacket = (CSerializeBuffer*)pSession->pPacketArr[i];
#ifdef MY_DEBUG
					pPacket->SetLog(13, pSession->SessionID);
#endif
					pPacket->DecreaseRefCount();
					(*pSendTps)++;
				}
				pSession->PacketArrLen = 0;

				InterlockedExchange8((char*)&pSession->IOSend, 0);

				DEBUG(DEBUG_LOCATION::IO_BEF_SENDPOST, pSession);
				int retSendPost = pLanServer->netWSASendPost(pSession);
				if (retSendPost != RET_SUCCESS)
				{
					pLanServer->DisconnectSession(pSession);
				}

				DEBUG(DEBUG_LOCATION::IO_AFT_SENDPOST, pSession);
			}
		}

		pLanServer->DecreaseIOCount(pSession);
	}

	return 0;
}

int CLanServer::netWSARecv(st_SESSION* pSession)
{
	// RecvTps
	long long* pRecvTps = (long long*)TlsGetValue(m_tlsRecvTpsIndex);
	if (pRecvTps == nullptr)
	{
		long long index = (long long)TlsGetValue(m_tlsIndex);
		if (index == 0)
		{
			index = InterlockedIncrement((LONG*)&m_tlsCount);
			TlsSetValue(m_tlsIndex, (LPVOID)index);
		}
		pRecvTps = &m_pTlsRecvTpsArr[index];
		TlsSetValue(m_tlsRecvTpsIndex, pRecvTps);
	}
#ifdef MY_DEBUG
	long long* pRecvBPS = (long long*)TlsGetValue(m_tlsRecvBPSIndex);
	if (pRecvBPS == nullptr)
	{
		long long index = (long long)TlsGetValue(m_tlsIndex);
		if (index == 0)
		{
			index = InterlockedIncrement((LONG*)&m_tlsCount);
			TlsSetValue(m_tlsIndex, (LPVOID)index);
		}
		pRecvBPS = &m_pRecvBytePerSecArr[index];
		TlsSetValue(m_tlsRecvBPSIndex, pRecvBPS);
	}
#endif

	while (1)
	{
		short Header = 0;

		int headerSize = LANHEADER;
		int UsedSize = pSession->RecvQ->GetUseSize();
		if (UsedSize < headerSize)
		{
			break;
		}

		if (headerSize == 0)
		{
			CCrashDump::Crash();
			return RET_SOCKET_ERROR;
		}

		int peekSize = pSession->RecvQ->Peek((char*)&Header, headerSize);
		if (peekSize != headerSize)
		{
			CCrashDump::Crash();
			return RET_SOCKET_ERROR;
		}

		short* pPayloadSize = &Header;
		// ringbuffer 크기보다 작아야하며, tcp 수신 버퍼보다 작아야함.
		if (*pPayloadSize <= 2 || 65535 < *pPayloadSize)
		{
			LOG(L"RecvError", en_LOG_LEVEL::LEVEL_DEBUG, L"[ IP %s, Port %d ] PayloadSizeWrong : %d", pSession->IpStr, pSession->usPort, *pPayloadSize);
			DEBUG(DEBUG_LOCATION::RECV_PAYLOAD_FAIL, pSession);
			return RET_SOCKET_ERROR;
		}

		// Packet의 최대 크기를 넘으면 할당에 실패한다.
		if (CSerializeBuffer::Check(LANHEADER, *pPayloadSize) == false)
		{
			LOG(L"RecvError", en_LOG_LEVEL::LEVEL_DEBUG, L"[ IP %s, Port %d ] RecvPacket Alloc Fail1", pSession->IpStr, pSession->usPort);
			DEBUG(DEBUG_LOCATION::RECV_RECVPACKET_FAIL, pSession);
			return RET_SOCKET_ERROR;
		}

		if (UsedSize < headerSize + *pPayloadSize)
		{
			break;
		}

		if (pSession->RecvQ->MoveFront(headerSize) == false)
		{
			CCrashDump::Crash();
			return RET_SOCKET_ERROR;
		}

		CSerializeBuffer* RecvPacket = CSerializeBuffer::Alloc(LANHEADER, *pPayloadSize);
		if (RecvPacket == nullptr)
		{
			LOG(L"CSerializeBuffer", en_LOG_LEVEL::LEVEL_ERROR, L"[ IP %s, Port %d ] OnRecv Alloc Fail");
			DEBUG(DEBUG_LOCATION::RECV_RECVPACKET_FAIL, pSession);
			return RET_SOCKET_ERROR;
		}

		int deqSize = pSession->RecvQ->Dequeue(RecvPacket->GetBufferPtr(), *pPayloadSize);
		if (deqSize != *pPayloadSize)
		{
			CCrashDump::Crash();
			return RET_SOCKET_ERROR;
		}

		int writePos = RecvPacket->MoveWritePos(deqSize);
		if (deqSize != writePos)
		{
			CCrashDump::Crash();
			return RET_SOCKET_ERROR;
		}

#ifdef MY_DEBUG
		* pRecvBPS += 20 + deqSize + headerSize;
#endif

		(*pRecvTps)++;

		DEBUG(DEBUG_LOCATION::RECV_BEF_ONRECV, pSession);

		int retOnRecv = OnRecv(pSession->SessionID, RecvPacket);
		RecvPacket->DecreaseRefCount();
		if (retOnRecv != RET_SUCCESS)
		{
			return retOnRecv;
		}

		DEBUG(DEBUG_LOCATION::RECV_AFT_ONRECV, pSession);
	}

	int retRecvPost = netWSARecvPost(pSession);
	if (retRecvPost != RET_SUCCESS)
	{
		return retRecvPost;
	}
	DEBUG(DEBUG_LOCATION::RECV_AFT_RECVPOST, pSession);

	return RET_SUCCESS;
}

int CLanServer::netWSARecvPost(st_SESSION* pSession)
{
	int afterEnqSize = 0;
	int enqSize = pSession->RecvQ->DirectEnqueueSize(&afterEnqSize);

	InterlockedIncrement16(&pSession->IOCount);
	DEBUG(DEBUG_LOCATION::RECVPOST_INCREASE_IOCNT, pSession);

	if (afterEnqSize == 0)
	{
		DEBUG(DEBUG_LOCATION::RECVPOST_MODE1_BEF_WSARECV, pSession);

		WSABUF wsaRecvBuf;
		wsaRecvBuf.buf = pSession->RecvQ->GetRearBufferPtr();
		wsaRecvBuf.len = enqSize;

		DWORD recvSize = 0;
		memset(&pSession->RecvOverlappedEx, 0, sizeof(OVERLAPPED));
		pSession->RecvOverlappedEx.recvFlags = true;

		DWORD flag = 0;
		int retRecv = WSARecv(pSession->Socket, &wsaRecvBuf, 1, &recvSize, &flag, &pSession->RecvOverlappedEx, nullptr);
		if (retRecv != 0)
		{
			int WSAError = WSAGetLastError();
			if (WSAError != WSA_IO_PENDING)
			{
				PrintError(pSession, L"netWSARecvPost() WSARecv Error ", WSAError);
				DEBUG(DEBUG_LOCATION::RECVPOST_MODE1_WSARECVERR, pSession);
				DecreaseIOCount(pSession);
				return RET_SOCKET_ERROR;
			}
		}

		DEBUG(DEBUG_LOCATION::RECVPOST_MODE1_AFT_WSARECV, pSession);
	}
	else
	{
		DEBUG(DEBUG_LOCATION::RECVPOST_MODE2_BEF_WSARECV, pSession);

		WSABUF wsaRecvBuf[2];
		wsaRecvBuf[0].buf = pSession->RecvQ->GetRearBufferPtr();
		wsaRecvBuf[0].len = enqSize;
		wsaRecvBuf[1].buf = pSession->RecvQ->GetBufferPtr();
		wsaRecvBuf[1].len = afterEnqSize;

		DWORD recvSize = 0;
		memset(&pSession->RecvOverlappedEx, 0, sizeof(OVERLAPPED));
		pSession->RecvOverlappedEx.recvFlags = true;

		DWORD flag = 0;

		int retRecv = WSARecv(pSession->Socket, wsaRecvBuf, 2, &recvSize, &flag, &pSession->RecvOverlappedEx, nullptr);
		if (retRecv == 0)
		{
			//(L"overlapped Recv Fail, SendBytes : %d ", recvSize);
		}

		if (retRecv != 0)
		{
			int WSAError = WSAGetLastError();
			if (WSAError != WSA_IO_PENDING)
			{
				PrintError(pSession, L"netWSARecvPost() WSARecv Error ", WSAError);
				DEBUG(DEBUG_LOCATION::RECVPOST_MODE2_WSARECVERR, pSession);
				DecreaseIOCount(pSession);
				return RET_SOCKET_ERROR;
			}
		}

		DEBUG(DEBUG_LOCATION::RECVPOST_MODE2_AFT_WSARECV, pSession);
	}

	return RET_SUCCESS;
}

int CLanServer::netWSASendPacket(unsigned long long SessionID, CSerializeBuffer* pPacket)
{
	st_SESSION* pSession = AcquireLock(SessionID);
	if (pSession == nullptr)
	{
		return RET_SOCKET_ERROR;
	}

	DEBUG(DEBUG_LOCATION::SENDPACKET_BEF_ENQ, pSession);

	pPacket->IncreaseRefCount();
#ifdef MY_DEBUG
	pPacket->SetLog(10, pSession->SessionID);
#endif
	pPacket->SetLanHeader();
	if (false == pSession->SendQ->Enqueue(pPacket))
	{
		LOG(L"Debug", en_LOG_LEVEL::LEVEL_DEBUG, L"[ IP %s, Port %d ] SendQ.Enqueue 1 Error", pSession->IpStr, pSession->usPort);

#ifdef MY_DEBUG
		pPacket->SetLog(11, pSession->SessionID);
#endif
		pPacket->DecreaseRefCount();
		ReleaseLock(pSession);
		return RET_SOCKET_ERROR;
	}
	DEBUG(DEBUG_LOCATION::SENDPACKET_AFT_ENQ, pSession);

	DEBUG(DEBUG_LOCATION::SENDPACKET_BEF_SENDPOST, pSession);
	int retSendPost = netWSASendPost(pSession);
	if (retSendPost != RET_SUCCESS)
	{
		ReleaseLock(pSession);
		return retSendPost;
	}

	DEBUG(DEBUG_LOCATION::SENDPACKET_AFT_SENDPOST, pSession);

	ReleaseLock(pSession);
	return RET_SUCCESS;
}

int CLanServer::netWSASendEnq(unsigned long long SessionID, CSerializeBuffer* pPacket)
{
	st_SESSION* pSession = AcquireLock(SessionID);
	if (pSession == nullptr)
	{
		return RET_SOCKET_ERROR;
	}

	DEBUG(DEBUG_LOCATION::SENDPACKET_BEF_ENQ, pSession);

	pPacket->IncreaseRefCount();
#ifdef MY_DEBUG
	pPacket->SetLog(10, pSession->SessionID);
#endif
	pPacket->SetLanHeader();
	if (false == pSession->SendQ->Enqueue(pPacket))
	{
		LOG(L"Debug", en_LOG_LEVEL::LEVEL_DEBUG, L"[ IP %s, Port %d ] SendQ.Enqueue 1 Error", pSession->IpStr, pSession->usPort);

#ifdef MY_DEBUG
		pPacket->SetLog(11, pSession->SessionID);
#endif
		pPacket->DecreaseRefCount();
		ReleaseLock(pSession);
		return RET_SOCKET_ERROR;
	}
	DEBUG(DEBUG_LOCATION::SENDPACKET_AFT_ENQ, pSession);

	// 송수신하지 않는 Session에 대해서 브로드캐스팅한 Packet이 쌓이지 않도록 유도
	if (pSession->SendQ->GetSize() > 10)
	{
		int retSendPost = netWSASendPost(pSession);
		if (retSendPost != RET_SUCCESS)
		{
			ReleaseLock(pSession);
			return retSendPost;
		}
	}

	ReleaseLock(pSession);
	return RET_SUCCESS;
}

int CLanServer::netWSASendPost(st_SESSION* pSession)
{
	if (pSession->SendQ->GetSize() == 0)
	{
		return RET_SUCCESS;
	}

	char retIOSend = InterlockedExchange8((char*)&pSession->IOSend, 1);
	if (retIOSend == 0)
	{
		DEBUG(DEBUG_LOCATION::SENDPOST_MODE1_BEF_WSASEND, pSession);

		WSABUF wsaSendBuf[WSABUF_SIZE];
		memset(&pSession->SendOverlappedEx, 0, sizeof(OVERLAPPED));
		pSession->SendOverlappedEx.recvFlags = false;

#ifdef MY_DEBUG
		long long* pSendBPS = (long long*)TlsGetValue(m_tlsSendBPSIndex);
		if (pSendBPS == nullptr)
		{
			long long index = (long long)TlsGetValue(m_tlsIndex);
			if (index == 0)
			{
				index = InterlockedIncrement((LONG*)&m_tlsCount);
				TlsSetValue(m_tlsIndex, (LPVOID)index);
			}
			pSendBPS = &m_pSendBytePerSecArr[index];
			TlsSetValue(m_tlsSendBPSIndex, pSendBPS);
		}
#endif

		int packetLoop = pSession->PacketArrLen;
		CLockFreeQueue<CSerializeBuffer*>* tSendQ = pSession->SendQ;
		CSerializeBuffer** ppPacketArr = pSession->pPacketArr;
		for (; packetLoop < WSABUF_SIZE; ++packetLoop)
		{
			CSerializeBuffer* pPacket = nullptr;
			if (false == tSendQ->Dequeue(&pPacket))
				break;

			wsaSendBuf[packetLoop].buf = pPacket->GetHeaderPtr();
			wsaSendBuf[packetLoop].len = pPacket->m_HeaderSize + pPacket->m_DataSize;
			ppPacketArr[packetLoop] = pPacket;
#ifdef MY_DEBUG
			pPacket->SetLog(12, pSession->SessionID);
			*pSendBPS += 20 + wsaSendBuf[packetLoop].len;
#endif
		}
		pSession->PacketArrLen = packetLoop;
		if (packetLoop == 0)
		{
			InterlockedExchange8((char*)&pSession->IOSend, 0);
			return RET_SUCCESS;
		}
#ifdef MY_DEBUG
		SOCKET sock = pSession->Socket;
		char* pPacketHeader = wsaSendBuf[0].buf;
		int bb = wsaSendBuf[0].len;
		int aa = packetLoop;
		OVERLAPPEDEX tmp = pSession->SendOverlappedEx;
#endif
		InterlockedIncrement16(&pSession->IOCount);
		DEBUG(DEBUG_LOCATION::SENDPOST_INCREASE_IOCNT, pSession);

		DWORD sendBytes = 0;
		int	sendErr = WSASend(pSession->Socket, wsaSendBuf, packetLoop, &sendBytes, 0, (LPWSAOVERLAPPED)&pSession->SendOverlappedEx, nullptr);
		if (sendErr == 0)
		{
			// 동기로 Send한 경우
			DEBUG(DEBUG_LOCATION::SENDPOST_MODE1_WSASEND_SYNC, pSession);
		}

		if (sendErr != 0)
		{
			int WSAError = WSAGetLastError();
			if (WSAError != WSA_IO_PENDING)
			{
				PrintError(pSession, L"WSASend() Error ", WSAError);

				DEBUG(DEBUG_LOCATION::SENDPOST_MODE1_WSASENDERR, pSession);
				DecreaseIOCount(pSession);

				return RET_SOCKET_ERROR;
			}
		}

		DEBUG(DEBUG_LOCATION::SENDPOST_MODE1_AFT_WSASEND, pSession);
	}

	return RET_SUCCESS;
}

st_SESSION* CLanServer::CreateSession(SOCKET clientSocket)
{
	int idx = 0;
	if (m_idxStack->Pop(&idx) == false)
		return nullptr;

	st_SESSION* pSession = &m_SessionArray[idx];

	pSession->isShutdown = false;
	pSession->Socket = clientSocket;
	pSession->SessionID = CONVERT_INFO_TO_ID(m_SessionID, idx);
	pSession->RecvQ->ClearBuffer();
	pSession->PacketArrLen = 0;
	InterlockedIncrement16(&pSession->IOCount);
	InterlockedExchange8((char*)&pSession->IOSend, 0);
	InterlockedExchange16(&pSession->releaseFlag, false);

	pSession->RecvOverlappedEx.recvFlags = true;
	pSession->SendOverlappedEx.recvFlags = false;
	++m_SessionID;

	InterlockedIncrement((LONG*)&m_SessionCnt);
	return pSession;
}

bool CLanServer::GetIP_Port(unsigned long long SessionID, wchar_t* outIpStr, unsigned short* outPortNum)
{
	st_SESSION* pSession = AcquireLock(SessionID);
	if (pSession == nullptr)
		return false;

	wcscpy_s(outIpStr, 16, pSession->IpStr);
	*outPortNum = pSession->usPort;

	ReleaseLock(pSession);
	return true;
}

// 세션 연결 종료 처리
void CLanServer::DisconnectSession(unsigned long long SessionID)
{
	st_SESSION* pSession = AcquireLock(SessionID);
	if (pSession == nullptr)
		return;

	DEBUG(DEBUG_LOCATION::DISCONNECT_TRY1, pSession);
	pSession->isShutdown = true;
	if (CancelIoEx((HANDLE)pSession->Socket, nullptr) == 0)
	{
		int WSAError = WSAGetLastError();
		if (WSAError != ERROR_NOT_FOUND)
		{
			PrintError(pSession, L"Disconnect1() Error ", WSAError);

			DEBUG(DEBUG_LOCATION::SENDPOST_MODE1_WSASENDERR, pSession);
		}
	}
	shutdown(pSession->Socket, SD_BOTH);

	ReleaseLock(pSession);
}

void CLanServer::DisconnectSession(st_SESSION* pSession)
{
	if (pSession->isShutdown == true)
		return;

	pSession->isShutdown = true;
	DEBUG(DEBUG_LOCATION::DISCONNECT_TRY2, pSession);
	if (CancelIoEx((HANDLE)pSession->Socket, nullptr) == 0)
	{
		int WSAError = WSAGetLastError();
		if (WSAError != ERROR_NOT_FOUND)
		{
			PrintError(pSession, L"Disconnect2() Error ", WSAError);

			DEBUG(DEBUG_LOCATION::SENDPOST_MODE1_WSASENDERR, pSession);
		}
	}
	shutdown(pSession->Socket, SD_BOTH);
}

void CLanServer::DecreaseIOCount(st_SESSION* pSession)
{
#ifdef DEBUG
	short retIOCnt = InterlockedDecrement16(&pSession->IOCount);

	DEBUG(DEBUG_LOCATION::DECREASE_TRY, pSession);

	if (retIOCnt < 0)
	{
		CCrashDump::Crash();
	}

	if (retIOCnt == 0)
#else
	if (0 == InterlockedDecrement16(&pSession->IOCount))
#endif // DEBUG
	{
		DEBUG(DEBUG_LOCATION::DECREASE_SUCCESS, pSession);

		if (InterlockedCompareExchange((LONG*)&pSession->releaseFlag, true, false) == false)
		{
			DEBUG(DEBUG_LOCATION::DISCONNECT_CLOSESOCKET, pSession);

			int len = pSession->PacketArrLen;
			CSerializeBuffer** pPacketArr = pSession->pPacketArr;
			for (int i = 0; i < len; ++i)
			{
				CSerializeBuffer* pPacket = pPacketArr[i];
#ifdef MY_DEBUG
				pPacket->SetLog(14, pSession->SessionID);
#endif
				pPacket->DecreaseRefCount();
			}
			pSession->PacketArrLen = 0;

			CLockFreeQueue<CSerializeBuffer*>* SendQ = pSession->SendQ;
			for (;;)
			{
				CSerializeBuffer* pPacket = nullptr;
				if (SendQ->Dequeue(&pPacket) == false)
					break;
#ifdef MY_DEBUG
				pPacket->SetLog(15, pSession->SessionID);
#endif
				pPacket->DecreaseRefCount();
			}
#ifdef MY_DEBUG
			if (SendQ->GetSize() != 0)
			{
				CCrashDump::Crash();
			}

			InterlockedIncrement64(&m_DisconnectCnt);
#endif

			closesocket(pSession->Socket);

			OnClientLeave(pSession->SessionID);

			DEBUG(DEBUG_LOCATION::RELEASE_SUCCESS, pSession);

			InterlockedDecrement((LONG*)&m_SessionCnt);
			if (m_idxStack->Push(CONVERT_ID_TO_INDEX(pSession->SessionID)) == false)
			{
				CCrashDump::Crash();
			}
		}
	}
}

void CLanServer::PrintError(st_SESSION* pSession, const wchar_t* errMsg, int errorCode)
{
	if (!(errorCode == 10004 || errorCode == 10022 || errorCode == 10053 || errorCode == 10054 || errorCode == 10058))
	{
		// ERROR_INVALID_HANDLE : 6 - 잘못된 핸들(소켓) 오류
		// ERROR_SEM_TIMEOUT : 121 -
		// ERROR_NOT_FOUND : 1168 - Cancel IO 시도했는데, 취소할 IO 작업이 없는 경우.
		// ERROR_IO_PENDING : 997 - IO 작업으로 인해 겹쳐진 작업은 나중에 완료됨.
		// WSAEINTR : 10004 - listenSocket을 closeSocket할때
		// WSAEFAULT : 10014 - 함수에 잘못된 포인터 인자를 전달한 경우. WSABUF 안의 값이 바뀐 경우도 발생.
		// WSAEINVAL : 10022 - IO 작업 중. Shutdown 함수를 호출한 경우 발생
		// WSAEWOULDBLOCK : 10035 - 블락을 걸리는 경우.
		// WSAENOTSOCK : 10038 - 잘못된 소켓인 경우
		// WSAECONNABORTED : 10053 - 데이터 정송 제한 시간 or 프로토콜 오류로 인해 연결 중단.(내부에서 연결 끊은 경우)
		// WSAECONNRESET: 10054 - RST를 받은 상황
		// WSAENOBUF : 10055 - nonpagedpool을 할당할 수 없거나, 백로그큐를 다 사용한 경우
		// WSAESHUTDOWN : 10058 - shutdown으로 종료된 이후에 Recv, Send로 보내려고 했을때 발생하는 신호

		OnError(pSession->IpStr, pSession->usPort, errorCode, errMsg);
	}
}

st_SESSION* CLanServer::AcquireLock(unsigned long long SessionID)
{
	st_SESSION* pSession = &m_SessionArray[CONVERT_ID_TO_INDEX(SessionID)];
	if (1 == InterlockedIncrement16(&pSession->IOCount))
	{
		DEBUG(DEBUG_LOCATION::ACQUIRE_ADD_MISTAKE, pSession);
		DecreaseIOCount(pSession);
		return nullptr;
	}

	if (true == pSession->releaseFlag)
	{
		DEBUG(DEBUG_LOCATION::ACQUIRE_ALREADY_RELEASED, pSession);
		DecreaseIOCount(pSession);
		return nullptr;
	}

	if (pSession->SessionID != SessionID)
	{
		DEBUG(DEBUG_LOCATION::ACQUIRE_NOT_MY_SESSION, pSession);
		DecreaseIOCount(pSession);
		return nullptr;
	}

	if (pSession->isShutdown == true)
	{
		DecreaseIOCount(pSession);
		return nullptr;
	}

	DEBUG(DEBUG_LOCATION::ACQUIRE_SUCCESS, pSession);
	return pSession;
}

void CLanServer::ReleaseLock(st_SESSION* pSession)
{
	DEBUG(DEBUG_LOCATION::RELEASE_BEGIN, pSession);
	DecreaseIOCount(pSession);
}

#ifdef MY_DEBUG
void DebugCheck(DEBUG_LOCATION location, st_SESSION* pSession)
{
	OVERLAPPEDEX* pOverlapped = (OVERLAPPEDEX*)TlsGetValue(pSession->tlsIndex);

	unsigned int debugID = InterlockedIncrement(&pSession->debugID);

	if (pOverlapped == nullptr)
	{
		unsigned int idx = InterlockedIncrement(&pSession->debugAcceptCount) % 100;
		pSession->debugAcceptArr[idx].workSocket = pSession->Socket;
		pSession->debugAcceptArr[idx].workID = pSession->SessionID;

		pSession->debugAcceptArr[idx].threadID = GetCurrentThreadId();
		pSession->debugAcceptArr[idx].location = location;
		pSession->debugAcceptArr[idx].IOCount = pSession->IOCount;
		pSession->debugAcceptArr[idx].IOSend = pSession->IOSend;
		pSession->debugAcceptArr[idx].isShutdown = pSession->isShutdown;
		pSession->debugAcceptArr[idx].RecvQSize = pSession->RecvQ->GetUseSize();
		pSession->debugAcceptArr[idx].SendQSize = pSession->SendQ->GetSize();
		pSession->debugAcceptArr[idx].releaseFlag = pSession->releaseFlag;
		pSession->debugAcceptArr[idx].debugID = debugID;
	}
	else
	{
		bool isRecv = pOverlapped->recvFlags;

		if (isRecv)
		{
			unsigned int idx = InterlockedIncrement(&pSession->debugRecvCount) % 100;

			pSession->debugRecvArr[idx].threadID = GetCurrentThreadId();
			pSession->debugRecvArr[idx].location = location;
			pSession->debugRecvArr[idx].IOCount = pSession->IOCount;
			pSession->debugRecvArr[idx].IOSend = pSession->IOSend;
			pSession->debugRecvArr[idx].isShutdown = pSession->isShutdown;
			pSession->debugRecvArr[idx].RecvQSize = pSession->RecvQ->GetUseSize();
			pSession->debugRecvArr[idx].SendQSize = pSession->SendQ->GetSize();
			pSession->debugRecvArr[idx].releaseFlag = pSession->releaseFlag;
			pSession->debugRecvArr[idx].debugID = debugID;
		}
		else
		{
			unsigned int idx = InterlockedIncrement(&pSession->debugSendCount) % 100;

			pSession->debugSendArr[idx].threadID = GetCurrentThreadId();
			pSession->debugSendArr[idx].location = location;
			pSession->debugSendArr[idx].IOCount = pSession->IOCount;
			pSession->debugSendArr[idx].IOSend = pSession->IOSend;
			pSession->debugSendArr[idx].isShutdown = pSession->isShutdown;
			pSession->debugSendArr[idx].RecvQSize = pSession->RecvQ->GetUseSize();
			pSession->debugSendArr[idx].SendQSize = pSession->SendQ->GetSize();
			pSession->debugSendArr[idx].releaseFlag = pSession->releaseFlag;
			pSession->debugSendArr[idx].debugID = debugID;
		}
	}
}
#endif