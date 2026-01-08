#include "stdafx.h"

// 세션 정보 구조체.
struct st_SESSION
{
	SOCKET Socket; // 현 접속의 TCP 소켓.
	DWORD dwSessionID; // 접속자의 고유 세션 ID.
	CRingBuffer RecvQ; // 수신 큐.
	CRingBuffer SendQ; // 송신 큐.

	DWORD dwRecvCount = 0;
	bool bRecvSet = false;
	DWORD dwSendCount = 0;

	DWORD dwLastRecvTime; // 메시지 수신 체크를 위한 시간 (타임아웃용)
};

//-----------------------------------------------
// 섹터 하나의 좌표 정보
//-----------------------------------------------
struct st_SECTOR_POS
{
	int iX;
	int iY;
};
//-----------------------------------------------
// 특정 위치 주변의 9개 섹터 정보
//-----------------------------------------------
struct st_SECTOR_AROUND
{
	int iCount;
	st_SECTOR_POS Around[9];
};

//---------------------------------------------------------------
// 캐릭터 정보 구조체.
//---------------------------------------------------------------
struct st_CHARACTER
{
	st_SESSION* pSession;
	DWORD dwSessionID;
	DWORD dwAction;			// 행동
	BYTE byDirection;		// 바라보는 방향
	BYTE byMoveDirection;	// 이동하는 방향
	short shX;
	short shY;

	st_SECTOR_POS CurSector; // 섹터 파트에서 설명
	st_SECTOR_POS OldSector;

	char chHP;
};

SOCKET			g_ListenSocket = INVALID_SOCKET;
sockaddr_in		g_ListenInfo;
DWORD			g_SessionCnt = 1;

map<SOCKET, st_SESSION*> g_SessionMap;

//-------------------------------------------------------------
// 캐릭터 객체 메인 관리
//-------------------------------------------------------------
map<DWORD, st_CHARACTER*> g_CharacterMap;
//-------------------------------------------------------------
// 월드맵 캐릭터 섹터
//-------------------------------------------------------------
list<st_CHARACTER*> g_Sector[dfSECTOR_MAX_Y][dfSECTOR_MAX_X];

bool g_bShutdown = false;

//------------------------------------------------------
// Log
//------------------------------------------------------
#define dfLOG_LEVEL_DEBUG 0
#define dfLOG_LEVEL_WARNING 1
#define dfLOG_LEVEL_ERROR 2
#define dfLOG_LEVEL_SYSTEM 3

int		g_iLogLevel;
WCHAR	g_szLogBuff[1024];

#define _LOG(LogLevel, fmt, ...)					\
do {												\
	if (g_iLogLevel <= LogLevel)					\
	{												\
		wsprintf(g_szLogBuff, fmt, ##__VA_ARGS__);	\
		Log(g_szLogBuff, LogLevel);					\
	}												\
} while (0)

//------------------------------------------------------
// Network
//------------------------------------------------------
// 네트워크 초기화
bool netStartUp();
// 네트워크 마무리
void netCleanUp();
// 서버 메인 네트워크 처리 함수
void netIOProcess(void);
// select 모델 체크 함수.
void netSelectSocket(SOCKET* pTableSocket, FD_SET* pReadSet, FD_SET* pWriteSet);
// Recv 처리.
void netProc_Recv(SOCKET Socket);
// Send 처리.
bool netProc_Send(SOCKET Socket);
// 사용자 접속 이벤트 처리.
bool netProc_Accept(void);
// 패킷이 완료되었는지 확인후 완료 되었다면 패킷을 처리한다.
int CompleteRecvPacket(st_SESSION* pSession);
// 패킷 타입에 따른 처리 함수 호출.
bool PacketProc(st_SESSION* pSession, BYTE byPacketType, CSerializeBuffer* pPacket);

//------------------------------------------------------
// Session
//------------------------------------------------------
// 새로운 세션을 생성,등록 한다.
st_SESSION* CreateSession(SOCKET Socket);
// Socket 으로 세션 찾기.
st_SESSION* FindSession(SOCKET Socket);
// 해당 세션 종료처리.
void DisconnectSession(SOCKET Socket);
void DisconnectSession(st_SESSION* pSession);

//------------------------------------------------------
// Send Packet
//------------------------------------------------------
// 특정 섹터 1개에 있는 클라이언트들 에게 메시지 보내기
void SendPacket_SectorOne(int iSectorX, int iSectorY, CSerializeBuffer* pPacket, st_SESSION* pExceptSession);
// 특정 1명의 클라이언트 에게 메시지 보내기
void SendPacket_Unicast(st_SESSION* pSession, CSerializeBuffer* pPacket);
// 클라이언트 기준 주변 섹터에 메시지 보내기 (최대 9개 영역)
void SendPacket_Around(st_SESSION* pSession, CSerializeBuffer* pPacket, bool bSendMe = false);
// 진정 브로드 캐스팅 (시스템적인 메시지 외에는 사용하지 않음)
void SendPacket_Broadcast(CSerializeBuffer* pPacket);

//------------------------------------------------------
// netPacket
//------------------------------------------------------
bool netPacketProc_MoveStart(st_SESSION* pSession, CSerializeBuffer* pPacket);
bool netPacketProc_MoveStop(st_SESSION* pSession, CSerializeBuffer* pPacket);
bool netPacketProc_Attack1(st_SESSION* pSession, CSerializeBuffer* pPacket);
bool netPacketProc_Attack2(st_SESSION* pSession, CSerializeBuffer* pPacket);
bool netPacketProc_Attack3(st_SESSION* pSession, CSerializeBuffer* pPacket);
bool netPacketProc_Echo(st_SESSION* pSession, CSerializeBuffer* pPacket);

//------------------------------------------------------
// main
//------------------------------------------------------
bool LoadData();
void SaveData();
void Update();
void ServerControl();

//------------------------------------------------------
// Character
//------------------------------------------------------
// 새로운 세션을 생성,등록 한다.
st_CHARACTER* CreateCharacter(st_SESSION * pSession);
// 해당 세션 종료처리.
void DisconnectCharacter(DWORD dwSesssionID);
// 캐릭터 찾기
st_CHARACTER* FindCharacter(DWORD dwSesssionID);
// 캐릭터 이동 가능한지 체크하기
bool CharacterMoveCheck(short shX, short shY);

//------------------------------------------------------
// Log
//------------------------------------------------------
void Log(WCHAR* szString, int iLogLevel);

//------------------------------------------------------
// Sector
//------------------------------------------------------
// 캐릭터의 현재 좌표 shX, shY 으로 섹터 위치를 계산하여 해당 섹터에 넣음
void Sector_AddCharacter(st_CHARACTER* pCharacter);
// 캐릭터의 현재 좌표 shX, shY 으로 섹터를 계산하여 해당 섹터에서 삭제
void Sector_RemoveCharacter(st_CHARACTER* pCharacter);
// 위의 RemoveCharacter, AddCharacter 를 사용하여
// 현재 위치한 섹터에서 삭제 후 현재의 좌표로 섹터를 새롭게 계산하여 해당 섹터에 넣음.
bool Sector_UpdateCharacter(st_CHARACTER* pCharacter);

void CharacterSectorUpdatePacket(st_CHARACTER* pCharacter);

void GetCollisionRect(LPRECT outRc, st_CHARACTER * pCharacter);

// 특정 섹터 좌표 기준 주변 영향권 섹터 얻기.
void GetSectorAround(int iSectorX, int iSectorY, st_SECTOR_AROUND* pSectorAround);
// 충돌 범위 안의 섹터 가져오기
void GetCollisionSectorAround(LPRECT colRc, st_SECTOR_AROUND* pSectorAround);

// 섹터에서 섹터를 이동 하였을 때 섹터 영향권에서 빠진 섹터, 새로 추가된 섹터의 정보 구하는 함수.
void GetUpdateSectorAround(st_CHARACTER* pCharacter, st_SECTOR_AROUND* pRemoveSector, st_SECTOR_AROUND* pAddSector);


int main()
{
	g_iLogLevel = dfLOG_LEVEL_ERROR;
	setlocale(LC_ALL, "korean");
	if (!LoadData())	// 설정 및 게임 데이터, DB 데이터 로딩
	{
		_LOG(dfLOG_LEVEL_ERROR, L"게임 초기화 에러\n");
		return 0;
	}

	if (!netStartUp())
	{
		_LOG(dfLOG_LEVEL_ERROR, L"네트워크 초기화 에러\n");
		return 0;
	}

	// 서버는 Lisen 함수를 호출함과 동시에 실질적인 ON 상태 (접속자를 받는 상태)가 된다.
	while ( !g_bShutdown )		// 서버 메인 루프, 전역의 g_bShutdown 값에 의해 종료 결정
	{
		netIOProcess();		// 네트워크 송수신 처리

		Update();				// 게임 로직 업데이트

		ServerControl();		// 키보드 입력을 통해서 서버를 제어할 경우 사용
	}

	// 서버의 종료 대기
	
	// 서버는 함부로 종료해도 안된다.
	// DB에 저장할 데이터나 기타 마무리 할 일들이 모두 끝났는지 확인한 뒤에 꺼주어야 한다.
	SaveData();

	netCleanUp();

	return 0;
}

bool netStartUp()
{
	WSADATA wsa;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsa);
	if (iResult != NO_ERROR) {
		_LOG(dfLOG_LEVEL_ERROR, L"WSAStartup Error : %d\n", WSAGetLastError());
		return false;
	}

	g_ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (g_ListenSocket == INVALID_SOCKET) {
		_LOG(dfLOG_LEVEL_ERROR, L"socket Error : %d\n", WSAGetLastError());
		return false;
	}

	ZeroMemory(&g_ListenInfo, sizeof(g_ListenInfo));
	g_ListenInfo.sin_family = AF_INET;
	InetPton(AF_INET, dfNETWORK_IP, &g_ListenInfo.sin_addr);
	g_ListenInfo.sin_port = htons(dfNETWORK_PORT);

	iResult = bind(g_ListenSocket, (sockaddr*)&g_ListenInfo, sizeof(g_ListenInfo));
	if (iResult == SOCKET_ERROR) {
		_LOG(dfLOG_LEVEL_ERROR, L"bind Error : %d\n", WSAGetLastError());
		return false;
	}

	iResult = listen(g_ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		_LOG(dfLOG_LEVEL_ERROR, L"listen Error : %d\n", WSAGetLastError());
		return false;
	}

	u_long on = 1;
	int retval = ioctlsocket(g_ListenSocket, FIONBIO, &on);
	if (retval == SOCKET_ERROR) 
	{
		_LOG(dfLOG_LEVEL_ERROR, L"ioctlsocket Error : %d\n", WSAGetLastError());
		return false;
	}

	_LOG(dfLOG_LEVEL_SYSTEM, L"Server Open...\n");
	return true;
}

