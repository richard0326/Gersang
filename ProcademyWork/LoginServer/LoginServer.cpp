#include "stdafx.h"
#include "LoginServer.h"
#include "MakePacket.h"
#include "PacketProc.h"

struct st_PLAYER
{
	unsigned long long SessionID;
	INT64 AccountNo;
	char SessionKey[64];
	WCHAR ID[20];
	WCHAR Nickname[20];
	WCHAR ChattingServerIP[16];
	USHORT Port;
	BYTE Status;
};

CLoginServer::CLoginServer()
{

}

CLoginServer::~CLoginServer()
{

}

bool CLoginServer::Start(const wchar_t* ipWstr, int portNum, int workerCreateCnt, int workerRunningCnt,
	bool bNoDelayOpt, int serverMaxUser, bool bRSTOpt, bool bKeepAliveOpt, bool bOverlappedSend, int SendQSize, int RingBufferSize)
{
	m_tlsDatabaseIndex = TlsAlloc();
	if (m_tlsDatabaseIndex == TLS_OUT_OF_INDEXES)
		return false;

	int CHUNK_SIZE = 0;
	int NODE_SIZE = 0;
	int PLAYERQUEUE_SIZE = 0;
	{
		CParser loginParser;

		wchar_t* errMsg = nullptr;
		if (loginParser.ReadBuffer(L"loginInfo.txt", &errMsg) == false)
		{
			LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"loginParser Err : %s", errMsg);
			return false;
		}

		int STRSIZE = 16;
		if (loginParser.GetValue(L"LoginInfo", L"GAME_IP", m_GameServerIP, &STRSIZE) == false)
			return false;

		int GAME_PORT = 0;
		if (loginParser.GetValue(L"LoginInfo", L"GAME_PORT", &GAME_PORT) == false)
			return false;

		m_GameServerPort = GAME_PORT;

		STRSIZE = 16;
		if (loginParser.GetValue(L"LoginInfo", L"CHATTING_IP_LOOP", m_ChattingServerIP[0], &STRSIZE) == false)
			return false;
		
		int CHATTING_PORT = 0;
		if (loginParser.GetValue(L"LoginInfo", L"CHATTING_PORT_LOOP", &CHATTING_PORT) == false)
			return false;

		m_ChattingServerPort[0] = CHATTING_PORT;

		STRSIZE = 16;
		if (loginParser.GetValue(L"LoginInfo", L"CHATTING_IP_1", m_ChattingServerIP[1], &STRSIZE) == false)
			return false;

		CHATTING_PORT = 0;
		if (loginParser.GetValue(L"LoginInfo", L"CHATTING_PORT_1", &CHATTING_PORT) == false)
			return false;

		m_ChattingServerPort[1] = CHATTING_PORT;

		STRSIZE = 16;
		if (loginParser.GetValue(L"LoginInfo", L"CHATTING_IP_2", m_ChattingServerIP[2], &STRSIZE) == false)
			return false;

		CHATTING_PORT = 0;
		if (loginParser.GetValue(L"LoginInfo", L"CHATTING_PORT_2", &CHATTING_PORT) == false)
			return false;

		if (loginParser.GetValue(L"LoginInfo", L"PLAYERPOOL_CHUNK", &CHUNK_SIZE) == false)
			return false;

		if (loginParser.GetValue(L"LoginInfo", L"PLAYERPOOL_NODE", &NODE_SIZE) == false)
			return false;

		if (loginParser.GetValue(L"LoginInfo", L"PLAYERQUEUE_SIZE", &PLAYERQUEUE_SIZE) == false)
			return false;

		m_ChattingServerPort[2] = CHATTING_PORT;
	}
	LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"[ Login System ] loginInfo.txt read Success");

	{
		WCHAR DatabaseIP[16];
		USHORT DatabasePort = 0;
		WCHAR DatabaseUser[20];
		WCHAR DatabasePassword[20];
		WCHAR DatabaseName[20];

		CParser databaseParser;

		wchar_t* errMsg = nullptr;
		if (databaseParser.ReadBuffer(L"databaseInfo.txt", &errMsg) == false)
		{
			LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"databaseParser Err : %s", errMsg);
			return false;
		}

		int STRSIZE = 16;
		if (databaseParser.GetValue(L"Database", L"DB_IP", DatabaseIP, &STRSIZE) == false)
			return false;

		int DATABASE_PORT = 0;
		if (databaseParser.GetValue(L"Database", L"DB_PORT", &DATABASE_PORT) == false)
			return false;
		
		DatabasePort = DATABASE_PORT;

		STRSIZE = 20;
		if (databaseParser.GetValue(L"Database", L"DB_USER", DatabaseUser, &STRSIZE) == false)
			return false;

		STRSIZE = 20;
		if (databaseParser.GetValue(L"Database", L"DB_PW", DatabasePassword, &STRSIZE) == false)
			return false;

		STRSIZE = 20;
		if (databaseParser.GetValue(L"Database", L"DB_DBNAME", DatabaseName, &STRSIZE) == false)
			return false;


		m_pTlsDBConnecter = new CTlsDBConnector(DatabaseIP, DatabaseUser, DatabasePassword, DatabaseName, DatabasePort);
	}
	LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"[ Login System ] databaseInfo.txt read Success");

	{
		CParser redisParser;

		wchar_t* errMsg = nullptr;
		if (redisParser.ReadBuffer(L"redisInfo.txt", &errMsg) == false)
		{
			LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"redisParser Err : %s", errMsg);
			return false;
		}

		int STRSIZE = 16;
		if (redisParser.GetValue(L"Redis", L"REDIS_IP", m_RedisIP, &STRSIZE) == false)
			return false;

		int REDIS_PORT = 0;
		if (redisParser.GetValue(L"Redis", L"REDIS_PORT", &REDIS_PORT) == false)
			return false;

		m_RedisPort = REDIS_PORT;

		if (redisParser.GetValue(L"Redis", L"REDIS_EXISTSEC", &m_RedisExistTime) == false)
			return false;
	}
	LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"[ Login System ] redisInfo.txt read Success");

	m_playerPool = new CLockFreeTlsPoolA<st_PLAYER>(CHUNK_SIZE, NODE_SIZE);
	
	m_redisQueue = new CLockFreeQueue<st_PLAYER*>(PLAYERQUEUE_SIZE);
	m_hRedisEvent = CreateEvent(nullptr, true, false, nullptr);

	m_hRedisThread = (HANDLE)_beginthreadex(nullptr, 0, RedisThread, this, 0, nullptr);

	return CNetServer::Start(ipWstr, portNum, workerCreateCnt, workerRunningCnt, bNoDelayOpt, serverMaxUser, bRSTOpt, bKeepAliveOpt, bOverlappedSend, SendQSize, RingBufferSize);
}

