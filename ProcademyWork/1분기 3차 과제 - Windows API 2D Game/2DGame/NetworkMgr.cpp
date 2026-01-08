#include "stdafx.h"
#include "NetworkMgr.h"
#include "Lib/RingBuffer.h"
#include "Lib/SerializeBuffer.h"

DECLARE_SINGLETON_IN_CPP(CNetworkMgr);

CRingBuffer g_SendQ;
CRingBuffer g_RecvQ;

CNetworkMgr::CNetworkMgr()
{

}

CNetworkMgr::~CNetworkMgr()
{

}

bool CNetworkMgr::Init(HWND hWnd)
{
	WCHAR* pErr;
	CParser ipParser;
	if (ipParser.ReadBuffer(L"networkInfo.txt", &pErr) == false)
	{
		AddLog(L"Read Buffer err : %s \n", pErr);
		return false;
	}

	int inOutLen = 25; // sizeof로 하면 50이 되서 에러남. 같은 전역 변수를 밀어버림
	if (ipParser.GetValue(L"NetworkTarget", L"IP", m_wstrIP, &inOutLen) == false)
	{
		AddLog(L"GetValue IP err : %s \n", pErr);
		return false;
	}
	
	if (ipParser.GetValue(L"NetworkTarget", L"Port", &m_iPort) == false)
	{
		AddLog(L"GetValue Port err : %s \n", pErr);
		return false;
	}

	// dll 초기화
    WSADATA wsa;
    int wsaRet = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (wsaRet != NO_ERROR) {
        AddLog(L"WSAStartup error : %Id\n", WSAGetLastError());
        return false;
    }

	// 클라이언트 socket 생성
	m_clientSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_clientSock == INVALID_SOCKET) {
		AddLog(L"socket error : %Id\n", WSAGetLastError());
		return false;
	}

	// 서버 주소 등록
	ZeroMemory(&m_clientAddr, sizeof(m_clientAddr));
	m_clientAddr.sin_family = AF_INET;
	InetPton(AF_INET, m_wstrIP, &m_clientAddr.sin_addr);
	m_clientAddr.sin_port = htons(m_iPort);

	int asyncRet = WSAAsyncSelect(m_clientSock, hWnd, WM_NETWORK, FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE );
	if (asyncRet == SOCKET_ERROR) {
		AddLog(L"WSAAsyncSelect error : %Id \n", WSAGetLastError());
		return false;
	}

	int connectRet = connect(m_clientSock, (sockaddr*)&m_clientAddr, sizeof(m_clientAddr));
	if (connectRet == SOCKET_ERROR) {
		if (WSAGetLastError() != WSAEWOULDBLOCK) {
			AddLog(L"Connect error : %Id \n", WSAGetLastError());
			return false;
		}
	}

    return true;
}

void CNetworkMgr::Release()
{
	closesocket(m_clientSock);
	WSACleanup();
}

bool CNetworkMgr::NetworkProc(WPARAM wParam, LPARAM lParam)
{
	if (WSAGETSELECTERROR(lParam) != 0) {
		AddLog(L"WSAGETSELECTERROR : %Id\n", WSAGETSELECTERROR(lParam));
		return false;
	}

	switch (WSAGETSELECTEVENT(lParam))
	{
	case FD_CONNECT:
		AddLog(L"Connect Success \n");
		m_isConnected = true;
		m_isSendPossible = true;
		break;
	case FD_READ:
		if (RecvEvent() == false) {
			return false;
		}
		break;
	case FD_WRITE:
		m_isSendPossible = true;
		if (SendEvent() == false) {
			return false;
		}
		break;
	case FD_CLOSE:
		CloseEvent();
		return false;
		break;
	}
	return true;
}

bool CNetworkMgr::isConnected()
{
	return m_isConnected;
}