void netCleanUp()
{
	for (auto eraseIter = g_CharacterMap.begin(); eraseIter != g_CharacterMap.end();)
	{
		delete eraseIter->second;
		eraseIter = g_CharacterMap.erase(eraseIter);
	}

	for (auto eraseIter = g_SessionMap.begin(); eraseIter != g_SessionMap.end();)
	{
		closesocket(eraseIter->second->Socket);
		delete eraseIter->second;
		eraseIter = g_SessionMap.erase(eraseIter);
	}

	closesocket(g_ListenSocket);

	// 윈속 종료
	WSACleanup();

	_LOG(dfLOG_LEVEL_SYSTEM, L"Server Close...\n");
}

void netIOProcess()
{
	st_SESSION* pSession;
	SOCKET UserTable_SOCKET[FD_SETSIZE] = { INVALID_SOCKET, }; // FD_SET 에 등록된 소켓을 저장.
	int iSocketCount = 0;
	int iMaxSessionCount = 0;

	//------------------------------------------------------
	// FD_SET 은 FD_SETSIZE 만큼씩만 소켓 검사가 가능하다
	// 그러므로 그 개수만큼 넣어서 사용함
	//------------------------------------------------------
	FD_SET ReadSet;
	FD_SET WriteSet;
	FD_ZERO(&ReadSet);
	FD_ZERO(&WriteSet);

	//------------------------------------------------------
	// 리슨소켓 넣기.
	//------------------------------------------------------
	FD_SET(g_ListenSocket, &ReadSet);
	UserTable_SOCKET[iSocketCount] = g_ListenSocket;
	iSocketCount++;

	//------------------------------------------------------
	// 리슨소켓 및 접속중인 모든 클라이언트에 대해 SOCKET 을 체크한다.
	//------------------------------------------------------
	map<SOCKET, st_SESSION*>::iterator SessionIter;

	for (SessionIter = g_SessionMap.begin(); SessionIter != g_SessionMap.end(); )
	{
		pSession = SessionIter->second;
		pSession->bRecvSet = true;

		//------------------------------------------------------
		// 해당 클라이언트 Read Set 등록 / SendQ 에 데이터가 있다면 Write Set 등록
		//------------------------------------------------------
		UserTable_SOCKET[iSocketCount] = pSession->Socket;
		FD_SET(pSession->Socket, &ReadSet);
		
		if (pSession->SendQ.GetUseSize() > 0)
			FD_SET(pSession->Socket, &WriteSet);

		iSocketCount++;
		SessionIter++;
		//------------------------------------------------------
		// select 최대치 도달, 만들어진 테이블 정보로 select 호출 후 정리.
		//------------------------------------------------------
		if (FD_SETSIZE <= iSocketCount)
		{
			netSelectSocket(UserTable_SOCKET, &ReadSet, &WriteSet);
			FD_ZERO(&ReadSet);
			FD_ZERO(&WriteSet);
			memset(UserTable_SOCKET, -1, sizeof(SOCKET) * FD_SETSIZE);

			//------------------------------------------------------
			// 단일 스레드에 select 방식의 경우 소켓이 천개 이상으로 많아지고
			// 서버의 로직도 부하가 걸리는 경우 접속자 처리가 엄청 느려진다.
			//
			// 그런 이유로 64개씩 select 처리를 할 때 매번 accdpt 처리를 해주도록 한다.
			// 접속자 처리가 조금 더 원활하게 진행 된다.
			//------------------------------------------------------
			FD_SET(g_ListenSocket, &ReadSet);
			UserTable_SOCKET[0] = g_ListenSocket;
			iMaxSessionCount += iSocketCount - 1;
			iSocketCount = 1;
		}
	}

	//------------------------------------------------------
	// 전체 클라이언트 for 문 종료 후 socketCount 수치가 있다면
	// 추가적으로 마지막 Select 호출을 해준다.
	//------------------------------------------------------
	if (iSocketCount > 0)
	{
		netSelectSocket(UserTable_SOCKET, &ReadSet, &WriteSet);
		iMaxSessionCount += iSocketCount - 1;
	}

	if (iMaxSessionCount != g_SessionMap.size())
	{
		_LOG(dfLOG_LEVEL_SYSTEM, L"No Enough Loop\n");
	}
}

void netSelectSocket(SOCKET* pTableSocket, FD_SET* pReadSet, FD_SET* pWriteSet)
{
	timeval Time;
	int iResult, iCnt;
	bool bProcFlag;

	// 64개 이상의 소캣에선 본 함수를 여러 번 호출하면서 소켓을 확인 해야 하므로
	// timeout 은 0으로 해야 한다. 그렇지 않으면 뒤쪽 소켓들 검사 타이밍이 점점 늦어진다.
	Time.tv_sec = 0;
	Time.tv_usec = 0;
	iResult = select(0, pReadSet, pWriteSet, 0, &Time);

	//-----------------------------------------------------
	// 리턴값이 0 이상이라면 누군가의 데이타가 왔다.
	//-----------------------------------------------------
	if (0 < iResult)
	{
		//-----------------------------------------------------
		// TableSocket 을 돌면서 어떤 소캣에 반응이 있었는지 확인!!!!!!
		//-----------------------------------------------------
		for (iCnt = 0; iResult > 0 && iCnt < FD_SETSIZE; ++iCnt)
		{
			bProcFlag = true;
			if (pTableSocket[iCnt] == INVALID_SOCKET)
				continue;
			//-----------------------------------------------------
			// Write 체크
			//-----------------------------------------------------
			if (FD_ISSET(pTableSocket[iCnt], pWriteSet))
			{
				--iResult;
				bProcFlag = netProc_Send(pTableSocket[iCnt]);
			}
			//-----------------------------------------------------
			// Read 체크
			//-----------------------------------------------------
			if (FD_ISSET(pTableSocket[iCnt], pReadSet))
			{
				--iResult;
				//-----------------------------------------------------
				// ProcSend 부분에서 에러(접속끊김 등..)의 상황으로
				// 이미 해당 클라이언트가 접속 종료를 한 경우가 있기 때문에
				// bProcFlag 로 확인 후 Recv 진행
				//-----------------------------------------------------
				if (bProcFlag)
				{
					//-----------------------------------------------------
					// ListenSocket 은 접속차 수락용도 별도 처리
					//-----------------------------------------------------
					if (pTableSocket[iCnt] == g_ListenSocket)
					{
						netProc_Accept();
					}
					else if (pTableSocket[iCnt] != g_ListenSocket)
					{
						netProc_Recv(pTableSocket[iCnt]);
					}
				}
			}
		}
	}
	else if (iResult == SOCKET_ERROR)
	{
		_LOG(dfLOG_LEVEL_ERROR, L"select() error");
	}
}

void netProc_Recv(SOCKET Socket)
{
	st_SESSION* pSession;
	int iBuffSize;
	int iResult;
	pSession = FindSession(Socket);
	if (NULL == pSession)
		return;

	pSession->dwRecvCount++;

	//------------------------------------------------------------------
	// 마지막으로 받은 메시지 타임
	//------------------------------------------------------------------
	pSession->dwLastRecvTime = GetTickCount();

	//------------------------------------------------------------------
	// 받기 작업.
	//------------------------------------------------------------------
	iBuffSize = pSession->RecvQ.DirectEnqueueSize();
	iResult = recv(pSession->Socket, pSession->RecvQ.GetRearBufferPtr(), iBuffSize, 0);
	if (SOCKET_ERROR == iResult || 0 == iResult)
	{
		DisconnectSession(pSession->Socket);
		_LOG(dfLOG_LEVEL_ERROR, L"Recv\n");
		return;
	}

	//-------------------------------------------------------------------
	// 받은 데이타가 있다면.
	//-------------------------------------------------------------------
	if (0 < iResult)
	{
		//-------------------------------------------------------------------
		// RecvQ 에 넣은걸로~
		//-------------------------------------------------------------------
		pSession->RecvQ.MoveRear(iResult);
		//-----------------------------------------------------------------
		// 패킷이 완료 되었는지 확인한다.
		// 패킷은 하나 이상이 버퍼에 있을 수 있으므로 한번에 전부 처리해야한다.
		//-----------------------------------------------------------------
		while (1)
		{
			iResult = CompleteRecvPacket(pSession);
			if (1 == iResult) // 더이상 처리할 패킷 없음.
				break;

			if (-1 == iResult) // 패킷 처리 오류
			{
				_LOG(dfLOG_LEVEL_ERROR, L"PRError SessionID : %d \n", pSession->dwSessionID);
				return;
			}
		}
	}
}

