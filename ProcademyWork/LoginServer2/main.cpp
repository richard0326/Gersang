#include "stdafx.h"
#include "GameServer.h"
#include "LoginServer.h"

int main()
{
	CLoginServer* loginServer = new CLoginServer();

	if (SINGLETON(CGameServer)->Init(loginServer) == false)
		return 0;

	SINGLETON(CGameServer)->Loop();

	SINGLETON(CGameServer)->Release();

	delete loginServer;

	return 0;
}