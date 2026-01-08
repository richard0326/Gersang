#pragma once

class CSocket;
class CSerializeBuffer;
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

	bool RecvEvent();
	bool SendEvent();
	void CloseEvent();

	void PacketProc(BYTE byPacketType, CSerializeBuffer* pSerializeBuffer);
	bool SendPacket(char* pPacket, int iPacketSize);

private:
	SOCKET			m_clientSock;
	sockaddr_in		m_clientAddr;
	bool			m_isConnected = false;
	bool			m_isSendPossible = false;

	WCHAR			m_wstrIP[25];
	int				m_iPort;
};