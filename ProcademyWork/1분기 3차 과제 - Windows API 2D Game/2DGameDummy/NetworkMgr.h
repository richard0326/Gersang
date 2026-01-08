#pragma once

struct stDummy {
	SOCKET			clientSock = INVALID_SOCKET;
	sockaddr_in		clientAddr;
	bool			isConnected = false;
	bool			isSendPossible = false;
	CRingBuffer		SendQ;
	CRingBuffer		RecvQ;
	int				ID = -1;
};

class CSocket;
class CNetworkMgr
{
private:
	CNetworkMgr();
	~CNetworkMgr();

	DECLARE_SINGLETON_IN_HEADER(CNetworkMgr)

public:

	bool Init(HWND hWnd);
	void Release();

	bool NetworkProc(WPARAM wParam, LPARAM lParam);
	bool isConnected();

	bool RecvEvent(stDummy* pDummy);
	bool SendEvent(stDummy* pDummy);
	void CloseEvent(stDummy* pDummy);

	void PacketProc(stDummy* pDummy, BYTE byPacketType, char* Packet);
	bool SendPacket(stDummy* pDummy, char* pPacket, int iPacketSize);
	bool SendPacket(int iID, char* pPacket, int iPacketSize);

private:
	stDummy* m_dummyArr;

	WCHAR			m_wstrIP[25];
	int				m_iPort = 0;
	int				m_dummyMax = 0;
};