#include "stdafx.h"
#include "NetworkMgr.h"
#include "ObjectMgr.h"

DECLARE_SINGLETON_IN_CPP(CNetworkMgr);

bool g_isConnected = false;
int g_connectCnt = 0;

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

	if (ipParser.GetValue(L"NetworkTarget", L"Dummy", &m_dummyMax) == false)
	{
		AddLog(L"GetValue Dummy err : %s \n", pErr);
		return false;
	}

	// dll 초기화
    WSADATA wsa;
    int wsaRet = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (wsaRet != NO_ERROR) {
        AddLog(L"WSAStartup error : %Id\n", WSAGetLastError());
        return false;
    }

	m_dummyArr = new stDummy[m_dummyMax];
	for (int i = 0; i < m_dummyMax; ++i) {
		// 클라이언트 socket 생성
		m_dummyArr[i].clientSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (m_dummyArr[i].clientSock == INVALID_SOCKET) {
			AddLog(L"socket error : %Id\n", WSAGetLastError());
			return false;
		}

		// 서버 주소 등록
		ZeroMemory(&m_dummyArr[i].clientAddr, sizeof(m_dummyArr[i].clientAddr));
		m_dummyArr[i].clientAddr.sin_family = AF_INET;
		InetPton(AF_INET, m_wstrIP, &m_dummyArr[i].clientAddr.sin_addr);
		m_dummyArr[i].clientAddr.sin_port = htons(m_iPort);

		int asyncRet = WSAAsyncSelect(m_dummyArr[i].clientSock, hWnd, WM_NETWORK, FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE);
		if (asyncRet == SOCKET_ERROR) {
			AddLog(L"WSAAsyncSelect error : %Id \n", WSAGetLastError());
			return false;
		}

		int connectRet = connect(m_dummyArr[i].clientSock, (sockaddr*)&m_dummyArr[i].clientAddr, sizeof(m_dummyArr[i].clientAddr));
		if (connectRet == SOCKET_ERROR) {
			if (WSAGetLastError() != WSAEWOULDBLOCK) {
				AddLog(L"Connect error : %Id \n", WSAGetLastError());
				return false;
			}
		}
		Sleep(10);
	}

    return true;
}

void CNetworkMgr::Release()
{
	for (int i = 0; i < m_dummyMax; ++i) {
		closesocket(m_dummyArr[i].clientSock);
	}
	delete[] m_dummyArr;
	WSACleanup();
}

bool CNetworkMgr::NetworkProc(WPARAM wParam, LPARAM lParam)
{
	if (WSAGETSELECTERROR(lParam) != 0) {
		AddLog(L"WSAGETSELECTERROR : %Id\n", WSAGETSELECTERROR(lParam));
		return false;
	}

	stDummy* pDummy = nullptr;
	for (int i = 0; i < m_dummyMax; ++i)
	{
		if (m_dummyArr[i].clientSock != INVALID_SOCKET && m_dummyArr[i].clientSock == wParam) {
			pDummy = &m_dummyArr[i];
		}
	}

	if (pDummy == nullptr)
	{
		// 삭제된 id와 중복된 메시지일 경우 예외처리를 해도 됨.
		AddLog(L"msg after delete \n");
		return true;
	}

	switch (WSAGETSELECTEVENT(lParam))
	{
	case FD_CONNECT:
		AddLog(L"Connect Success \n");
		g_connectCnt++;
		if (g_connectCnt == m_dummyMax) {
			g_isConnected = true;
		}
		pDummy->isConnected = true;
		pDummy->isSendPossible = true;
		break;
	case FD_READ:
		if (RecvEvent(pDummy) == false) {
			return false;
		}
		break;
	case FD_WRITE:
		pDummy->isSendPossible = true;
		if (SendEvent(pDummy) == false) {
			return false;
		}
		break;
	case FD_CLOSE:
		CloseEvent(pDummy);
		return false;
		break;
	}
	return true;
}

bool CNetworkMgr::isConnected()
{
	return g_isConnected;
}

