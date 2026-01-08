#include "stdafx.h"
#include "MMOServer.h"
#include "EchoServer.h"
#include "MakePacket.h"
#include "PacketProc.h"

CEchoServer::st_PLAYER::st_PLAYER()
{

}

CEchoServer::st_PLAYER::~st_PLAYER()
{

}


void CEchoServer::st_PLAYER::OnAuth_ClientJoin(void)
{

}

void CEchoServer::st_PLAYER::OnAuth_ClientLeave(void)
{

}

bool CEchoServer::st_PLAYER::OnAuth_Packet(CSerializeBuffer* pRecvPacket)
{
	WORD Type;
#ifdef MY_DEBUG
	try {
#endif
		(*pRecvPacket) >> Type;
		switch (Type)
		{
		case en_PACKET_CS_GAME_REQ_LOGIN:
		{
			long long AccountNo;
			char SessionKey[64];
			int version;
			if (ppGameReqLogin(pRecvPacket, &AccountNo, SessionKey, &version) == false)
			{
				Disconnect();
				return false;
			}

			CSerializeBuffer* pSendPacket = CSerializeBuffer::Alloc(NETHEADER, 11);
			mpGameResLogin(pSendPacket, 1, AccountNo); // 1 성공, 0 실패

			SendPacket(pSendPacket);

			pSendPacket->DecreaseRefCount();
			SetMode_Game();
			return false;
		}
			break;
		default:
			Disconnect();
			return false;
		}
#ifdef MY_DEBUG
	}
	catch (CSerializeBufException e)
	{
		wprintf(L"%s", (const wchar_t*)e.what());
		int* crash = nullptr;
		*crash = 0;
	}
#endif // MY_DEBUG

	return true;
}

void CEchoServer::st_PLAYER::OnGame_ClientJoin(void)
{

}

void CEchoServer::st_PLAYER::OnGame_ClientLeave(void)
{

}

bool CEchoServer::st_PLAYER::OnGame_Packet(CSerializeBuffer* pRecvPacket)
{
	WORD Type;
#ifdef MY_DEBUG
	try {
#endif
		(*pRecvPacket) >> Type;
		switch (Type)
		{
		case en_PACKET_CS_GAME_REQ_ECHO:
		{
			long long AccountNo;
			long long SendTick;
			if (ppGameReqEcho(pRecvPacket, &AccountNo, &SendTick) == false)
			{
				Disconnect();
				return false;
			}

			CSerializeBuffer* pSendPacket = CSerializeBuffer::Alloc(NETHEADER, 18);
			mpGameResEcho(pSendPacket, AccountNo, SendTick);

			SendPacket(pSendPacket);

			pSendPacket->DecreaseRefCount();
		}
			break;
		case en_PACKET_CS_GAME_REQ_HEARTBEAT:
			// 시간만 갱신... timeout 당하지 않도록...
			break;
		default:
			Disconnect();
			return false;
		}
#ifdef MY_DEBUG
	}
	catch (CSerializeBufException e)
	{
		wprintf(L"%s", (const wchar_t*)e.what());
		int* crash = nullptr;
		*crash = 0;
	}
#endif // MY_DEBUG
	return true;
}

void CEchoServer::st_PLAYER::OnGame_Release(void)
{

}

CEchoServer::CEchoServer()
{

}

CEchoServer::~CEchoServer()
{

}

bool CEchoServer::Start(const wchar_t* ipWstr, int portNum, int workerCreateCnt, int workerRunningCnt,
	bool bNoDelayOpt, int serverMaxUser, bool bRSTOpt, bool bKeepAliveOpt, bool bOverlappedSend, 
	int SendQSize, int RingBufferSize, int CompleteRecvQSize, int SocketQSize)
{
	{
		CParser EchoParser;
		wchar_t* errMsg = nullptr;
		if (EchoParser.ReadBuffer(L"echoInfo.txt", &errMsg) == false)
		{
			LOG(L"System", en_LOG_LEVEL::LEVEL_SYSTEM, L"chattingParser Err : %s", errMsg);
			return false;
		}

		int PLAYERPOOL_CHUNK = 0;
		if (EchoParser.GetValue(L"Echo", L"PLAYERPOOL_CHUNK", &PLAYERPOOL_CHUNK) == false)
			return false;

		int PLAYERPOOL_NODE = 0;
		if (EchoParser.GetValue(L"Echo", L"PLAYERPOOL_NODE", &PLAYERPOOL_NODE) == false)
			return false;

		if (EchoParser.GetValue(L"Echo", L"TIMEOUT_MILLISEC", &m_timeOut) == false)
			return false;


		m_pPlayerPool = new CLockFreeTlsPoolA<st_PLAYER>(PLAYERPOOL_CHUNK, PLAYERPOOL_NODE, false, true);
	}

	m_shutdown = false;

	// MMOServer의 Session은 외부에서 초기화를 해줘야함.
	st_SESSION** ppSessionArray = CMMOServer::GetSessionPPArr(serverMaxUser);
	for (int i = 0; i < serverMaxUser; ++i)
	{
		ppSessionArray[i] = m_pPlayerPool->Alloc();
		if (ppSessionArray[i] == nullptr)
		{
			CCrashDump::Crash();
		}
	}

	return CMMOServer::Start(ipWstr, portNum, workerCreateCnt, workerRunningCnt, 
		bNoDelayOpt, serverMaxUser, bRSTOpt, bKeepAliveOpt, bOverlappedSend, 
		SendQSize, RingBufferSize, CompleteRecvQSize, SocketQSize);
}

void CEchoServer::Stop()
{

}

// accept 직후
bool CEchoServer::OnConnectionRequest(const wchar_t* ipWstr, int portNum)
{
	// WhiteIP, BlackIP
	return true;
}

void CEchoServer::OnError(const wchar_t* ipWstr, int portNum, int errorCode, const wchar_t* errorMsg)
{
	LOG(L"OnError", en_LOG_LEVEL::LEVEL_ERROR, L"[ IP %s, Port %d ] errMsg  %s , errCode %d ", ipWstr, portNum, errorMsg, errorCode);
}

void CEchoServer::GetMonitoringInfo(long long AcceptTps, long long RecvTps, long long SendTps,
	long long AuthTps, long long GameTps,
	long long acceptCount, long long disconnectCount, int sessionCount,
	int chunkCount, int chunkNodeCount,
	long long sendBytePerSec, long long recvBytePerSec)
{
	printf("=========================================== \n");
	printf(" - Server Library -\n");
	printf(" Key Info                   : U / L ( Unlock / Lock ) \n");
	printf(" Send / Recv / Accept TPS   : %lld / %lld / %d\n", SendTps, RecvTps, AcceptTps);
	printf(" Auth / Game TPS            : %lld / %lld\n", AuthTps, GameTps);
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
	printf("\n - Echo Server -\n");
	printf("=========================================== \n");
}