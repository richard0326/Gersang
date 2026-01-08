#include "stdafx.h"
#include "MMOServer.h"

CMMOServer::st_SESSION::st_SESSION()
{
}
CMMOServer::st_SESSION::~st_SESSION()
{
}

void CMMOServer::st_SESSION::SendPacket(CSerializeBuffer* pSendPacket)
{
	pSendPacket->IncreaseRefCount();
#ifdef MY_DEBUG
	pSendPacket->SetLog(10, Index);
#endif
	pSendPacket->SetNetHeader();
	if (false == SendQ->Enqueue(pSendPacket))
	{
		LOG(L"Debug", en_LOG_LEVEL::LEVEL_DEBUG, L"[ IP %s, Port %d ] SendQ.Enqueue 1 Error", IpStr, usPort);

#ifdef MY_DEBUG
		pSendPacket->SetLog(11, Index);
#endif
		pSendPacket->DecreaseRefCount();
		DebugCheck(DEBUG_LOCATION::DISCONNECT_SENDPACKET_FAIL);
		Disconnect();
		return ;
	}
}

void CMMOServer::st_SESSION::DecreaseIOCount(void)
{
	short retIOCount = InterlockedDecrement16(&IOCount);
	if (retIOCount == 0)
	{
		DebugCheck(DEBUG_LOCATION::DISCONNECT_IO_COUNT);
		Disconnect();
	}
}

void CMMOServer::st_SESSION::Disconnect(void)
{
	DebugCheck(DEBUG_LOCATION::CALL_DISCONNECT);

	if (isShutdown == true)
		return;

	isShutdown = true;
	if (CancelIoEx((HANDLE)Socket, nullptr) == 0)
	{
		int WSAError = WSAGetLastError();
		if (WSAError != ERROR_NOT_FOUND)
		{
			LOG(L"CandelIOError", en_LOG_LEVEL::LEVEL_ERROR, L"Disconnect2() Error ", WSAError);
		}
	}
	shutdown(Socket, SD_BOTH);

	DisconnectFlag = true;
}

void CMMOServer::st_SESSION::SetMode_Game()
{
	ModeToGameFlag = true;
}

void CMMOServer::st_SESSION::DebugCheck(DEBUG_LOCATION location)
{
#ifdef MY_DEBUG
	OVERLAPPEDEX* pOverlapped = (OVERLAPPEDEX*)TlsGetValue(tlsIndex);

	unsigned int _debugID = InterlockedIncrement(&debugID);

	if (pOverlapped == nullptr)
	{
		unsigned int idx = InterlockedIncrement(&debugNonIOCount) % 100;
		debugNonIOArr[idx].workSocket = Socket;
		debugNonIOArr[idx].workID = SessionID;
			
		debugNonIOArr[idx].threadID = GetCurrentThreadId();
		debugNonIOArr[idx].location = location;
		debugNonIOArr[idx].IOCount = IOCount;
		debugNonIOArr[idx].IOSend = IOSend;
		debugNonIOArr[idx].isShutdown = isShutdown;
		debugNonIOArr[idx].RecvQSize = RecvQ->GetUseSize();
		debugNonIOArr[idx].SendQSize = SendQ->GetSize();
		debugNonIOArr[idx].CompQSize = CompleteRecvQ->size();
		debugNonIOArr[idx].debugID = _debugID;
		debugNonIOArr[idx].State = State;
		debugNonIOArr[idx].ModeToGameFlag = ModeToGameFlag;
		debugNonIOArr[idx].DisconnectFlag = DisconnectFlag;
	}
	else
	{
		bool isRecv = pOverlapped->recvFlags;

		if (isRecv)
		{
			unsigned int idx = InterlockedIncrement(&debugRecvCount) % 100;

			debugRecvArr[idx].threadID = GetCurrentThreadId();
			debugRecvArr[idx].location = location;
			debugRecvArr[idx].IOCount = IOCount;
			debugRecvArr[idx].IOSend = IOSend;
			debugRecvArr[idx].isShutdown = isShutdown;
			debugRecvArr[idx].RecvQSize = RecvQ->GetUseSize();
			debugRecvArr[idx].SendQSize = SendQ->GetSize();
			debugRecvArr[idx].CompQSize = CompleteRecvQ->size();
			debugRecvArr[idx].debugID = _debugID;
			debugRecvArr[idx].State = State;
			debugRecvArr[idx].ModeToGameFlag = ModeToGameFlag;
			debugRecvArr[idx].DisconnectFlag = DisconnectFlag;
		}
		else
		{
			unsigned int idx = InterlockedIncrement(&debugSendCount) % 100;

			debugSendArr[idx].threadID = GetCurrentThreadId();
			debugSendArr[idx].location = location;
			debugSendArr[idx].IOCount = IOCount;
			debugSendArr[idx].IOSend = IOSend;
			debugSendArr[idx].isShutdown = isShutdown;
			debugSendArr[idx].RecvQSize = RecvQ->GetUseSize();
			debugSendArr[idx].SendQSize = SendQ->GetSize();
			debugSendArr[idx].CompQSize = CompleteRecvQ->size();
			debugSendArr[idx].debugID = _debugID;
			debugSendArr[idx].State = State;
			debugSendArr[idx].ModeToGameFlag = ModeToGameFlag;
			debugSendArr[idx].DisconnectFlag = DisconnectFlag;
		}
	}
#endif
}