bool netProc_Send(SOCKET Socket)
{
	st_SESSION* pSession;
	int iResult;
	int iSendSize;

	// 해당 사용자 세션 찾기
	pSession = FindSession(Socket);
	if (NULL == pSession)
		return false;

	pSession->dwSendCount++;

	// 전송한다.
	iResult = send(Socket, pSession->SendQ.GetFrontBufferPtr(), pSession->SendQ.DirectDequeueSize(), 0);

	if (SOCKET_ERROR == iResult)
	{
		DWORD dwError = WSAGetLastError();
		if (WSAEWOULDBLOCK == dwError)
		{
			// WouldBlock인 경우에도 종료가 되게끔 한다.
			_LOG(dfLOG_LEVEL_ERROR, L"/////// Socket WOULDBLOCK - UserNO:%d \n", pSession->dwSessionID);
			DisconnectSession(Socket);

			return false;
		}

		_LOG(dfLOG_LEVEL_ERROR, L"/////// Socket Error - UserNO:%d ErrorCode:%d \n", pSession->dwSessionID, dwError);
		DisconnectSession(Socket);
		return false;
	}
	else
	{
		// 송신 작업을 완료하였다.
		// 패킷이 완전히 전송되었다는 건 아니고 소켓 버퍼에 복사를 완료했다는 의미
		// 송신류에서 Peek로 빼냈던 데이터를 이제 지워주자.
		pSession->SendQ.MoveFront(iResult);
	}

	return true;
}

bool netProc_Accept(void)
{
	sockaddr_in clientaddr;
	int clientLen = sizeof(clientaddr);
	SOCKET clientSocket = accept(g_ListenSocket, (sockaddr*)&clientaddr, &clientLen);
	if (clientSocket == INVALID_SOCKET)
	{
		_LOG(dfLOG_LEVEL_ERROR, L"accept Error : %d \n", WSAGetLastError());
		return false;
	}

	st_SESSION* pSession = CreateSession(clientSocket);
	if (pSession == nullptr)
	{
		_LOG(dfLOG_LEVEL_ERROR, L"CreateSession Error \n", );
		return false;
	}

	// 로그인 과정이 없기 때문에 바로 캐릭터를 생성해준다.
	st_CHARACTER* pCharacter = CreateCharacter(pSession);
	if (pCharacter == nullptr)
	{
		_LOG(dfLOG_LEVEL_ERROR, L"CreateCharacter Error \n");
		return false;
	}

	CSerializeBuffer Packet;
	mpCreateMyCharacter(&Packet, pSession->dwSessionID, 
		pCharacter->byDirection,
		pCharacter->shX, 
		pCharacter->shY, 
		pCharacter->chHP);

	SendPacket_Unicast(pSession, &Packet);

	Sector_AddCharacter(pCharacter);

	mpCreateOtherCharacter(&Packet, pSession->dwSessionID,
		pCharacter->byDirection,
		pCharacter->shX,
		pCharacter->shY,
		pCharacter->chHP);

	SendPacket_Around(pSession, &Packet);

	WCHAR szClient[16] = { 0, };
	InetNtop(AF_INET, &clientaddr.sin_addr, szClient, 16);

	_LOG(dfLOG_LEVEL_DEBUG, L"/// Accept Success /// IP : %s, Port : %d, Session ID : %d \n",
		szClient,
		ntohs(clientaddr.sin_port),
		pSession->dwSessionID);

	return true;
}

int CompleteRecvPacket(st_SESSION* pSession)
{
	st_PACKET_HEADER stPacketHeader;
	int iRecvQSize;
	iRecvQSize = pSession->RecvQ.GetUseSize();

	// 받은 내용을 검사해야한다. 그런데 패킷헤더 크기 이상인 경우에만!
	if (sizeof(st_PACKET_HEADER) > iRecvQSize)
		return 1;

	//-----------------------------------------------------------------
	// 1. PacketCode 검사.
	// Peek 으로 검사를 하는 이유는, 헤더를 얻은후 사이즈 비교후에 하나의 완성된 패킷만큼의
	// 데이터가 있는지 없는지 확인 한 후 패킷을 마저 얻을지, 그냥 중단할지를 결정해야 한다.
	// Get 으로 얻는 경우는 검사 후에 사이즈가 안맞다면 헤더를 다시 큐에 넣어야 하는데… 불가능.
	//-----------------------------------------------------------------
	pSession->RecvQ.Peek((char*)&stPacketHeader, sizeof(st_PACKET_HEADER));
	if (dfPACKET_CODE != stPacketHeader.byCode)
		return -1;

	if (stPacketHeader.bySize + sizeof(st_PACKET_HEADER) > iRecvQSize)
		return 1;

	//-----------------------------------------------------------------
	// 위에서 헤더 부분은 Get 이 아닌 Peek 으로 빼왔으니까.. 큐에서 지워주자.
	//-----------------------------------------------------------------
	pSession->RecvQ.MoveFront(sizeof(st_PACKET_HEADER));

	CSerializeBuffer Packet;
	//-----------------------------------------------------------------
	// Payload 부분 버퍼로 빼옴.
	//-----------------------------------------------------------------
	if (!pSession->RecvQ.Dequeue(Packet.GetBufferPtr(), stPacketHeader.bySize))
		return -1;

	//-----------------------------------------------------------------
	// 패킷 클래스에 버퍼 포인터를 얻어 임의로 데이터를 넣었으므로
	// 클래스 내부의 사이즈 이동처리를 다시 해주자.
	//-----------------------------------------------------------------
	Packet.MoveWritePos(stPacketHeader.bySize);

	// 실질적인 패킷 처리 함수를 호출한다.
	if (!PacketProc(pSession, stPacketHeader.byType, &Packet))
		return -1;

	return 0;

}

bool PacketProc(st_SESSION* pSession, BYTE byPacketType, CSerializeBuffer* pPacket)
{
	switch (byPacketType)
	{
	case dfPACKET_CS_MOVE_START:
		return netPacketProc_MoveStart(pSession, pPacket);
		break;
	case dfPACKET_CS_MOVE_STOP:
		return netPacketProc_MoveStop(pSession, pPacket);
		break;
	case dfPACKET_CS_ATTACK1:
		return netPacketProc_Attack1(pSession, pPacket);
		break;
	case dfPACKET_CS_ATTACK2:
		return netPacketProc_Attack2(pSession, pPacket);
		break;
	case dfPACKET_CS_ATTACK3:
		return netPacketProc_Attack3(pSession, pPacket);
		break;
	case dfPACKET_CS_ECHO:
		return netPacketProc_Echo(pSession, pPacket);
		break;
	}
	return true;
}

st_SESSION* CreateSession(SOCKET Socket)
{
	st_SESSION* retSession = new st_SESSION;
	retSession->dwLastRecvTime = GetTickCount();
	retSession->dwSessionID = g_SessionCnt++;
	retSession->Socket = Socket;
	g_SessionMap[Socket] = retSession;

	return retSession;
}

st_SESSION* FindSession(SOCKET Socket)
{
	return g_SessionMap[Socket];
}

void DisconnectSession(SOCKET Socket)
{
	st_SESSION* pSession = g_SessionMap[Socket];
	_LOG(dfLOG_LEVEL_DEBUG, L"Disconnect Session ID : %d \n", pSession->dwSessionID);

	DisconnectCharacter(pSession->dwSessionID);
	closesocket(pSession->Socket);
	delete pSession;

	auto eraseIter = g_SessionMap.find(Socket);
	g_SessionMap.erase(eraseIter);
}

void DisconnectSession(st_SESSION* pSession)
{
	_LOG(dfLOG_LEVEL_DEBUG, L"Disconnect Session ID : %d \n", pSession->dwSessionID);

	DisconnectCharacter(pSession->dwSessionID);
	closesocket(pSession->Socket);
	delete pSession;

	auto eraseIter = g_SessionMap.find(pSession->Socket);
	g_SessionMap.erase(eraseIter);
}

void SendPacket_SectorOne(int iSecotrX, int iSectorY, CSerializeBuffer* pPacket, st_SESSION* pSession)
{
	for (auto sectorIter = g_Sector[iSectorY][iSecotrX].begin();
		sectorIter != g_Sector[iSectorY][iSecotrX].end();
		++sectorIter)
	{
		st_CHARACTER* pOtherCharacter = (*sectorIter);
		if (pSession == pOtherCharacter->pSession)
			continue;
		
		SendPacket_Unicast(pOtherCharacter->pSession, pPacket);
	}
}

void SendPacket_Unicast(st_SESSION* pSession, CSerializeBuffer* pPacket)
{
	if (pSession == NULL)
	{
		_LOG(dfLOG_LEVEL_ERROR, L"SendUnicast Session is NULL \n");
		return;
	}


	BYTE* checkCode = (BYTE*)pPacket->GetBufferPtr();
	if (*checkCode != 0x89)
	{
		_LOG(dfLOG_LEVEL_ERROR, L"checkCode %d \n", *checkCode);
	}

	int iPacketDataSize = pPacket->GetDataSize();
	int iRet = pSession->SendQ.Enqueue((char*)pPacket->GetBufferPtr(), pPacket->GetDataSize());

	if (iPacketDataSize != iRet)
	{
		_LOG(dfLOG_LEVEL_ERROR, L"Not Enough SendQ Error\n");
		DisconnectSession(pSession);
	}
}

void SendPacket_Around(st_SESSION* pSession, CSerializeBuffer* pPacket, bool bSendMe)
{
	st_CHARACTER* pCharacter = FindCharacter(pSession->dwSessionID);
	if (pCharacter == nullptr)
		return;

	st_SECTOR_AROUND sectorAround;
	GetSectorAround(pCharacter->CurSector.iX, pCharacter->CurSector.iY, &sectorAround);

	for (int i = 0; i < sectorAround.iCount; i++)
	{
		SendPacket_SectorOne(sectorAround.Around[i].iX, sectorAround.Around[i].iY, pPacket, pSession);
	}
}

