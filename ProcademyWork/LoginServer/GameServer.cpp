#include "stdafx.h"
#include "GameServer.h"
#include "LoginServer.h"

DECLARE_SINGLETON_IN_CPP(CGameServer);

CGameServer::CGameServer()
{

}

CGameServer::~CGameServer()
{

}

bool CGameServer::Init(CLoginServer* LoginServer)
{
	if (LoginServer == nullptr)
	{
		LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"LanServer Nullptr");
		return false;
	}

	m_LoginServer = LoginServer;
	_wsetlocale(LC_ALL, L"korean");
	srand((unsigned int)GetCurrentThreadId());

	// SystemLog 초기 설정
	SYSLOG_DIRECTORY(L"SystemLog");
	SYSLOG_LEVEL(en_LOG_LEVEL::LEVEL_DEBUG);
	SYSLOG_MODE(en_LOG_MODE::MODE_FILE);

	LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"System Log 세팅 완료");

	if (m_LoginServer->Init() == false)
	{
		LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"네트워크 초기화 에러");
		return false;
	}

	if (Start() == false)
		return false;

	LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"GameServer 초기화 Success");

	return true;
}

void CGameServer::Release()
{
	LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"GameServer Release Success");
}

void CGameServer::Loop()
{
	while (!m_bShutdown)		// 서버 메인 루프, 전역의 g_bShutdown 값에 의해 종료 결정
	{
		ServerControl();		// 키보드 입력을 통해서 서버를 제어할 경우 사용

		Sleep(1000);
	}

	// 서버는 함부로 종료해도 안된다.
	// DB에 저장할 데이터나 기타 마무리 할 일들이 모두 끝났는지 확인한 뒤에 꺼주어야 한다.
	SaveData();

	m_LoginServer->Stop();
}

void CGameServer::Shutdown()
{
	m_bShutdown = true;
}

bool CGameServer::Start()
{
	CParser gameParser;

	wchar_t* errMsg = nullptr;
	if (gameParser.ReadBuffer(L"networkInfo.txt", &errMsg) == false)
	{
		LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"Parser Err : %s", errMsg);
		return false;
	}

	wchar_t LISTEN_IP[20];
	int outSize = 20;
	if (gameParser.GetValue(L"Network", L"LISTEN_IP", LISTEN_IP, &outSize) == false)
		return false;

	int LISTEN_Port = 0;
	if (gameParser.GetValue(L"Network", L"LISTEN_Port", &LISTEN_Port) == false)
		return false;

	int CREATE_WORKER_TH = 0;
	if (gameParser.GetValue(L"Network", L"CREATE_WORKER_TH", &CREATE_WORKER_TH) == false)
		return false;

	int USE_WORKER_TH = 0;
	if (gameParser.GetValue(L"Network", L"USE_WORKER_TH", &USE_WORKER_TH) == false)
		return false;

	int NODELAY_OPT = 0;
	if (gameParser.GetValue(L"Network", L"NODELAY_OPT", &NODELAY_OPT) == false)
		return false;

	int MAX_USER = 0;
	if (gameParser.GetValue(L"Network", L"MAX_USER", &MAX_USER) == false)
		return false;

	int RST_OPT = 0;
	if (gameParser.GetValue(L"Network", L"RST_OPT", &RST_OPT) == false)
		return false;

	int KEEPALIVE_OPT = 0;
	if (gameParser.GetValue(L"Network", L"KEEPALIVE_OPT", &KEEPALIVE_OPT) == false)
		return false;

	int SENDOVERLAPPED_OPT = 0;
	if (gameParser.GetValue(L"Network", L"SENDOVERLAPPED_OPT", &SENDOVERLAPPED_OPT) == false)
		return false;

	int SENDQ_SIZE = 0;
	if (gameParser.GetValue(L"Network", L"SENDQ_SIZE", &SENDQ_SIZE) == false)
		return false;

	int RINGBUFFER_SIZE = 0;
	if (gameParser.GetValue(L"Network", L"RINGBUFFER_SIZE", &RINGBUFFER_SIZE) == false)
		return false;

	LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"Parser 세팅 완료");

	if (m_LoginServer->Start(LISTEN_IP, LISTEN_Port,
		CREATE_WORKER_TH, USE_WORKER_TH,
		(bool)NODELAY_OPT, // NODELAY 옵션
		MAX_USER, // 서버 최대 유저
		(bool)RST_OPT, // 링거 옵션 - RST
		(bool)KEEPALIVE_OPT, // keepAlive 옵션
		(bool)SENDOVERLAPPED_OPT, // sendOverlapped
		SENDQ_SIZE,	// SendQ size
		RINGBUFFER_SIZE
	) == false)
		return false;

	return true;
}

void CGameServer::ServerControl()
{
	// 키보드 컨트롤 잠금, 풀림 변수
	static bool bControlMode = false;

	// L : 컨트롤 Lock / U : 컨트롤 Unlock / Q : 서버 종료
	if (_kbhit())
	{
		WCHAR ControlKey = _getwch();

		// 키보드 제어 허용
		if (L'u' == ControlKey || L'U' == ControlKey)
		{
			bControlMode = true;

			// 관련 키 도움말 출력
			wprintf(L"Control Mode : Press Q - Quit \n");
			wprintf(L"Control Mode : Press L - Key Lock \n");
			wprintf(L"Control Mode : Press S - Stop \n");
			wprintf(L"Control Mode : Press P - Profile Save \n");
			wprintf(L"Control Mode : Press C - Crash Dump \n");
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
			m_bShutdown = true;
		}

		if ((L's' == ControlKey || L'S' == ControlKey) && bControlMode)
		{
			if (m_isServerStop)
			{
				if (Start() == false)
					CCrashDump::Crash();
			}
			else
				m_LoginServer->Stop();

			m_isServerStop = !m_isServerStop;
		}

		if ((L'p' == ControlKey || L'P' == ControlKey) && bControlMode)
		{
			PRO_OUT(L"profile.txt");
		}

		if ((L'c' == ControlKey || L'C' == ControlKey) && bControlMode)
		{
			CCrashDump::Crash();
		}

		rewind(stdin);
	}
}

void CGameServer::SaveData()
{
	// DB 저장
}