CMMOServer::CMMOServer()
{
	CSerializeBuffer::Init();
}

CMMOServer::~CMMOServer()
{

}

bool CMMOServer::Init()
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

void CMMOServer::Release()
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
bool CMMOServer::Start(const wchar_t* ipWstr, int portNum,
	int workerCreateCnt, int workerRunningCnt,
	bool bNoDelayOpt, int serverMaxUser, bool bRSTOpt, bool bKeepAliveOpt, bool bOverlappedSend, 
	int SendQSize, int RingBufferSize, int CompleteRecvQSize, int SocketQSize)
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

	LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"WorkerCreate : %d, WorkerRunning : %d, SendQSize : %d",
		m_workerCreateCnt, m_workerRunningCnt, SendQSize);

	m_idxStack = new CLockFreeStack<int>(m_MaxSession);
	for (int i = m_MaxSession - 1; i >= 0; --i)
	{
		m_ppSessionArray[i]->State = en_SESSION_STATE::MODE_NONE;
		m_ppSessionArray[i]->SendQ = new CLockFreeQueue<CSerializeBuffer*>(SendQSize);
		m_ppSessionArray[i]->RecvQ = new CRingBuffer(RingBufferSize);
		m_ppSessionArray[i]->CompleteRecvQ = new CQueue<CSerializeBuffer*>(CompleteRecvQSize);
		m_idxStack->Push(i);
	}

	m_pSocketQueue = new CLockFreeQueue<SOCKET>(SocketQSize);

	m_hIOThread = new HANDLE[m_workerCreateCnt];
	for (int i = 0; i < m_workerCreateCnt; ++i)
	{
		m_hIOThread[i] = (HANDLE)_beginthreadex(nullptr, 0, CMMOServer::netIOThread, this, 0, nullptr);
	}

	m_hAuthThread = (HANDLE)_beginthreadex(nullptr, 0, CMMOServer::netAuthThread, this, 0, nullptr);
	m_hGameThread = (HANDLE)_beginthreadex(nullptr, 0, CMMOServer::netGameThread, this, 0, nullptr);
	m_hSendThread = (HANDLE)_beginthreadex(nullptr, 0, CMMOServer::netSendThread, this, 0, nullptr);
	m_hAcceptThread = (HANDLE)_beginthreadex(nullptr, 0, CMMOServer::netAcceptThread, this, 0, nullptr);
	m_hMonitoringThread = (HANDLE)_beginthreadex(nullptr, 0, CMMOServer::monitoringThread, this, 0, nullptr);

	LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"Server Start...");

	return true;
}