void SendPacket_Broadcast(CSerializeBuffer* pPacket)
{
	st_SESSION* pSession;
	map<SOCKET, st_SESSION*>::iterator SessionIter;
	for (SessionIter = g_SessionMap.begin(); SessionIter != g_SessionMap.end(); SessionIter++)
	{
		pSession = SessionIter->second;
		SendPacket_Unicast(pSession, pPacket);
	}
}

bool netPacketProc_MoveStart(st_SESSION* pSession, CSerializeBuffer* pPacket)
{
	BYTE byDirection;
	short shX, shY;
	*pPacket >> byDirection;
	*pPacket >> shX;
	*pPacket >> shY;
	_LOG(dfLOG_LEVEL_DEBUG, L"# MOVESTART # SessionID:%d / Direction:%d / X:%d / Y:%d \n",
		pSession->dwSessionID, byDirection, shX, shY);

	//-----------------------------------------------------
	// ID 로 캐릭터를 검색한다.
	//-----------------------------------------------------
	st_CHARACTER* pCharacter = FindCharacter(pSession->dwSessionID);
	if (pCharacter == NULL)
	{
		_LOG(dfLOG_LEVEL_ERROR, L"# MOVESTART > SessionID:%d Character Not Found! \n",
			pSession->dwSessionID);
		return false;
	}
	//-----------------------------------------------------
	// 서버의 위치와 받은 패킷의 위치값이 너무 큰 차이가 난다면 싱크 패킷을 보내어 좌표 보정.
	//
	// 본 게임의 좌표 동기화 구조가 단순한 키보드 조작 (클라이언트의 선처리, 서버의 후 반영) 방식으로
	// 클라이언트의 좌표를 그대로 믿는 방식을 택하고 있음.
	// 실제 온라인 게임이라면 클라이언트에서 목적지를 공유하는 방식을 택해야 함.
	// 지금은 좌표에 대해서는 간단한 구현을 목적으로 하고 있으므로
	// 서버는 클라이언트의 좌표를 그대로 믿지만, 서버와 너무 큰 차이가 있다면 강제좌표 동기화 하도록 함
	//-----------------------------------------------------
	if (abs(pCharacter->shX - shX) > dfERROR_RANGE || abs(pCharacter->shY - shY) > dfERROR_RANGE)
	{
		_LOG(dfLOG_LEVEL_DEBUG, L"# MOVESTART Sync # Packet X Y : %d %d, Sync X Y %d %d \n", 
			shX, shY, pCharacter->shX, pCharacter->shY);
		mpSync(pPacket, pCharacter->dwSessionID, pCharacter->shX, pCharacter->shY);
		SendPacket_Around(pCharacter->pSession, pPacket, true);
		shX = pCharacter->shX;
		shY = pCharacter->shY;
	}
	//-----------------------------------------------------
	// 동작을 변경. 동작번호와, 방향값이 같다.
	//-----------------------------------------------------
	pCharacter->dwAction = byDirection;
	//-----------------------------------------------------
	// 단순 방향표시용 byDirection (LL,RR) 과
	// 이동시 8방향 (LL,LU,UU,RU,RR,RD,DD,LD) 용 MoveDirection 이 있음.
	//-----------------------------------------------------
	pCharacter->byMoveDirection = byDirection;

	//-----------------------------------------------------
	// 방향을 변경.
	//-----------------------------------------------------
	switch (byDirection)
	{
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
		pCharacter->byDirection = dfPACKET_MOVE_DIR_RR;
		break;
	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LD:
		pCharacter->byDirection = dfPACKET_MOVE_DIR_LL;
		break;
	}
	pCharacter->shX = shX;
	pCharacter->shY = shY;
	//-----------------------------------------------------------------------
	// 정지를 하면서 좌표가 약간 조절된 경우 섹터 업데이트를 함. (섹터는 차후 설명)
	//-----------------------------------------------------------------------
	if (Sector_UpdateCharacter(pCharacter))
	{
		//-----------------------------------------------------------------------
		// 섹터가 변경된 경우는 클라에게 관련 정보를 쏜다. (섹터는 차후 설명)
		//-----------------------------------------------------------------------
		CharacterSectorUpdatePacket(pCharacter);
	}
	
	mpMoveStart(pPacket, pSession->dwSessionID, byDirection, pCharacter->shX, pCharacter->shY);

	//-----------------------------------------------------
	// 현재 접속중인 사용자에게 모든 패킷을 뿌린다. (섹터 단위 패킷 전송 함수 )
	//-----------------------------------------------------
	SendPacket_Around(pSession, pPacket);

	return true;
}

bool netPacketProc_MoveStop(st_SESSION* pSession, CSerializeBuffer* pPacket)
{
	BYTE byDirection;
	short shX, shY;
	*pPacket >> byDirection;
	*pPacket >> shX;
	*pPacket >> shY;
	_LOG(dfLOG_LEVEL_DEBUG, L"# MOVESTOP # SessionID:%d / Direction:%d / X:%d / Y:%d \n",
		pSession->dwSessionID, byDirection, shX, shY);

	//-----------------------------------------------------
	// ID 로 캐릭터를 검색한다.
	//-----------------------------------------------------
	st_CHARACTER* pCharacter = FindCharacter(pSession->dwSessionID);
	if (pCharacter == NULL)
	{
		_LOG(dfLOG_LEVEL_ERROR, L"# MOVESTOP > SessionID:%d Character Not Found! \n",
			pSession->dwSessionID);
		return false;
	}
	//-----------------------------------------------------
	// 서버의 위치와 받은 패킷의 위치값이 너무 큰 차이가 난다면 싱크 패킷을 보내어 좌표 보정.
	//
	// 본 게임의 좌표 동기화 구조가 단순한 키보드 조작 (클라이언트의 선처리, 서버의 후 반영) 방식으로
	// 클라이언트의 좌표를 그대로 믿는 방식을 택하고 있음.
	// 실제 온라인 게임이라면 클라이언트에서 목적지를 공유하는 방식을 택해야 함.
	// 지금은 좌표에 대해서는 간단한 구현을 목적으로 하고 있으므로
	// 서버는 클라이언트의 좌표를 그대로 믿지만, 서버와 너무 큰 차이가 있다면 강제좌표 동기화 하도록 함
	//-----------------------------------------------------
	if (abs(pCharacter->shX - shX) > dfERROR_RANGE || abs(pCharacter->shY - shY) > dfERROR_RANGE)
	{
		_LOG(dfLOG_LEVEL_DEBUG, L"# MOVESTOP Sync # Packet X Y : %d %d, Sync X Y %d %d \n",
			shX, shY, pCharacter->shX, pCharacter->shY);
		mpSync(pPacket, pCharacter->dwSessionID, pCharacter->shX, pCharacter->shY);
		SendPacket_Around(pCharacter->pSession, pPacket, true);
		shX = pCharacter->shX;
		shY = pCharacter->shY;
	}

	//-----------------------------------------------------
	// 동작을 변경. 동작번호와, 방향값이 같다.
	//-----------------------------------------------------
	pCharacter->dwAction = dfACTION_STAND;
	pCharacter->byMoveDirection = dfACTION_STAND;
	
	//-----------------------------------------------------
	// 방향을 변경.
	//-----------------------------------------------------
	switch (byDirection)
	{
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
		pCharacter->byDirection = dfPACKET_MOVE_DIR_RR;
		break;
	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LD:
		pCharacter->byDirection = dfPACKET_MOVE_DIR_LL;
		break;
	}

	pCharacter->shX = shX;
	pCharacter->shY = shY;

	//-----------------------------------------------------------------------
	// 정지를 하면서 좌표가 약간 조절된 경우 섹터 업데이트를 함. (섹터는 차후 설명)
	//-----------------------------------------------------------------------
	if (Sector_UpdateCharacter(pCharacter))
	{
		//-----------------------------------------------------------------------
		// 섹터가 변경된 경우는 클라에게 관련 정보를 쏜다. (섹터는 차후 설명)
		//-----------------------------------------------------------------------
		CharacterSectorUpdatePacket(pCharacter);
	}

	mpMoveStop(pPacket, pSession->dwSessionID, byDirection, pCharacter->shX, pCharacter->shY);

	//-----------------------------------------------------
	// 현재 접속중인 사용자에게 모든 패킷을 뿌린다. (섹터 단위 패킷 전송 함수 )
	//-----------------------------------------------------
	SendPacket_Around(pSession, pPacket);

	return true;
}

