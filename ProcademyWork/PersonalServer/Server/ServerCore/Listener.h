#pragma once

class CListener
{
public:
	CListener();
	~CListener();

	bool Init(const wchar_t* ipStr, int port);
	bool InitDomain(const wchar_t* DomainStr, int port);
	void Release();

	SOCKET Accept(sockaddr_in* client_info, int* infoSize);

private:
	bool DomainToIP(const WCHAR* szDomain, IN_ADDR* pAddr);
	bool DomainToIPList(const WCHAR* szDomain, IN_ADDR* pAddr, int iAddrLen);

private:
	SOCKET		m_ListenSock;
	sockaddr_in m_ListenInfo;
};