void CMMOServer::Stop()
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
		closesocket(m_ppSessionArray[i]->Socket);
		delete m_ppSessionArray[i]->SendQ;
		delete m_ppSessionArray[i]->RecvQ;
	}

	if (m_MaxSession == 1)
	{
		delete m_ppSessionArray;
	}
	else
	{
		delete[] m_ppSessionArray;
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

unsigned __stdcall CMMOServer::monitoringThread(void* pParameter)
{
	CMMOServer* pMMOServer = (CMMOServer*)pParameter;
	srand((unsigned int)GetCurrentThreadId());

	int* pWorkerCount = &pMMOServer->m_workerRunningCnt;
	int* pCreateCount = &pMMOServer->m_workerCreateCnt;
	int* pSessionCount = &pMMOServer->m_SessionCnt;
	long long* pAcceptCnt = &pMMOServer->m_acceptCount;
	long long* pAcceptPrevCnt = &pMMOServer->m_acceptPrevCount;

	long long* pAuthCnt = &pMMOServer->m_authCount;
	long long* pAuthPrevCnt = &pMMOServer->m_authPrevCount;

	long long* pGameCnt = &pMMOServer->m_gameCount;
	long long* pGamePrevCnt = &pMMOServer->m_gamePrevCount;

	int* pTlsCount = &pMMOServer->m_tlsCount;
	long long* pRecvArr = pMMOServer->m_pTlsRecvTpsArr;
	long long* pRecvPrevArr = pMMOServer->m_pSaveRecvTpsArr;
	long long* pSendArr = pMMOServer->m_pTlsSendTpsArr;
	long long* pSendPrevArr = pMMOServer->m_pSaveSendTpsArr;

#ifdef MY_DEBUG
	long long* pDisconnectCnt = &pMMOServer->m_DisconnectCnt;

	long long* pRecvBPS = pMMOServer->m_pRecvBytePerSecArr;
	long long* pRecvPrevBPS = pMMOServer->m_pSaveRecvBytePerSecArr;
	long long* pSendBPS = pMMOServer->m_pSendBytePerSecArr;
	long long* pSendPrevBPS = pMMOServer->m_pSaveSendBytePerSecArr;
#endif

	CLockFreeTlsPoolA<CSerializeBuffer>* pPacketTlsPool = pMMOServer->m_packetTlsPool;

	long long RecvTpsSum = 0;
	long long SendTpsSum = 0;
	while (true)
	{
		long long temp = *pAcceptCnt;
		long long AcceptTps = temp - *pAcceptPrevCnt;
		*pAcceptPrevCnt = temp;

		temp = *pAuthCnt;
		long long AuthTps = temp - *pAuthPrevCnt;
		*pAuthPrevCnt = temp;

		temp = *pGameCnt;
		long long GameTps = temp - *pGamePrevCnt;
		*pGamePrevCnt = temp;

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

		pMMOServer->GetMonitoringInfo(AcceptTps, RecvTpsSum, SendTpsSum, 
			AuthTps, GameTps,
			*pAcceptCnt, *pDisconnectCnt, *pSessionCount,
			pPacketTlsPool->GetChunkSize(), pPacketTlsPool->GetNodeSize(),
			SendBytePerSec, RecvBytePerSec);
#else
		pMMOServer->GetMonitoringInfo(AcceptTps, RecvTpsSum, SendTpsSum,
			AuthTps, GameTps,
			*pAcceptCnt, 0, *pSessionCount,
			pPacketTlsPool->GetChunkSize(), 0,
			0, 0);
#endif
		Sleep(1000);
	}

	return 0;
}

unsigned __stdcall CMMOServer::netAcceptThread(void* pParameter)
{
	CMMOServer* pMMOServer = (CMMOServer*)pParameter;
	srand((unsigned int)GetCurrentThreadId());

	while (true)
	{
		sockaddr_in clientaddr;
		int clientLen = sizeof(clientaddr);
		SOCKET clientSocket = accept(pMMOServer->m_ListenSocket, (sockaddr*)&clientaddr, &clientLen);
		if (clientSocket == INVALID_SOCKET)
		{
			LOG(L"Error", en_LOG_LEVEL::LEVEL_ERROR, L"accept() Error : %d ", WSAGetLastError());
			return 0;
		}

		// 송신버퍼 0으로 해서 비동기 방식으로 작동
		if (pMMOServer->m_bOverlappedSend == true)
		{
			int sendSize = 0;
			setsockopt(clientSocket, SOL_SOCKET, SO_SNDBUF, (char*)&sendSize, sizeof(int));
		}

		// AcceptTps
		pMMOServer->m_acceptCount++;
		pMMOServer->m_pSocketQueue->Enqueue(clientSocket);
	}

	return 0;
}

unsigned __stdcall CMMOServer::netIOThread(void* pParameter)
{
	CMMOServer* pMMOServer = (CMMOServer*)pParameter;
	srand((unsigned int)GetCurrentThreadId());

	while (1)
	{
		DWORD transferred = 0;
		st_SESSION* pSession = nullptr;
		OVERLAPPEDEX* pOverlapped = nullptr;
		GetQueuedCompletionStatus(pMMOServer->m_hIOCP, &transferred, (PULONG_PTR)&pSession, (LPOVERLAPPED*)&pOverlapped, INFINITE);

		if (pOverlapped == nullptr)
		{
			// 서버 종료
			LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"Overlapped IO nullptr : Exit Server ");
			return 0;
		}

#ifdef MY_DEBUG
		pSession->tlsIndex = pMMOServer->m_tlsDebugIndex;
		if (TlsSetValue(pSession->tlsIndex, (LPVOID)pOverlapped) == false)
		{
			LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"TLS Fail : Exit Server ");
			CCrashDump::Crash();
			return 0;
		}
#endif

		if (transferred == 0)
		{
			pSession->Disconnect();
			// 세션 종료
			if (pOverlapped->recvFlags == false)
			{
				InterlockedExchange8((char*)&pSession->IOSend, 0);
				pSession->DebugCheck(DEBUG_LOCATION::IO_TRANS_SEND_ZERO);
			}
			else
			{
				pSession->DebugCheck(DEBUG_LOCATION::IO_TRANS_RECV_ZERO);
			}
		}
		else
		{
			if (pOverlapped->recvFlags == true)
			{
				pSession->DebugCheck(DEBUG_LOCATION::IO_RECV_COMPLETE);
				if (pSession->RecvQ->MoveRear(transferred) == false)
				{
					LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"RecvQ MoveRear Error ");
					CCrashDump::Crash();
				}
				else
				{
					pSession->DebugCheck(DEBUG_LOCATION::IO_RECV_RECVPACKET);
					int retRecv = pMMOServer->netWSARecv(pSession);
					if (retRecv != RET_SUCCESS)
					{
						pSession->DebugCheck(DEBUG_LOCATION::DISCONNECT_IO_RECVPOST_FAIL);
						pSession->Disconnect();
					}
				}
			}
			// Send Flag
			else
			{
				pSession->DebugCheck(DEBUG_LOCATION::IO_SEND_COMPLETE);
				// SendTps
				long long* pSendTps = (long long*)TlsGetValue(pMMOServer->m_tlsSendTpsIndex);
				if (pSendTps == nullptr)
				{
					long long index = (long long)TlsGetValue(pMMOServer->m_tlsIndex);
					if (index == 0)
					{
						index = InterlockedIncrement((LONG*)&pMMOServer->m_tlsCount);
						TlsSetValue(pMMOServer->m_tlsIndex, (LPVOID)index);
					}
					pSendTps = &pMMOServer->m_pTlsSendTpsArr[index];
					TlsSetValue(pMMOServer->m_tlsSendTpsIndex, pSendTps);
				}
				for (int i = 0; i < pSession->PacketArrLen; i++)
				{
					CSerializeBuffer* pPacket = (CSerializeBuffer*)pSession->pPacketArr[i];
#ifdef MY_DEBUG
					pPacket->SetLog(13, pSession->Index);
#endif
					pPacket->DecreaseRefCount();
					(*pSendTps)++;
				}
				pSession->PacketArrLen = 0;

				InterlockedExchange8((char*)&pSession->IOSend, 0);
			}
		}

		pSession->DecreaseIOCount();
	}

	return 0;
}