bool CNetworkMgr::RecvEvent(stDummy* pDummy)
{
	// recvBuf를 안쓰는 방향으로 수정
	// ptr을 이용해서 읽어들이는 방식
	int directEnqSize = pDummy->RecvQ.DirectEnqueueSize();
	int recvRet = recv(pDummy->clientSock, pDummy->RecvQ.GetRearBufferPtr(), directEnqSize, 0);
	if (recvRet == SOCKET_ERROR) {
		if (WSAGetLastError() != WSAEWOULDBLOCK) {
			AddLog(L"recv : %Id\n", WSAGetLastError());
			return false;
		}
		return true;
	}

	if (pDummy->RecvQ.MoveRear(recvRet) == false)
	{
		AddLog(L"MoveRear Fail\n");
		return false;
	}

	while (true) {
		if (pDummy->RecvQ.GetUseSize() < sizeof(stPacket_Header))
			break;

		char deqBuf[20];
		stPacket_Header* peekHeader = (stPacket_Header*)&deqBuf;
		int peekSize = pDummy->RecvQ.Peek((char*)peekHeader, sizeof(stPacket_Header));

		if (peekHeader->byCode != 0x89) {
			AddLog(L"packet Wrong code Recv \n");
			return false;
		}

		if (pDummy->RecvQ.GetUseSize() < sizeof(stPacket_Header) + peekHeader->bySize)
			break;

		if(pDummy->RecvQ.MoveFront(sizeof(stPacket_Header)) == false) {
			AddLog(L"Recv MoveFront Error \n");
			return false;
		}
		
		int deqSize = pDummy->RecvQ.Dequeue(deqBuf + sizeof(stPacket_Header), peekHeader->bySize);

		PacketProc(pDummy, peekHeader->byType, deqBuf);
	}

	return true;
}

bool CNetworkMgr::SendEvent(stDummy* pDummy)
{
	while (pDummy->isSendPossible)
	{
		int directDeqSize = pDummy->SendQ.DirectDequeueSize();
		if (directDeqSize == 0) {
			break;
		}

		int sendRet = send(pDummy->clientSock, pDummy->SendQ.GetFrontBufferPtr(), directDeqSize, 0);
		if (sendRet == SOCKET_ERROR) {
			if (WSAGetLastError() != WSAEWOULDBLOCK) {
				AddLog(L"send : %Id\n", WSAGetLastError());
				return false;
			}
			pDummy->isSendPossible = false;
			return true;
		}

		if (pDummy->SendQ.MoveFront(sendRet) == false)
		{
			AddLog(L"send MoveFront error\n");
			return false;
		}
	}

	return true;
}

void CNetworkMgr::CloseEvent(stDummy* pDummy)
{
	//SINGLETON(CObjectMgr)->EraseByObjectID(pDummy->ID);
	pDummy->clientSock = INVALID_SOCKET;
	closesocket(pDummy->clientSock);
}

void CNetworkMgr::PacketProc(stDummy* pDummy, BYTE byPacketType, char* Packet)
{
	switch (byPacketType)
	{
	case dfPACKET_SC_CREATE_MY_CHARACTER:
		netPacketProc_CreateMyCharacter(Packet, &pDummy->ID);
		break;
	case dfPACKET_SC_CREATE_OTHER_CHARACTER:
		netPacketProc_CreateOtherCharacter(Packet);
		break;
	case dfPACKET_SC_DELETE_CHARACTER:
		netPacketProc_DeleteCharacter(Packet);
		break;
	case dfPACKET_SC_MOVE_START:
		netPacketProc_MoveStart(Packet);
		break;
	case dfPACKET_SC_MOVE_STOP:
		netPacketProc_MoveStop(Packet);
		break;
	case dfPACKET_SC_ATTACK1:
		netPacketProc_Attack1(Packet);
		break;
	case dfPACKET_SC_ATTACK2:
		netPacketProc_Attack2(Packet);
		break;
	case dfPACKET_SC_ATTACK3:
		netPacketProc_Attack3(Packet);
		break;
	case dfPACKET_SC_DAMAGE:
		netPacketProc_Damage(Packet);
		break;
	}
}

bool CNetworkMgr::SendPacket(stDummy* pDummy, char* pPacket, int iPacketSize)
{
	if (pDummy->isConnected) {
		int enqSize = pDummy->SendQ.Enqueue(pPacket, iPacketSize);
		if (SendEvent(pDummy) == false)
		{
			return false;
		}
	}

	return true;
}

bool CNetworkMgr::SendPacket(int iID, char* pPacket, int iPacketSize)
{
	stDummy* pDummy = nullptr;
	for (int i = 0; i < m_dummyMax; ++i) {
		if (m_dummyArr[i].clientSock != INVALID_SOCKET && m_dummyArr[i].ID == iID)
		{
			pDummy = &m_dummyArr[i];
			break;
		}
	}

	if (pDummy == nullptr)
		return false;

	if (pDummy->isConnected) {
		int enqSize = pDummy->SendQ.Enqueue(pPacket, iPacketSize);
		if (SendEvent(pDummy) == false)
		{
			return false;
		}
	}

	return true;
}