bool netPacketProc_Attack1(st_SESSION* pSession, CSerializeBuffer* pPacket)
{
	BYTE byDirection;
	short shX, shY;
	*pPacket >> byDirection;
	*pPacket >> shX;
	*pPacket >> shY;
	_LOG(dfLOG_LEVEL_DEBUG, L"# ATTACK1 # SessionID:%d / Direction:%d / X:%d / Y:%d \n",
		pSession->dwSessionID, byDirection, shX, shY);

	//-----------------------------------------------------
	// ID 로 캐릭터를 검색한다.
	//-----------------------------------------------------
	st_CHARACTER* pCharacter = FindCharacter(pSession->dwSessionID);
	if (pCharacter == NULL)
	{
		_LOG(dfLOG_LEVEL_ERROR, L"# ATTACK1 > SessionID:%d Character Not Found! \n",
			pSession->dwSessionID);
		return false;
	}
	//-----------------------------------------------------
	// 서버의 위치와 받은 패킷의 위치값이 너무 큰 차이가 난다면 싱크 패킷을 보내어 좌표 보정.
	//
	// 본 게임의 좌표 동기화 구조가 단순한 키보드 조작 (클라이언트의 선처리, 서버의 후 반영) 방식으로
	// 클라이언트의 좌표를 그대로 믿는 방식을 택하고 있음.
	// 실제 온라인 게임이라면 클라이언트에서 목적지를 공유하는 방식을 택해야 함.
	// 지금은 좌표에 대해서는 간단한 구현을 목적으로 하고 있으므로
	// 서버는 클라이언트의 좌표를 그대로 믿지만, 서버와 너무 큰 차이가 있다면 강제좌표 동기화 하도록 함
	//-----------------------------------------------------
	if (abs(pCharacter->shX - shX) > dfERROR_RANGE || abs(pCharacter->shY - shY) > dfERROR_RANGE)
	{
		mpSync(pPacket, pCharacter->dwSessionID, pCharacter->shX, pCharacter->shY);
		SendPacket_Around(pCharacter->pSession, pPacket, true);
		shX = pCharacter->shX;
		shY = pCharacter->shY;
	}
	
	//-----------------------------------------------------
	// 동작을 변경. 동작번호와, 방향값이 같다.
	//-----------------------------------------------------
	pCharacter->dwAction = dfACTION_ATTACK1;
	pCharacter->byMoveDirection = dfACTION_ATTACK1;
	
	//-----------------------------------------------------
	// 방향을 변경.
	//-----------------------------------------------------
	switch (byDirection)
	{
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
		pCharacter->byDirection = dfPACKET_MOVE_DIR_RR;
		break;
	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LD:
		pCharacter->byDirection = dfPACKET_MOVE_DIR_LL;
		break;
	}

	pCharacter->shX = shX;
	pCharacter->shY = shY;

	//-----------------------------------------------------------------------
	// 정지를 하면서 좌표가 약간 조절된 경우 섹터 업데이트를 함. (섹터는 차후 설명)
	//-----------------------------------------------------------------------
	if (Sector_UpdateCharacter(pCharacter))
	{
		//-----------------------------------------------------------------------
		// 섹터가 변경된 경우는 클라에게 관련 정보를 쏜다. (섹터는 차후 설명)
		//-----------------------------------------------------------------------
		CharacterSectorUpdatePacket(pCharacter);
	}

	mpAttack1(pPacket, pSession->dwSessionID, byDirection, pCharacter->shX, pCharacter->shY);

	//-----------------------------------------------------
	// 현재 접속중인 사용자에게 모든 패킷을 뿌린다. (섹터 단위 패킷 전송 함수 )
	//-----------------------------------------------------
	SendPacket_Around(pSession, pPacket);

	//-----------------------------------------------------
	// 충돌 체크
	//-----------------------------------------------------
	RECT colRc;
	GetCollisionRect(&colRc, pCharacter);

	st_SECTOR_AROUND collisionSector;
	GetCollisionSectorAround(&colRc, &collisionSector);

	for (int i = 0; i < collisionSector.iCount; i++)
	{
		int idxX = collisionSector.Around[i].iX;
		int idxY = collisionSector.Around[i].iY;

		st_SECTOR_AROUND innerSector;
		GetSectorAround(idxX, idxY, &innerSector);

		for (auto iter = g_Sector[idxY][idxX].begin(); iter != g_Sector[idxY][idxX].end(); ++iter)
		{
			if (*iter == pCharacter)
				continue;

			st_CHARACTER* pOtherCharacter = (*iter);
			POINT pt = { pOtherCharacter->shX, pOtherCharacter->shY };
			if (PtInRect(&colRc, pt) == true)
			{
				pOtherCharacter->chHP -= dfDAMAGE_ATTACK1;
				if (pOtherCharacter->chHP < 0)
					pOtherCharacter->chHP = 0;

				mpDamage(pPacket, pCharacter->dwSessionID, pOtherCharacter->dwSessionID, pOtherCharacter->chHP);

				for (int j = 0; j < innerSector.iCount; j++)
				{
					int innerIdxX = innerSector.Around[j].iX;
					int innerIdxY = innerSector.Around[j].iY;
					SendPacket_SectorOne(innerIdxX, innerIdxY, pPacket, NULL);
				}
			}
		}
	}

	return true;
}

bool netPacketProc_Attack2(st_SESSION* pSession, CSerializeBuffer* pPacket)
{
	BYTE byDirection;
	short shX, shY;
	*pPacket >> byDirection;
	*pPacket >> shX;
	*pPacket >> shY;
	_LOG(dfLOG_LEVEL_DEBUG, L"# ATTACK2 # SessionID:%d / Direction:%d / X:%d / Y:%d \n",
		pSession->dwSessionID, byDirection, shX, shY);

	//-----------------------------------------------------
	// ID 로 캐릭터를 검색한다.
	//-----------------------------------------------------
	st_CHARACTER* pCharacter = FindCharacter(pSession->dwSessionID);
	if (pCharacter == NULL)
	{
		_LOG(dfLOG_LEVEL_ERROR, L"# ATTACK2 > SessionID:%d Character Not Found! \n",
			pSession->dwSessionID);
		return false;
	}
	//-----------------------------------------------------
	// 서버의 위치와 받은 패킷의 위치값이 너무 큰 차이가 난다면 싱크 패킷을 보내어 좌표 보정.
	//
	// 본 게임의 좌표 동기화 구조가 단순한 키보드 조작 (클라이언트의 선처리, 서버의 후 반영) 방식으로
	// 클라이언트의 좌표를 그대로 믿는 방식을 택하고 있음.
	// 실제 온라인 게임이라면 클라이언트에서 목적지를 공유하는 방식을 택해야 함.
	// 지금은 좌표에 대해서는 간단한 구현을 목적으로 하고 있으므로
	// 서버는 클라이언트의 좌표를 그대로 믿지만, 서버와 너무 큰 차이가 있다면 강제좌표 동기화 하도록 함
	//-----------------------------------------------------
	if (abs(pCharacter->shX - shX) > dfERROR_RANGE || abs(pCharacter->shY - shY) > dfERROR_RANGE)
	{
		mpSync(pPacket, pCharacter->dwSessionID, pCharacter->shX, pCharacter->shY);
		SendPacket_Around(pCharacter->pSession, pPacket, true);
		shX = pCharacter->shX;
		shY = pCharacter->shY;
	}
	
	//-----------------------------------------------------
	// 동작을 변경. 동작번호와, 방향값이 같다.
	//-----------------------------------------------------
	pCharacter->dwAction = dfACTION_ATTACK2;
	pCharacter->byMoveDirection = dfACTION_ATTACK2;

	//-----------------------------------------------------
	// 방향을 변경.
	//-----------------------------------------------------
	switch (byDirection)
	{
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
		pCharacter->byDirection = dfPACKET_MOVE_DIR_RR;
		break;
	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LD:
		pCharacter->byDirection = dfPACKET_MOVE_DIR_LL;
		break;
	}

	pCharacter->shX = shX;
	pCharacter->shY = shY;

	//-----------------------------------------------------------------------
	// 정지를 하면서 좌표가 약간 조절된 경우 섹터 업데이트를 함. (섹터는 차후 설명)
	//-----------------------------------------------------------------------
	if (Sector_UpdateCharacter(pCharacter))
	{
		//-----------------------------------------------------------------------
		// 섹터가 변경된 경우는 클라에게 관련 정보를 쏜다. (섹터는 차후 설명)
		//-----------------------------------------------------------------------
		CharacterSectorUpdatePacket(pCharacter);
	}

	mpAttack2(pPacket, pSession->dwSessionID, byDirection, pCharacter->shX, pCharacter->shY);

	//-----------------------------------------------------
	// 현재 접속중인 사용자에게 모든 패킷을 뿌린다. (섹터 단위 패킷 전송 함수 )
	//-----------------------------------------------------
	SendPacket_Around(pSession, pPacket);

	//-----------------------------------------------------
	// 충돌 체크
	//-----------------------------------------------------
	RECT colRc;
	GetCollisionRect(&colRc, pCharacter);

	st_SECTOR_AROUND collisionSector;
	GetCollisionSectorAround(&colRc, &collisionSector);

	for (int i = 0; i < collisionSector.iCount; i++)
	{
		int idxX = collisionSector.Around[i].iX;
		int idxY = collisionSector.Around[i].iY;

		st_SECTOR_AROUND innerSector;
		GetSectorAround(idxX, idxY, &innerSector);

		for (auto iter = g_Sector[idxY][idxX].begin(); iter != g_Sector[idxY][idxX].end(); ++iter)
		{
			if (*iter == pCharacter)
				continue;

			st_CHARACTER* pOtherCharacter = (*iter);
			POINT pt = { pOtherCharacter->shX, pOtherCharacter->shY };
			if (PtInRect(&colRc, pt) == true)
			{
				pOtherCharacter->chHP -= dfDAMAGE_ATTACK2;
				if (pOtherCharacter->chHP < 0)
					pOtherCharacter->chHP = 0;

				mpDamage(pPacket, pCharacter->dwSessionID, pOtherCharacter->dwSessionID, pOtherCharacter->chHP);

				for (int j = 0; j < innerSector.iCount; j++)
				{
					int innerIdxX = innerSector.Around[j].iX;
					int innerIdxY = innerSector.Around[j].iY;
					SendPacket_SectorOne(innerIdxX, innerIdxY, pPacket, NULL);
				}
			}
		}
	}

	return true;
}