unsigned __stdcall CMMOServer::netAuthThread(void* pParameter)
{
	CMMOServer* pMMOServer = (CMMOServer*)pParameter;
	srand((unsigned int)GetCurrentThreadId());

	int max = pMMOServer->m_MaxSession;
	st_SESSION** ppSessionArray = pMMOServer->m_ppSessionArray;
	CLockFreeQueue<SOCKET>* pSocketQ = pMMOServer->m_pSocketQueue;

	SOCKET outSocket;
	for (;;)
	{
		// MODE_NONE에 대한 처리
		for (;;)
		{
			if (pSocketQ->Dequeue(&outSocket) == false)
				break;

			st_SESSION* pSession = pMMOServer->CreateSession(outSocket);
			if (pSession == nullptr)
			{
				// 인원이 초과한 경우
				LOG(L"Error", en_LOG_LEVEL::LEVEL_ERROR, L"MaxUser Login Fail");
				closesocket(outSocket);
				continue;
			}

			if (pMMOServer->OnConnectionRequest(pSession->IpStr, pSession->usPort) == false)
			{
				LOG(L"Error", en_LOG_LEVEL::LEVEL_ERROR, L"[ IP %s, Port %d ] Banned ", pSession->IpStr, pSession->usPort);
				pSession->Disconnect();
				continue;
			}

			CreateIoCompletionPort((HANDLE)outSocket, pMMOServer->m_hIOCP, (ULONG_PTR)pSession, 0);

			pSession->DebugCheck(DEBUG_LOCATION::AUTH_CREATESESSION);
			pSession->OnAuth_ClientJoin();
			pSession->State = en_SESSION_STATE::MODE_AUTH;

			int retRecvPost = pMMOServer->netWSARecvPost(pSession);
			if (retRecvPost != RET_SUCCESS)
			{
				pSession->DebugCheck(DEBUG_LOCATION::DISCONNECT_CREATE_RECVPOST_FAIL);
				pSession->Disconnect();
			}
		}

		// MODE_AUTH, ModeToGameFlag 처리
		for (int idx = 0; idx < max; ++idx)
		{
			st_SESSION* pSession = ppSessionArray[idx];
			if (pSession->State == en_SESSION_STATE::MODE_AUTH)
			{
				for (;;)
				{
					CSerializeBuffer* compPacket;
					if (pSession->CompleteRecvQ->pop_front(&compPacket) == false)
					{
						break;
					}

					if (pSession->OnAuth_Packet(compPacket) == false)
					{
						compPacket->DecreaseRefCount();
						break;
					}
					compPacket->DecreaseRefCount();
				}

				if (pSession->DisconnectFlag == true)
				{
					pSession->DebugCheck(DEBUG_LOCATION::AUTH_DISCONNECT);
				
					for (;;)
					{
						CSerializeBuffer* pPacket = nullptr;
						if (pSession->CompleteRecvQ->pop_front(&pPacket) == false)
							break;

#ifdef MY_DEBUG
						pPacket->SetLog(16, pSession->Index);
#endif
						pPacket->DecreaseRefCount();
					}
					
					pSession->OnAuth_ClientLeave();
					pSession->State = en_SESSION_STATE::MODE_AUTH_TO_RELEASE;
					continue;
				}

				if (pSession->ModeToGameFlag == true)
				{
					pSession->DebugCheck(DEBUG_LOCATION::AUTH_AUTH2GAME);
					pSession->ModeToGameFlag = false;
					pSession->OnAuth_ClientLeave();
					pSession->State = en_SESSION_STATE::MODE_AUTH_TO_GAME;
				}
			}			
		}

		pMMOServer->m_authCount++;
		Sleep(10);
	}

	return 0;
}

