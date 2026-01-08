#pragma once

class CPlayerObject;
class CGameMgr {
private:
	enum {
		WCHAR_LEN = 40,
		FRAMEPERSECOND = 50,
		COLORBIT_SIZE = 32,
	};

	CGameMgr();
	~CGameMgr();

	DECLARE_SINGLETON_IN_HEADER(CGameMgr)

public:

	bool Init(HINSTANCE hIntance, CONST WCHAR* windowName);
	void Release();

	int runMessageLoop();

private:
	bool CreateWindows();

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	bool RunCycle();

	void Input();
	bool Update();
	void Render();

public:
	void SetPlayerObject(CPlayerObject* playerObj);

private:
	HWND			m_hWnd;
	HINSTANCE		m_hInstance;
	WCHAR			m_szClassName[WCHAR_LEN];
	WCHAR			m_szWindowsName[WCHAR_LEN];

	static bool		m_bActiveGame;
	static HIMC		m_hIMC;

	static CPlayerObject*  m_playerObject;
};