bool netPacketProc_Attack3(st_SESSION* pSession, CSerializeBuffer* pPacket)
{
	BYTE byDirection;
	short shX, shY;
	*pPacket >> byDirection;
	*pPacket >> shX;
	*pPacket >> shY;
	_LOG(dfLOG_LEVEL_DEBUG, L"# ATTACK3 # SessionID:%d / Direction:%d / X:%d / Y:%d \n",
		pSession->dwSessionID, byDirection, shX, shY);

	//-----------------------------------------------------
	// ID 로 캐릭터를 검색한다.
	//-----------------------------------------------------
	st_CHARACTER* pCharacter = FindCharacter(pSession->dwSessionID);
	if (pCharacter == NULL)
	{
		_LOG(dfLOG_LEVEL_ERROR, L"# ATTACK3 > SessionID:%d Character Not Found! \n",
			pSession->dwSessionID);
		return false;
	}
	//-----------------------------------------------------
	// 서버의 위치와 받은 패킷의 위치값이 너무 큰 차이가 난다면 싱크 패킷을 보내어 좌표 보정.
	//
	// 본 게임의 좌표 동기화 구조가 단순한 키보드 조작 (클라이언트의 선처리, 서버의 후 반영) 방식으로
	// 클라이언트의 좌표를 그대로 믿는 방식을 택하고 있음.
	// 실제 온라인 게임이라면 클라이언트에서 목적지를 공유하는 방식을 택해야 함.
	// 지금은 좌표에 대해서는 간단한 구현을 목적으로 하고 있으므로
	// 서버는 클라이언트의 좌표를 그대로 믿지만, 서버와 너무 큰 차이가 있다면 강제좌표 동기화 하도록 함
	//-----------------------------------------------------
	if (abs(pCharacter->shX - shX) > dfERROR_RANGE || abs(pCharacter->shY - shY) > dfERROR_RANGE)
	{
		mpSync(pPacket, pCharacter->dwSessionID, pCharacter->shX, pCharacter->shY);
		SendPacket_Around(pCharacter->pSession, pPacket, true);
		shX = pCharacter->shX;
		shY = pCharacter->shY;
	}
	
	//-----------------------------------------------------
	// 동작을 변경. 동작번호와, 방향값이 같다.
	//-----------------------------------------------------
	pCharacter->dwAction = dfACTION_ATTACK3;
	pCharacter->byMoveDirection = dfACTION_ATTACK3;

	//-----------------------------------------------------
	// 방향을 변경.
	//-----------------------------------------------------
	switch (byDirection)
	{
	case dfPACKET_MOVE_DIR_RR:
	case dfPACKET_MOVE_DIR_RU:
	case dfPACKET_MOVE_DIR_RD:
		pCharacter->byDirection = dfPACKET_MOVE_DIR_RR;
		break;
	case dfPACKET_MOVE_DIR_LU:
	case dfPACKET_MOVE_DIR_LL:
	case dfPACKET_MOVE_DIR_LD:
		pCharacter->byDirection = dfPACKET_MOVE_DIR_LL;
		break;
	}

	pCharacter->shX = shX;
	pCharacter->shY = shY;

	//-----------------------------------------------------------------------
	// 정지를 하면서 좌표가 약간 조절된 경우 섹터 업데이트를 함. (섹터는 차후 설명)
	//-----------------------------------------------------------------------
	if (Sector_UpdateCharacter(pCharacter))
	{
		//-----------------------------------------------------------------------
		// 섹터가 변경된 경우는 클라에게 관련 정보를 쏜다. (섹터는 차후 설명)
		//-----------------------------------------------------------------------
		CharacterSectorUpdatePacket(pCharacter);
	}

	mpAttack3(pPacket, pSession->dwSessionID, byDirection, pCharacter->shX, pCharacter->shY);

	//-----------------------------------------------------
	// 현재 접속중인 사용자에게 모든 패킷을 뿌린다. (섹터 단위 패킷 전송 함수 )
	//-----------------------------------------------------
	SendPacket_Around(pSession, pPacket);

	//-----------------------------------------------------
	// 충돌 체크
	//-----------------------------------------------------
	RECT colRc;
	GetCollisionRect(&colRc, pCharacter);

	st_SECTOR_AROUND collisionSector;
	GetCollisionSectorAround(&colRc, &collisionSector);

	for (int i = 0; i < collisionSector.iCount; i++)
	{
		int idxX = collisionSector.Around[i].iX;
		int idxY = collisionSector.Around[i].iY;

		st_SECTOR_AROUND innerSector;
		GetSectorAround(idxX, idxY, &innerSector);

		for (auto iter = g_Sector[idxY][idxX].begin(); iter != g_Sector[idxY][idxX].end(); ++iter)
		{
			if (*iter == pCharacter)
				continue;

			st_CHARACTER* pOtherCharacter = (*iter);
			POINT pt = { pOtherCharacter->shX, pOtherCharacter->shY };
			if (PtInRect(&colRc, pt) == true)
			{
				pOtherCharacter->chHP -= dfDAMAGE_ATTACK3;
				if (pOtherCharacter->chHP < 0)
					pOtherCharacter->chHP = 0;

				mpDamage(pPacket, pCharacter->dwSessionID, pOtherCharacter->dwSessionID, pOtherCharacter->chHP);

				for (int j = 0; j < innerSector.iCount; j++)
				{
					int innerIdxX = innerSector.Around[j].iX;
					int innerIdxY = innerSector.Around[j].iY;
					SendPacket_SectorOne(innerIdxX, innerIdxY, pPacket, NULL);
				}
			}
		}
	}

	return true;
}

bool netPacketProc_Echo(st_SESSION* pSession, CSerializeBuffer* pPacket)
{
	DWORD dwTime;
	*pPacket >> dwTime;

	mpEcho(pPacket, dwTime);

	SendPacket_Unicast(pSession, pPacket);

	return true;
}

bool LoadData()
{
	srand((unsigned int)time(0));

	SINGLETON(CFPSTimer)->Init(dfNETWORK_FPS);

	return true;
}

void SaveData()
{
	SINGLETON(CFPSTimer)->Release();
}

void Update(void)
{
	/// 적절한 게임 업데이트 처리 타이밍 계산 
	// 25fps
	if(!SINGLETON(CFPSTimer)->CheckFrame())
		return;

	// 1초 단위로 출력하기 위함
	if(SINGLETON(CFPSTimer)->CheckSec())
	{
		__int64 logicFPS = SINGLETON(CFPSTimer)->GetCurrentLogicFPS();
		if (logicFPS != dfNETWORK_FPS)
		{
			__int64 loopFPS = SINGLETON(CFPSTimer)->GetCurrentLoopFPS();
			_LOG(dfLOG_LEVEL_SYSTEM, L"Server FPS : %d, LoopCnt : %d \n", logicFPS, loopFPS);
		}
		else
		{
			__int64 loopFPS = SINGLETON(CFPSTimer)->GetCurrentLoopFPS();
			_LOG(dfLOG_LEVEL_SYSTEM, L"Server FPS : %d, LoopCnt : %d \n", logicFPS, loopFPS);
		}
	}

	DWORD dwCurrentTick = GetTickCount();

	// 게임 업데이트 처리를 매번 해준다면 쓸 때 없는 부하가 일어남.
	// 가장 좋은 것은 클라이언트의 처리 주기와 똑같이 맞추는 것이지만
	// 서버의 부담이 너무 크기도 하고, 요즘 3D 게임들은 클라이언트의 프레임 고정이 없기 때문에
	// 적절하게 클라이언트와 같은 처리 결과가 나오도록 돌아가도록 만 해주면 됨

	st_CHARACTER* pCharacter = NULL;
	//-----------------------------------------------------
	// 캐릭터 정보를 찾아서 처리한다.
	//-----------------------------------------------------
	map<DWORD, st_CHARACTER*>::iterator Iter;
	for (Iter = g_CharacterMap.begin(); Iter != g_CharacterMap.end(); )
	{
		pCharacter = Iter->second;
		Iter++;

		if (0 >= pCharacter->chHP)
		{
			// 사망처리.
			DisconnectSession(pCharacter->pSession->Socket);
			_LOG(dfLOG_LEVEL_ERROR, L"chHP: 0\n");
		}
		else
		{
			// 일정 시간동안 수신이 없으면 종료처리
			if (dwCurrentTick - pCharacter->pSession->dwLastRecvTime >
				dfNETWORK_PACKET_RECV_TIMEOUT)
			{
				DisconnectSession(pCharacter->pSession->Socket);
				st_SESSION* pSession = pCharacter->pSession;
				_LOG(dfLOG_LEVEL_ERROR, L"ID: %d, RECVQ : %d, SENDQ : %d, RECV_CNT : %d, SEND_CNT : %d, RECVSET : %d, TIMEOUT : %d \n", 
					pSession->dwSessionID, pSession->RecvQ.GetUseSize(), pSession->SendQ.GetUseSize(), 
					pSession->dwRecvCount, pSession->dwSendCount, pSession->bRecvSet,
					pSession->dwLastRecvTime - dwCurrentTick);
				continue;
			}

			//-----------------------------------------------------------
			// 현재 동작에 따른 처리.
			//-----------------------------------------------------------
			switch (pCharacter->dwAction)
			{
			case dfACTION_MOVE_LL:
				if (CharacterMoveCheck(pCharacter->shX - dfSPEED_PLAYER_X,
					pCharacter->shY))
				{
					pCharacter->shX -= dfSPEED_PLAYER_X;
				}
				break;
			case dfACTION_MOVE_LU:
				if (CharacterMoveCheck(pCharacter->shX - dfSPEED_PLAYER_X,
					pCharacter->shY - dfSPEED_PLAYER_Y))
				{
					pCharacter->shX -= dfSPEED_PLAYER_X;
					pCharacter->shY -= dfSPEED_PLAYER_Y;
				}
				break;
			case dfACTION_MOVE_UU:
				if (CharacterMoveCheck(pCharacter->shX,
					pCharacter->shY - dfSPEED_PLAYER_Y))
					pCharacter->shY -= dfSPEED_PLAYER_Y;
				break;
			case dfACTION_MOVE_RU:
				if (CharacterMoveCheck(pCharacter->shX + dfSPEED_PLAYER_X,
					pCharacter->shY - dfSPEED_PLAYER_Y))
				{
					pCharacter->shX += dfSPEED_PLAYER_X;
					pCharacter->shY -= dfSPEED_PLAYER_Y;
				}
				break;
			case dfACTION_MOVE_RR:
				if (CharacterMoveCheck(pCharacter->shX + dfSPEED_PLAYER_X, pCharacter->shY))
				{
					pCharacter->shX += dfSPEED_PLAYER_X;
				}
				break;
			case dfACTION_MOVE_RD:
				if (CharacterMoveCheck(pCharacter->shX + dfSPEED_PLAYER_X,
					pCharacter->shY + dfSPEED_PLAYER_Y))
				{
					pCharacter->shX += dfSPEED_PLAYER_X;
					pCharacter->shY += dfSPEED_PLAYER_Y;
				}
				break;
			case dfACTION_MOVE_DD:
				if (CharacterMoveCheck(pCharacter->shX, pCharacter->shY + dfSPEED_PLAYER_Y))
				{
					pCharacter->shY += dfSPEED_PLAYER_Y;
				}
				break;
			case dfACTION_MOVE_LD:
				if (CharacterMoveCheck(pCharacter->shX - dfSPEED_PLAYER_X,
					pCharacter->shY + dfSPEED_PLAYER_Y))
				{
					pCharacter->shX -= dfSPEED_PLAYER_X;
					pCharacter->shY += dfSPEED_PLAYER_Y;
				}
				break;
			}

			if (dfACTION_MOVE_LL <= pCharacter->dwAction && pCharacter->dwAction <= dfACTION_MOVE_LD)
			{
				// 이동 인 경우 섹터 업데이트를 함.
				if (Sector_UpdateCharacter(pCharacter))
				{
					// 섹터가 변경된 경우는 클라에게 관련 정보를 쏜다.
					CharacterSectorUpdatePacket(pCharacter);
				}
			}
		}
	}
}