unsigned __stdcall CMMOServer::netGameThread(void* pParameter)
{
	CMMOServer* pMMOServer = (CMMOServer*)pParameter;
	srand((unsigned int)GetCurrentThreadId());

	int max = pMMOServer->m_MaxSession;
	st_SESSION** ppSessionArray = pMMOServer->m_ppSessionArray;
	CLockFreeStack<int>* pIdxStack = pMMOServer->m_idxStack;
	for (;;)
	{
		// AUTH_TO_GAME, GAME_TO_RELEASE 처리
		for (int idx = 0; idx < max; ++idx)
		{
			st_SESSION* pSession = ppSessionArray[idx];
			if (pSession->State == en_SESSION_STATE::MODE_AUTH_TO_GAME)
			{
				pSession->DebugCheck(DEBUG_LOCATION::GAME_AUTH2GAME);
				pSession->OnGame_ClientJoin();
				pSession->State = en_SESSION_STATE::MODE_GAME;
			}

			if (pSession->State == en_SESSION_STATE::MODE_AUTH_TO_RELEASE && pSession->IOSend == false)
			{
				pSession->DebugCheck(DEBUG_LOCATION::GAME_AUTH2REL);
				pSession->State = en_SESSION_STATE::MODE_RELEASE;
			}

			if (pSession->State == en_SESSION_STATE::MODE_GAME_TO_RELEASE && pSession->IOSend == false)
			{
				pSession->DebugCheck(DEBUG_LOCATION::GAME_GAME2REL);
				pSession->State = en_SESSION_STATE::MODE_RELEASE;
			}
		}

		// MODE_GAME, RELEASE 처리
		for (int idx = 0; idx < max; ++idx)
		{
			st_SESSION* pSession = ppSessionArray[idx];
			if (pSession->State == en_SESSION_STATE::MODE_GAME)
			{
				for (;;)
				{
					CSerializeBuffer* compPacket;
					if (pSession->CompleteRecvQ->pop_front(&compPacket) == false)
					{
						break;
					}

					if (pSession->OnGame_Packet(compPacket) == false)
					{
						compPacket->DecreaseRefCount();
						break;
					}
					compPacket->DecreaseRefCount();
				}

				if (pSession->DisconnectFlag == true)
				{
					pSession->DebugCheck(DEBUG_LOCATION::GAME_DISCONNECT);

					for (;;)
					{
						CSerializeBuffer* pPacket = nullptr;
						if (pSession->CompleteRecvQ->pop_front(&pPacket) == false)
							break;

#ifdef MY_DEBUG
						pPacket->SetLog(16, pSession->Index);
#endif
						pPacket->DecreaseRefCount();
					}

					pSession->OnGame_ClientLeave();
					pSession->State = en_SESSION_STATE::MODE_GAME_TO_RELEASE;
					continue;
				}
			}

			if (pSession->State == en_SESSION_STATE::MODE_RELEASE)
			{
				pSession->DebugCheck(DEBUG_LOCATION::GAME_RELEASESESSION_BEGIN);
				pMMOServer->ReleaseSession(pSession);
				pSession->State = en_SESSION_STATE::MODE_NONE;
				pSession->DebugCheck(DEBUG_LOCATION::GAME_RELEASESESSION_END);
				pIdxStack->Push(pSession->Index);
			}
		}

		pMMOServer->m_gameCount++;
		Sleep(10);
	}

	return 0;
}