void CLoginServer::Stop()
{
	delete m_playerPool;
	delete m_redisQueue;
	CloseHandle(m_hRedisEvent);
	CloseHandle(m_hRedisThread);

	TlsFree(m_tlsDatabaseIndex);
}

// accept 직후
bool CLoginServer::CLoginServer::OnConnectionRequest(const wchar_t* ipWstr, int portNum)
{
	return true;
}

// Accept 후 접속처리 완료 후 호출.
void CLoginServer::OnClientJoin(unsigned long long SessionID)
{
}

// Release 후 호출
void CLoginServer::OnClientLeave(unsigned long long SessionID)
{

}

// 패킷/메시지 수신 완료 후
int CLoginServer::OnRecv(unsigned long long SessionID, CSerializeBuffer* pPacket)
{
	WORD Type;
	CSerializeBuffer* pRecvPacket = pPacket;
	CLockFreeTlsPoolA<st_PLAYER>* pPlayerPool = m_playerPool;

#ifdef MY_DEBUG
	try {
#endif
		(*pRecvPacket) >> Type;

		if (en_PACKET_CS_LOGIN_REQ_LOGIN != Type)
		{
			LOG(L"WrongType", en_LOG_LEVEL::LEVEL_DEBUG, L"Type Fail");

			DisconnectSession(SessionID);
			return 0;
		}

		st_PLAYER* pPlayer = pPlayerPool->Alloc();
		if (pPlayer == nullptr)
		{
			CCrashDump::Crash();
		}
		pPlayer->SessionID = SessionID;

		if (ppLoginReqLogin(pRecvPacket, &pPlayer->AccountNo, pPlayer->SessionKey) == false)
		{
			LOG(L"LoginReqLogin", en_LOG_LEVEL::LEVEL_DEBUG, L"ppLoginReqLogin Fail");

			DisconnectSession(SessionID);
			pPlayerPool->Free(pPlayer);
			return 0;
		}

		wchar_t IpStr[16];
		u_short usPort;
		if (GetIP_Port(SessionID, IpStr, &usPort) == false)
		{
			LOG(L"GetIP_Port", en_LOG_LEVEL::LEVEL_DEBUG, L"Worker GetIP_Port Fail");

			DisconnectSession(SessionID);
			pPlayerPool->Free(pPlayer);
			return 0;
		}

		if (wcscmp(IpStr, L"127.0.0.1") != 0)
		{
			if (wcscmp(IpStr, L"10.0.1.2") == 0)
			{
				wcscpy_s(pPlayer->ChattingServerIP, m_ChattingServerIP[1]);
				pPlayer->Port = m_ChattingServerPort[1];
			}
			else
			{
				wcscpy_s(pPlayer->ChattingServerIP, m_ChattingServerIP[2]);
				pPlayer->Port = m_ChattingServerPort[2];
			}
		}
		else
		{
			wcscpy_s(pPlayer->ChattingServerIP, m_ChattingServerIP[0]);
			pPlayer->Port = m_ChattingServerPort[0];
		}
		
		CDBConnector* dbConnector = m_pTlsDBConnecter->GetDBConnector();

		//dbConnector->Query(L"SELECT `userid`, `usernick` FROM v_account WHERE `accountno` = %lld;", pPlayer->AccountNo);
		dbConnector->Query(L"SELECT `userid`, `usernick` FROM account WHERE `accountno` = %lld;", pPlayer->AccountNo);
		MYSQL_ROW sqlrow = dbConnector->FetchRow();
		if (sqlrow == nullptr)
		{
			dbConnector->FreeResult();
			LOG(L"dbError", en_LOG_LEVEL::LEVEL_DEBUG, L"[ IP %s, Port %d ] accountno find Fail", IpStr, usPort);

			SendPlayerMessage(pPlayer, dfLOGIN_STATUS_FAIL);
			DisconnectSession(pPlayer->SessionID);
			pPlayerPool->Free(pPlayer);
			return 0;
		}

		ConvertMultiByteToWideChar_NoAlloc(sqlrow[0], pPlayer->ID, 20);
		ConvertMultiByteToWideChar_NoAlloc(sqlrow[1], pPlayer->Nickname, 20);

		dbConnector->FreeResult();

		m_redisQueue->Enqueue(pPlayer);
		SetEvent(m_hRedisEvent);
#ifdef MY_DEBUG
	}
	catch (CSerializeBufException e)
	{
		wprintf(L"%s", (const wchar_t*)e.what());
		int* crash = nullptr;
		*crash = 0;
	}
#endif // MY_DEBUG

	return 1; // 0 실패, 1 성공
}