void ServerControl()
{
	// 키보드 컨트롤 잠금, 풀림 변수
	static bool bControlMode = false;

	// L : 컨트롤 Lock / U : 컨트롤 Unlock / Q : 서버 종료

	if ( _kbhit() )
	{
		WCHAR ControlKey = _getwch();

		// 키보드 제어 허용
		if (L'u' == ControlKey || L'U' == ControlKey)
		{
			bControlMode = true;

			// 관련 키 도움말 출력
			wprintf(L"Control Mode : Press Q - Quit \n");
			wprintf(L"Control Mode : Press L - Key Lock \n");
		}

		// 키보드 제어 잠금
		if ((L'l' == ControlKey || L'L' == ControlKey) && bControlMode)
		{
			wprintf(L"Control Lock..! Press U - Control Unlock\n");
			bControlMode = false;
		}

		// 키보드 제어 풀림 상태에서 특정 기능
		if ((L'q' == ControlKey || L'Q' == ControlKey) && bControlMode)
		{
			g_bShutdown = true;
		}
	}
}

// 새로운 세션을 생성,등록 한다.
st_CHARACTER* CreateCharacter(st_SESSION* pSession)
{
	if (pSession == nullptr)
		return nullptr;

	st_CHARACTER* pCharacter = new st_CHARACTER;
	pCharacter->pSession = pSession;
	pCharacter->dwSessionID = pSession->dwSessionID;

	// 캐릭터 생성에 대한 정보를 보내준다.
	pCharacter->byDirection = dfACTION_MOVE_LL;
	pCharacter->byMoveDirection = dfACTION_STAND;
	pCharacter->dwAction = dfACTION_STAND;
	pCharacter->shX = rand() % (dfRANGE_MOVE_RIGHT - dfRANGE_MOVE_LEFT);
	pCharacter->shY = rand() % (dfRANGE_MOVE_BOTTOM - dfRANGE_MOVE_TOP);
	pCharacter->chHP = 100;
	pCharacter->CurSector.iX = pCharacter->OldSector.iX = pCharacter->shX / dfSECTOR_WIDTH;
	pCharacter->CurSector.iY = pCharacter->OldSector.iY = pCharacter->shY / dfSECTOR_HEIGHT;

	g_CharacterMap[pSession->dwSessionID] = pCharacter;

	return pCharacter;
}

// 해당 세션 종료처리.
void DisconnectCharacter(DWORD dwSessionID)
{
	st_CHARACTER* pCharacter = FindCharacter(dwSessionID);
	if (pCharacter == nullptr)
		return;

	// 주변 섹터에 지우기
	CSerializeBuffer Packet;
	mpDeleteCharacter(&Packet, dwSessionID);
	SendPacket_Around(pCharacter->pSession, &Packet);

	// Sector에서 지우기
	g_Sector[pCharacter->CurSector.iY][pCharacter->CurSector.iX].remove(pCharacter);

	auto eraseIter = g_CharacterMap.find(dwSessionID);
	g_CharacterMap.erase(eraseIter);
}

st_CHARACTER* FindCharacter(DWORD dwSessionID)
{
	return g_CharacterMap[dwSessionID];
}

bool CharacterMoveCheck(short shX, short shY)
{
	if (dfRANGE_MOVE_LEFT <= shX && shX < dfRANGE_MOVE_RIGHT && dfRANGE_MOVE_TOP <= shY && shY < dfRANGE_MOVE_BOTTOM)
		return true;

	return false;
}

void Log(WCHAR* szString, int iLogLevel)
{
	// 파일 저장 +  DB 저장 상황에 따라서 다양한 기능들이 추가되어야 함.
	SYSTEMTIME		st;
	GetLocalTime(&st);
	wprintf(L"%s", szString);
}

void GetCollisionRect(LPRECT outRect, st_CHARACTER* pCharacter)
{
	if (pCharacter->byDirection == dfACTION_MOVE_LL)
	{
		switch (pCharacter->dwAction)
		{
		case dfACTION_ATTACK1:
			outRect->right = pCharacter->shX;
			outRect->left = pCharacter->shX - 70;
			outRect->top = pCharacter->shY - 10;
			outRect->bottom = pCharacter->shY + 10;
			break;
		case dfACTION_ATTACK2:
			outRect->right = pCharacter->shX;
			outRect->left = pCharacter->shX - 70;
			outRect->top = pCharacter->shY - 20;
			outRect->bottom = pCharacter->shY + 20;
			break;
		case dfACTION_ATTACK3:
			outRect->right = pCharacter->shX;
			outRect->left = pCharacter->shX - 70;
			outRect->top = pCharacter->shY - 30;
			outRect->bottom = pCharacter->shY + 30;
			break;
		default:
			outRect->left = outRect->right = outRect->top = outRect->bottom = 0;
			break;
		}
	}
	else if (pCharacter->byDirection == dfACTION_MOVE_RR)
	{
		switch (pCharacter->dwAction)
		{
		case dfACTION_ATTACK1:
			outRect->right = pCharacter->shX + 70;
			outRect->left = pCharacter->shX;
			outRect->top = pCharacter->shY - 10;
			outRect->bottom = pCharacter->shY + 10;
			break;
		case dfACTION_ATTACK2:
			outRect->right = pCharacter->shX + 70;
			outRect->left = pCharacter->shX;
			outRect->top = pCharacter->shY - 20;
			outRect->bottom = pCharacter->shY + 20;
			break;
		case dfACTION_ATTACK3:
			outRect->right = pCharacter->shX + 70;
			outRect->left = pCharacter->shX;
			outRect->top = pCharacter->shY - 30;
			outRect->bottom = pCharacter->shY + 30;
			break;
		default:
			outRect->left = outRect->right = outRect->top = outRect->bottom = 0;
			break;
		}
	}
}

void GetSectorAround(int iSectorX, int iSectorY, st_SECTOR_AROUND* pSectorAround)
{
	int iCntX, iCntY;
	pSectorAround->iCount = 0;
	for (iCntY = -1; iCntY < 2; iCntY++)
	{
		if (iSectorY + iCntY < 0 || iSectorY + iCntY >= dfSECTOR_MAX_Y)
			continue;

		for (iCntX = -1; iCntX < 2; iCntX++)
		{
			if (iSectorX + iCntX < 0 || iSectorX + iCntX >= dfSECTOR_MAX_X)
				continue;

			pSectorAround->Around[pSectorAround->iCount].iX = iSectorX + iCntX;
			pSectorAround->Around[pSectorAround->iCount].iY = iSectorY + iCntY;
			pSectorAround->iCount++;
		}
	}
}

void GetCollisionSectorAround(LPRECT colRc, st_SECTOR_AROUND* pSectorAround)
{
	pSectorAround->iCount = 0;

	// 왼쪽 위
	int idxX[4] = { colRc->left / dfSECTOR_WIDTH , colRc->right / dfSECTOR_WIDTH, colRc->right / dfSECTOR_WIDTH, colRc->left / dfSECTOR_WIDTH };
	int idxY[4] = { colRc->top / dfSECTOR_HEIGHT, colRc->top / dfSECTOR_HEIGHT, colRc->bottom / dfSECTOR_HEIGHT, colRc->bottom / dfSECTOR_HEIGHT };

	for (int i = 0; i < 4; i++)
	{
		if (0 <= idxY[i] && idxY[i] < dfSECTOR_MAX_Y && 0 <= idxX[i] && idxX[i] < dfSECTOR_MAX_X)
		{
			bool isSameValue = false;
			for (int j = 0; j < pSectorAround->iCount; j++)
			{
				if (idxX[i] == pSectorAround->Around->iX && idxY[i] == pSectorAround->Around->iY)
				{
					isSameValue = true;
					break;
				}
			}

			if (isSameValue == true)
				continue;

			pSectorAround->Around[pSectorAround->iCount].iX = idxX[i];
			pSectorAround->Around[pSectorAround->iCount].iY = idxY[i];
			pSectorAround->iCount++;
		}
	}
}

