#include "stdafx.h"
#include "Listener.h"

CListener::CListener()
{

}

CListener::~CListener()
{

}

bool CListener::Init(const wchar_t* ipStr, int port)
{
	// listen socket
	m_ListenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_ListenSock == INVALID_SOCKET)
	{
		printf("socket Error : %d\n", WSAGetLastError());
		return false;
	}

	ZeroMemory(&m_ListenInfo, sizeof(sockaddr_in));
	m_ListenInfo.sin_family = AF_INET;
	InetPton(AF_INET, ipStr, &m_ListenInfo.sin_addr);
	m_ListenInfo.sin_port = htons(port);

	// bind
	int bindRet = bind(m_ListenSock, (sockaddr*)&m_ListenInfo, sizeof(m_ListenInfo));
	if (bindRet == SOCKET_ERROR)
	{
		printf("bind Error : %d\n", WSAGetLastError());
		closesocket(m_ListenSock);
		return false;
	}

	// listen
	int listenRet = listen(m_ListenSock, SOMAXCONN);
	if (listenRet == SOCKET_ERROR)
	{
		printf("listen Error : %d\n", WSAGetLastError());
		closesocket(m_ListenSock);
		return false;
	}

	printf("Server Opened... (made by Nabzacko)\n");

	return true;
}

bool CListener::InitDomain(const wchar_t* DomainStr, int port)
{
	// listen socket
	m_ListenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_ListenSock == INVALID_SOCKET)
	{
		printf("socket Error : %d\n", WSAGetLastError());
		return false;
	}

	ZeroMemory(&m_ListenInfo, sizeof(sockaddr_in));
	m_ListenInfo.sin_family = AF_INET;
	if (DomainToIP(DomainStr, &m_ListenInfo.sin_addr) == false)
	{
		return false;
	}
	m_ListenInfo.sin_port = port;

	// bind
	int bindRet = bind(m_ListenSock, (sockaddr*)&m_ListenInfo, sizeof(m_ListenInfo));
	if (bindRet == SOCKET_ERROR)
	{
		printf("bind Error : %d\n", WSAGetLastError());
		closesocket(m_ListenSock);
		return false;
	}

	// listen
	int listenRet = listen(m_ListenSock, SOMAXCONN);
	if (listenRet == SOCKET_ERROR)
	{
		printf("listen Error : %d\n", WSAGetLastError());
		closesocket(m_ListenSock);
		return false;
	}

	printf("Server Opened... (made by Nabzacko)\n");

	return true;
}

void CListener::Release()
{
	closesocket(m_ListenSock);
}

SOCKET CListener::Accept(sockaddr_in* client_info, int* infoSize)
{
	return accept(m_ListenSock, (sockaddr*)client_info, infoSize);
}

bool CListener::DomainToIP(const WCHAR* szDomain, IN_ADDR* pAddr)
{
	ADDRINFOW* pAddrInfo;  // HOSTENT와 유사

	// 캐시되어 있는 DNS 값을 가져온다. (ping과 동일)
	if (GetAddrInfo(szDomain, L"0", NULL, &pAddrInfo) != 0)
	{
		return false;
	}

	SOCKADDR_IN* pSockAddr;
	pSockAddr = (SOCKADDR_IN*)pAddrInfo->ai_addr;
	*pAddr = pSockAddr->sin_addr;
	FreeAddrInfo(pAddrInfo);
	return true;
}

bool CListener::DomainToIPList(const WCHAR* szDomain, IN_ADDR* pAddr, int iAddrLen)
{
	ADDRINFOW* pAddrInfo;  // HOSTENT와 유사

	// 캐시되어 있는 DNS 값을 가져온다. (ping과 동일)
	if (GetAddrInfo(szDomain, L"0", NULL, &pAddrInfo) != 0)
	{
		return false;
	}

	ADDRINFOW* pNextAddr = pAddrInfo;
	for (int i = 0; i < iAddrLen; i++)
	{
		if (pNextAddr == nullptr)
			break;

		SOCKADDR_IN* pSockAddr = (SOCKADDR_IN*)pNextAddr->ai_addr;
		pAddr[i] = pSockAddr->sin_addr;
		pNextAddr = pNextAddr->ai_next;
	}
	FreeAddrInfo(pAddrInfo);
	return true;
}