unsigned __stdcall CMMOServer::netSendThread(void* pParameter)
{
	CMMOServer* pMMOServer = (CMMOServer*)pParameter;
	srand((unsigned int)GetCurrentThreadId());

	int max = pMMOServer->m_MaxSession;
	st_SESSION** ppSessionArray = pMMOServer->m_ppSessionArray;
	
	for (;;)
	{
		for (int idx = 0; idx < max; ++idx)
		{
			st_SESSION* pSession = ppSessionArray[idx];
			if (pSession->State == en_SESSION_STATE::MODE_AUTH || pSession->State == en_SESSION_STATE::MODE_GAME)
			{
				if (pMMOServer->netWSASendPost(pSession) == RET_SOCKET_ERROR)
				{
					pSession->DebugCheck(DEBUG_LOCATION::DISCONNECT_SENDPOST_FAIL);
					pSession->Disconnect();
				}
			}
		}

		Sleep(100);
	}

	return 0;
}

CMMOServer::st_SESSION* CMMOServer::CreateSession(SOCKET clientSocket)
{
	int outIdx;
	if (m_idxStack->Pop(&outIdx) == false)
	{
		return nullptr;
	}

	st_SESSION* pSession = m_ppSessionArray[outIdx];
	if (pSession->State != en_SESSION_STATE::MODE_NONE)
	{
		int* crash = nullptr;
		*crash = 0;
	}

	pSession->isShutdown = false;
	pSession->Socket = clientSocket;
	pSession->SessionID = m_SessionID;
	pSession->RecvQ->ClearBuffer();
	pSession->PacketArrLen = 0;
	InterlockedExchange8((char*)&pSession->IOSend, 0);

	pSession->RecvOverlappedEx.recvFlags = true;
	pSession->SendOverlappedEx.recvFlags = false;

	sockaddr_in clientaddr;
	int clientLen = sizeof(clientaddr);
	getpeername(clientSocket, (sockaddr*)&clientaddr, &clientLen);
	InetNtop(AF_INET, &clientaddr.sin_addr, pSession->IpStr, 16);
	pSession->usPort = ntohs(clientaddr.sin_port);
	pSession->ModeToGameFlag = false;
	pSession->DisconnectFlag = false;
	pSession->Index = outIdx;

	++m_SessionID;

	InterlockedIncrement((LONG*)&m_SessionCnt);

	return pSession;
}