// 캐릭터의 현재 좌표 shX, shY 으로 섹터위치를 계산하여 해당 섹터에 넣음
void Sector_AddCharacter(st_CHARACTER* pCharacter)
{
	g_Sector[pCharacter->CurSector.iY][pCharacter->CurSector.iX].push_back(pCharacter);
}

// 캐릭터의 현재 좌표 shX, shY 으로 섹터를 계산하여 해당 섹터에서 삭제
void Sector_RemoveCharacter(st_CHARACTER* pCharacter)
{
	g_Sector[pCharacter->OldSector.iY][pCharacter->OldSector.iX].remove(pCharacter);
}

// 위의 RemoveCharacter, AddCharacter 를 사용하여
// 현재 위치한 섹터에서 삭제 후 현재의 좌표로 섹터를 새롭게 계산하여 해당 섹터에 넣음.
bool Sector_UpdateCharacter(st_CHARACTER* pCharacter)
{
	int idxX = pCharacter->shX / dfSECTOR_WIDTH;
	int idxY = pCharacter->shY / dfSECTOR_HEIGHT;

	if (pCharacter->CurSector.iX == idxX && pCharacter->CurSector.iY == idxY)
		return false;

	pCharacter->OldSector = pCharacter->CurSector;
	pCharacter->CurSector.iX = idxX;
	pCharacter->CurSector.iY = idxY;

	Sector_RemoveCharacter(pCharacter);

	Sector_AddCharacter(pCharacter);	

	return true;
}

void CharacterSectorUpdatePacket(st_CHARACTER* pCharacter)
{
	//-----------------------------------------------------------------------
	// 1. 이전 섹터에서 없어진 부분에 - 캐릭터 삭제 메시지
	// 2. 이동하는 캐릭터에게 이전 섹터에서 제외된 섹터의 캐릭터들 삭제 시키는 메시지
	// 3. 새로 추가된 섹터에 - 캐릭터 생성 메시지 & 이동 메시지
	// 4. 이동하는 캐릭터에게 - 새로 진입한 섹터의 캐릭터들 생성 메시지
	//-----------------------------------------------------------------------
	st_SECTOR_AROUND RemoveSector, AddSector;
	st_CHARACTER* pExistCharacter;
	list<st_CHARACTER*>* pSectorList;
	list<st_CHARACTER*>::iterator ListIter;
	CSerializeBuffer Packet;

	int iCnt;
	GetUpdateSectorAround(pCharacter, &RemoveSector, &AddSector);
	//-----------------------------------------------------------------------
	// 1. RemoveSector 에 캐릭터 삭제 패킷 보내기
	//-----------------------------------------------------------------------
	mpDeleteCharacter(&Packet, pCharacter->dwSessionID);
	for (iCnt = 0; iCnt < RemoveSector.iCount; iCnt++)
	{
		// 특정섹터 한 공간에만 메시지를 전달하는 함수
		SendPacket_SectorOne(RemoveSector.Around[iCnt].iX,
			RemoveSector.Around[iCnt].iY,
			&Packet, NULL);
	}

	//-----------------------------------------------------------------------
	// 2. 지금 움직이는 녀석에게, RemoveSector 의 캐릭터들 삭제 패킷 보내기
	//-----------------------------------------------------------------------
	for (iCnt = 0; iCnt < RemoveSector.iCount; iCnt++)
	{
		pSectorList = &g_Sector[RemoveSector.Around[iCnt].iY][RemoveSector.Around[iCnt].iX];
		for (ListIter = pSectorList->begin(); ListIter != pSectorList->end(); ListIter++)
		{
			mpDeleteCharacter(&Packet, (*ListIter)->dwSessionID);
			// 특정 클라이언트 한 명 에게만 메시지를 전달하는 함수
			SendPacket_Unicast(pCharacter->pSession, &Packet);
		}
	}

	//-----------------------------------------------------
	// 3. AddSector 에 캐릭터 생성 패킷 보내기
	//-----------------------------------------------------
	mpCreateOtherCharacter(&Packet, pCharacter->dwSessionID,
		pCharacter->byDirection,
		pCharacter->shX,
		pCharacter->shY,
		pCharacter->chHP);

	for (iCnt = 0; iCnt < AddSector.iCount; iCnt++)
	{
		SendPacket_SectorOne(AddSector.Around[iCnt].iX, AddSector.Around[iCnt].iY, &Packet, NULL);
	}

	//-----------------------------------------------------
	// 3-1. AddSector 에 생성된 캐릭터 이동 패킷 보내기
	//-----------------------------------------------------
	mpMoveStart(&Packet, pCharacter->dwSessionID,
		pCharacter->byMoveDirection,
		pCharacter->shX,
		pCharacter->shY);

	for (iCnt = 0; iCnt < AddSector.iCount; iCnt++)
	{
		SendPacket_SectorOne(AddSector.Around[iCnt].iX, AddSector.Around[iCnt].iY, &Packet, NULL);
	}

	//-----------------------------------------------------
	// 4. 이동한 녀석에게 AddSector 에 있는 캐릭터들 생성 패킷 보내기
	//-----------------------------------------------------
	for (iCnt = 0; iCnt < AddSector.iCount; iCnt++)
	{
		//-----------------------------------------------------
		// 얻어진 섹터를 돌면서 섹터리스트 접근
		//-----------------------------------------------------
		pSectorList = &g_Sector[AddSector.Around[iCnt].iY][AddSector.Around[iCnt].iX];

		//-----------------------------------------------------
		// 해당 섹터마다 등록된 캐릭터들을 뽑아서 생성패킷 만들어 보냄
		//-----------------------------------------------------
		for (ListIter = pSectorList->begin(); ListIter != pSectorList->end(); ListIter++)
		{
			pExistCharacter = *ListIter;
			// 내가 아닌 경우에만 !
			if (pExistCharacter != pCharacter)
			{
				mpCreateOtherCharacter(&Packet,
					pExistCharacter->dwSessionID,
					pExistCharacter->byDirection,
					pExistCharacter->shX,
					pExistCharacter->shY,
					pExistCharacter->chHP);
				SendPacket_Unicast(pCharacter->pSession, &Packet);

				//-----------------------------------------------------
				// 새 AddSector 의 캐릭터가 걷고 있었다면 이동 패킷 만들어서 보냄
				//-----------------------------------------------------
				switch (pExistCharacter->dwAction)
				{
				case dfACTION_MOVE_DD:
				case dfACTION_MOVE_LD:
				case dfACTION_MOVE_LL:
				case dfACTION_MOVE_LU:
				case dfACTION_MOVE_UU:
				case dfACTION_MOVE_RU:
				case dfACTION_MOVE_RR:
				case dfACTION_MOVE_RD:
					mpMoveStart(&Packet,
						pExistCharacter->dwSessionID,
						pExistCharacter->byMoveDirection,
						pExistCharacter->shX,
						pExistCharacter->shY);
					SendPacket_Unicast(pCharacter->pSession, &Packet);
					break;
				}
			}
		}
	}
}

void GetUpdateSectorAround(st_CHARACTER* pCharacter, st_SECTOR_AROUND* pRemoveSector, st_SECTOR_AROUND* pAddSector)
{
	int iCntOld, iCntCur;
	bool bFind;
	st_SECTOR_AROUND OldSectorAround, CurSectorAround;
	OldSectorAround.iCount = 0;
	CurSectorAround.iCount = 0;
	pRemoveSector->iCount = 0;
	pAddSector->iCount = 0;

	GetSectorAround(pCharacter->OldSector.iX, pCharacter->OldSector.iY, &OldSectorAround);
	GetSectorAround(pCharacter->CurSector.iX, pCharacter->CurSector.iY, &CurSectorAround);
	
	//------------------------------------------------------------
	// 이전 (OldSector) 섹터 정보 중, 신규섹터(AddSector) 에는 없는 정보를 찾아서 RemoveSector 에 넣음.
	//------------------------------------------------------------
	for (iCntOld = 0; iCntOld < OldSectorAround.iCount; iCntOld++)
	{
		bFind = false;
		for (iCntCur = 0; iCntCur < CurSectorAround.iCount; iCntCur++)
		{
			if (OldSectorAround.Around[iCntOld].iX == CurSectorAround.Around[iCntCur].iX &&
				OldSectorAround.Around[iCntOld].iY == CurSectorAround.Around[iCntCur].iY)
			{
				bFind = true;
				break;
			}
		}
		if (bFind == false)
		{
			pRemoveSector->Around[pRemoveSector->iCount] = OldSectorAround.Around[iCntOld];
			pRemoveSector->iCount++;
		}
	}

	//------------------------------------------------------------
	// 현재 (CurSector) 섹터 정보중 이전 (OldSector) 섹터에 없는 정보를 찾아서 AddSector 에 넣음.
	//------------------------------------------------------------
	for (iCntCur = 0; iCntCur < CurSectorAround.iCount; iCntCur++)
	{
		bFind = false;
		for (iCntOld = 0; iCntOld < OldSectorAround.iCount; iCntOld++)
		{
			if (OldSectorAround.Around[iCntOld].iX == CurSectorAround.Around[iCntCur].iX &&
				OldSectorAround.Around[iCntOld].iY == CurSectorAround.Around[iCntCur].iY)
			{
				bFind = true;
				break;
			}
		}
		if (bFind == false)
		{
			pAddSector->Around[pAddSector->iCount] = CurSectorAround.Around[iCntCur];
			pAddSector->iCount++;
		}
	}
}