#include "stdafx.h"
#include "Listener.h"

#define IP_ADDRESS		L"0.0.0.0"
#define PORT_NUM		7777

int main()
{
	WSADATA wsa;
	int wsaRet = WSAStartup(MAKEWORD(2, 2), &wsa);
	if (wsaRet != NO_ERROR)
	{
		printf("WSAStartup Error : %d\n", WSAGetLastError());
		return 0;
	}

	CListener listener;
	if (listener.Init(IP_ADDRESS, PORT_NUM) == false)
	{
		printf("listener.Init Error : %d\n");
		return 0;
	}

	while (true)
	{
		printf("Listening...\n");

		// accept
		sockaddr_in client_info;
		int infoSize = sizeof(client_info);

		SOCKET clientSock = listener.Accept(&client_info, &infoSize);
		if (clientSock == INVALID_SOCKET)
		{
			printf("accept Error : %d\n", WSAGetLastError());
			break;
		}

		char recvBuf[1024];
		int recvSize = recv(clientSock, recvBuf, sizeof(recvBuf), 0);
		if (recvSize <= 0)
		{
			printf("recv Error : %d\n", WSAGetLastError());
			break;
		}

		recvBuf[recvSize] = '\0';
		printf("[From client] %s\n", recvBuf);

		int sendSize = send(clientSock, recvBuf, recvSize, 0);
		if (sendSize <= 0)
		{
			printf("send Error : %d\n", WSAGetLastError());
			break;
		}

		closesocket(clientSock);
	}

	// closesocket
	listener.Release();

	return 0;
}