void CLoginServer::OnError(const wchar_t* ipWstr, int portNum, int errorCode, const wchar_t* errorMsg)
{
	LOG(L"OnError", en_LOG_LEVEL::LEVEL_ERROR, L"[ IP %s, Port %d ] errMsg  %s , errCode %d ", ipWstr, portNum, errorMsg, errorCode);
}

void CLoginServer::GetMonitoringInfo(long long AcceptTps, long long RecvTps, long long SendTps,
	long long acceptCount, long long disconnectCount, int sessionCount,
	int chunkCount, int chunkNodeCount,
	long long sendBytePerSec, long long recvBytePerSec)
{
	long long temp = m_successCount;
	long long SuccessTps = temp - m_successPrevCount;
	m_successPrevCount = temp;

	printf("=========================================== \n");
	printf(" - Server Library -\n");
	printf(" Key Info                   : U / L ( Unlock / Lock ) \n");
	printf(" Send / Recv / Accept TPS   : %lld / %lld / %d\n", SendTps, RecvTps, AcceptTps);
	printf(" Session Count              : %d \n", sessionCount);
#ifdef MY_DEBUG
	printf(" Accept / Disconnect Count  : %lld / %lld \n", acceptCount, disconnectCount);
	printf(" Packet Chunk / Node Count  : %d / %d \n", chunkCount, chunkNodeCount);
#else
	printf(" Accept Count               : %lld \n", acceptCount);
	printf(" Packet Chunk Count         : %d \n", chunkCount);
#endif

#ifdef MY_DEBUG
	printf(" Send / Recv BytePerSec     : %lld / %lld \n", sendBytePerSec, recvBytePerSec);
#endif
	printf("\n - Login Server -\n");
#ifdef MY_DEBUG
	printf(" Player Chunk / Node Count  : %d / %d \n", m_playerPool->GetChunkSize(), m_playerPool->GetNodeSize());
#else
	printf(" Player Chunk Count         : %d \n", m_playerPool->GetChunkSize());
#endif
	printf(" Redis Queue Size           : %d \n", m_redisQueue->GetSize());
	printf(" Success Count              : %d \n", SuccessTps);
	printf("=========================================== \n");
}

