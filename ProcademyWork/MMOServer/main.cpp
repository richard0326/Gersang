#include "stdafx.h"
#include "EchoServer.h"
#include "GameServer.h"

int main()
{
	CEchoServer* echoServer = new CEchoServer();

	if (SINGLETON(CGameServer)->Init(echoServer) == false)
		return 0;

	SINGLETON(CGameServer)->Loop();

	SINGLETON(CGameServer)->Release();

	delete echoServer;

	return 0;
}