bool CNetworkMgr::RecvEvent()
{
	// recvBuf를 안쓰는 방향으로 수정
	// ptr을 이용해서 읽어들이는 방식
	int directEnqSize = g_RecvQ.DirectEnqueueSize();
	int recvRet = recv(m_clientSock, g_RecvQ.GetRearBufferPtr(), directEnqSize, 0);
	if (recvRet == SOCKET_ERROR) {
		if (WSAGetLastError() != WSAEWOULDBLOCK) {
			AddLog(L"recv : %Id\n", WSAGetLastError());
			return false;
		}
		return true;
	}

	if (g_RecvQ.MoveRear(recvRet) == false)
	{
		AddLog(L"MoveRear Fail\n");
		return false;
	}

	// 한번 리시브하도 다음 메시지로 리시브해도 됨. 어차피 같음.

	while (true) {
		if (g_RecvQ.GetUseSize() < sizeof(stPacket_Header))
			break;

		stPacket_Header peekHeader;
		int peekSize = g_RecvQ.Peek((char*)&peekHeader, sizeof(stPacket_Header));

		if (peekHeader.byCode != 0x89) {
			AddLog(L"packet Wrong code Recv \n");
			return false;
		}

		if (g_RecvQ.GetUseSize() < sizeof(stPacket_Header) + peekHeader.bySize)
			break;

		if(g_RecvQ.MoveFront(sizeof(stPacket_Header)) == false) {
			AddLog(L"Recv MoveFront Error \n");
			return false;
		}
		
		CSerializeBuffer sbuf;
		int deqSize = g_RecvQ.Dequeue(sbuf.GetBufferPtr(), peekHeader.bySize);
		if (deqSize != peekHeader.bySize) {
			AddLog(L"deq size Error \n");
			return false;
		}

		sbuf.MoveWritePos(deqSize);

		try {
			PacketProc(peekHeader.byType, &sbuf);
		}
		catch (CSerializeBufException e)
		{
			AddLog((const wchar_t*)e.what());
			return false;
		}
	}

	return true;
}

bool CNetworkMgr::SendEvent()
{
	while (m_isSendPossible)
	{
		int directDeqSize = g_SendQ.DirectDequeueSize();
		if (directDeqSize == 0) {
			break;
		}

		int sendRet = send(m_clientSock, g_SendQ.GetFrontBufferPtr(), directDeqSize, 0);
		if (sendRet == SOCKET_ERROR) {
			if (WSAGetLastError() != WSAEWOULDBLOCK) {
				AddLog(L"send : %Id\n", WSAGetLastError());
				return false;
			}
			m_isSendPossible = false;
			return true;
		}

		if (g_SendQ.MoveFront(sendRet) == false)
		{
			AddLog(L"send MoveFront error\n");
			return false;
		}
	}

	return true;
}

void CNetworkMgr::CloseEvent()
{
	closesocket(m_clientSock);
}

void CNetworkMgr::PacketProc(BYTE byPacketType, CSerializeBuffer* pSerializeBuffer)
{
	switch (byPacketType)
	{
	case dfPACKET_SC_CREATE_MY_CHARACTER:
		netPacketProc_CreateMyCharacter(pSerializeBuffer);
		break;
	case dfPACKET_SC_CREATE_OTHER_CHARACTER:
		netPacketProc_CreateOtherCharacter(pSerializeBuffer);
		break;
	case dfPACKET_SC_DELETE_CHARACTER:
		netPacketProc_DeleteCharacter(pSerializeBuffer);
		break;
	case dfPACKET_SC_MOVE_START:
		netPacketProc_MoveStart(pSerializeBuffer);
		break;
	case dfPACKET_SC_MOVE_STOP:
		netPacketProc_MoveStop(pSerializeBuffer);
		break;
	case dfPACKET_SC_ATTACK1:
		netPacketProc_Attack1(pSerializeBuffer);
		break;
	case dfPACKET_SC_ATTACK2:
		netPacketProc_Attack2(pSerializeBuffer);
		break;
	case dfPACKET_SC_ATTACK3:
		netPacketProc_Attack3(pSerializeBuffer);
		break;
	case dfPACKET_SC_DAMAGE:
		netPacketProc_Damage(pSerializeBuffer);
		break;
	case dfPACKET_SC_SYNC:
		netPacketProc_Sync(pSerializeBuffer);
		break;
	case dfPACKET_SC_ECHO:
		netPacketProc_Echo(pSerializeBuffer);
		break;
	}
}

bool CNetworkMgr::SendPacket(char* pPacket, int iPacketSize)
{
	if (m_isConnected) {
		int enqSize = g_SendQ.Enqueue(pPacket, iPacketSize);
		if (SendEvent() == false)
		{
			return false;
		}
	}

	return true;
}