unsigned int __stdcall CLoginServer::RedisThread(void* pParameter)
{
	CLoginServer* pThis = (CLoginServer*)pParameter;
	HANDLE hRedisEvent = pThis->m_hRedisEvent;
	CLockFreeQueue<st_PLAYER*>* redisQueue = pThis->m_redisQueue;
	CRedisConnector redisConnector(pThis->m_RedisIP, pThis->m_RedisPort);
	wchar_t* GameServerIP = pThis->m_GameServerIP;
	USHORT GameServerPort = pThis->m_GameServerPort;
	int existTime = pThis->m_RedisExistTime;

	CLockFreeTlsPoolA<st_PLAYER>* pPlayerPool = pThis->m_playerPool;

	for (;;)
	{
		WaitForSingleObject(hRedisEvent, INFINITE);

		for (;;)
		{
			st_PLAYER* pPlayer;
			if (redisQueue->Dequeue(&pPlayer) == false)
				break;

			redisConnector.set(pPlayer->AccountNo, pPlayer->SessionKey);
			//if (redisConnector.setIfNot(pPlayer->AccountNo, pPlayer->SessionKey) == false)
			//{
			//	LOG(L"redisError", en_LOG_LEVEL::LEVEL_DEBUG, L"redis set Fail");
			//	pThis->SendPlayerMessage(pPlayer, dfLOGIN_STATUS_FAIL);
			//	pThis->DisconnectSession(pPlayer->SessionID);
			//	pPlayerPool->Free(pPlayer);
			//	continue;
			//}

			if (pThis->SendPlayerMessage(pPlayer, dfLOGIN_STATUS_OK) == false)
			{
				pThis->DisconnectSession(pPlayer->SessionID);
				pPlayerPool->Free(pPlayer);
				continue;
			}

			pPlayerPool->Free(pPlayer);
			++(pThis->m_successCount);
		}

		ResetEvent(hRedisEvent);
	}

	return 0;
}

bool CLoginServer::SendPlayerMessage(st_PLAYER* pPlayer, BYTE Status)
{
	CSerializeBuffer* pSendPacket = CSerializeBuffer::Alloc(5, 159);
	if (pSendPacket == nullptr)
	{
		CCrashDump::Crash();
	}

	pPlayer->Status = Status;
	mpLoginResLogin(pSendPacket, pPlayer->AccountNo, pPlayer->Status, pPlayer->ID, pPlayer->Nickname,
		m_GameServerIP, m_GameServerPort,
		pPlayer->ChattingServerIP, pPlayer->Port);

	if (netWSASendPacket(pPlayer->SessionID, pSendPacket) == 0)
	{
		pSendPacket->DecreaseRefCount();
		return false;
	}

	pSendPacket->DecreaseRefCount();
	return true;
}