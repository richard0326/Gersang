#pragma once

class CEchoServer;
class CGameServer
{
private:
	CGameServer();
	~CGameServer();

	DECLARE_SINGLETON_IN_HEADER(CGameServer)

public:
	bool Init(CEchoServer* lanServer);
	void Release();
	void Loop();
	void Shutdown();

private:
	bool Start();
	void ServerControl();
	void SaveData();

private:
	bool m_bShutdown = false;
	bool m_isServerStop = false;
	CEchoServer* m_EchoServer;
};