void CMMOServer::ReleaseSession(st_SESSION* pSession)
{
	int len = pSession->PacketArrLen;
	CSerializeBuffer** pPacketArr = pSession->pPacketArr;
	for (int i = 0; i < len; ++i)
	{
		CSerializeBuffer* pPacket = pPacketArr[i];
#ifdef MY_DEBUG
		pPacket->SetLog(14, pSession->Index);
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
		pPacket->SetLog(15, pSession->Index);
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
	pSession->OnGame_Release();
	InterlockedDecrement((LONG*)&m_SessionCnt);
}

int CMMOServer::netWSARecv(st_SESSION* pSession)
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
		char Header[5];

		int headerSize = NETHEADER;
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

		int peekSize = pSession->RecvQ->Peek((char*)Header, headerSize);
		if (peekSize != headerSize)
		{
			CCrashDump::Crash();
			return RET_SOCKET_ERROR;
		}

		short* pPayloadSize = (short*)&Header[1];
		// ringbuffer 크기보다 작아야하며, tcp 수신 버퍼보다 작아야함.
		if (*pPayloadSize <= 2 || 65535 < *pPayloadSize)
		{
			LOG(L"RecvError", en_LOG_LEVEL::LEVEL_DEBUG, L"[ IP %s, Port %d ] PayloadSizeWrong : %d", pSession->IpStr, pSession->usPort, *pPayloadSize);
			return RET_SOCKET_ERROR;
		}

		// Packet의 최대 크기를 넘으면 할당에 실패한다.
		if (CSerializeBuffer::Check(NETHEADER, *pPayloadSize) == false)
		{
			LOG(L"RecvError", en_LOG_LEVEL::LEVEL_DEBUG, L"[ IP %s, Port %d ] RecvPacket Alloc Fail1", pSession->IpStr, pSession->usPort);
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

		CSerializeBuffer* RecvPacket = CSerializeBuffer::Alloc(NETHEADER, *pPayloadSize);
		if (RecvPacket == nullptr)
		{
			LOG(L"CSerializeBuffer", en_LOG_LEVEL::LEVEL_ERROR, L"[ IP %s, Port %d ] OnRecv Alloc Fail");
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

		if (RecvPacket->Decryption(Header) == false)
		{
			LOG(L"RecvError", en_LOG_LEVEL::LEVEL_DEBUG, L"[ IP %s, Port %d ] Decryption Fail", pSession->IpStr, pSession->usPort);
			RecvPacket->DecreaseRefCount();
			return RET_SOCKET_ERROR;
		}

#ifdef MY_DEBUG
		* pRecvBPS += 20 + deqSize + headerSize;
#endif

		(*pRecvTps)++;
		RecvPacket->IncreaseRefCount();
		if (pSession->CompleteRecvQ->push_back(RecvPacket) == false)
		{
			return RET_SOCKET_ERROR;
		}
		RecvPacket->DecreaseRefCount();
	}

	int retRecvPost = netWSARecvPost(pSession);
	if (retRecvPost != RET_SUCCESS)
	{
		return retRecvPost;
	}

	return RET_SUCCESS;
}

int CMMOServer::netWSARecvPost(st_SESSION* pSession)
{
	int afterEnqSize = 0;
	int enqSize = pSession->RecvQ->DirectEnqueueSize(&afterEnqSize);

	InterlockedIncrement16(&pSession->IOCount);

	pSession->DebugCheck(DEBUG_LOCATION::AUTH_RECVPOST);
	if (afterEnqSize == 0)
	{
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
				pSession->DecreaseIOCount();
				return RET_SOCKET_ERROR;
			}
		}
	}
	else
	{
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
				pSession->DecreaseIOCount();
				return RET_SOCKET_ERROR;
			}
		}
	}

	return RET_SUCCESS;
}

int CMMOServer::netWSASendPost(st_SESSION* pSession)
{
	pSession->DebugCheck(DEBUG_LOCATION::SEND_BEF_SENDPOST);
	if (pSession->SendQ->GetSize() == 0)
	{
		return RET_SUCCESS;
	}

	char retIOSend = InterlockedExchange8((char*)&pSession->IOSend, 1);
	if (retIOSend == 0)
	{
		// 교차 체크
		if (!(pSession->State == en_SESSION_STATE::MODE_AUTH || pSession->State == en_SESSION_STATE::MODE_GAME))
		{
			InterlockedExchange8((char*)&pSession->IOSend, 0);
			return RET_SUCCESS;
		}

		pSession->DebugCheck(DEBUG_LOCATION::SEND_SENDPOST);

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
			pPacket->SetLog(12, pSession->Index);
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

		DWORD sendBytes = 0;
		int	sendErr = WSASend(pSession->Socket, wsaSendBuf, packetLoop, &sendBytes, 0, (LPWSAOVERLAPPED)&pSession->SendOverlappedEx, nullptr);
		if (sendErr == 0)
		{
			// 동기로 Send한 경우
		}

		if (sendErr != 0)
		{
			int WSAError = WSAGetLastError();
			if (WSAError != WSA_IO_PENDING)
			{
				PrintError(pSession, L"WSASend() Error ", WSAError);
				pSession->DecreaseIOCount();
				InterlockedExchange8((char*)&pSession->IOSend, 0);

				return RET_SOCKET_ERROR;
			}
		}
	}

	return RET_SUCCESS;
}

void CMMOServer::PrintError(st_SESSION* pSession, const wchar_t* errMsg, int errorCode)
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