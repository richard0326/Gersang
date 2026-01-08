#include "stdafx.h"
#include "Resource.h"
#include "GameMgr.h"
#include "Lib/FPSTimer.h"
#include "Lib/SerializeBuffer.h"
#include "NetworkMgr.h"
#include "RenderMgr.h"
#include "ResourceMgr.h"
#include "ObjectMgr.h"
#include "Sprite.h"
#include "BaseObject.h"
#include "PlayerObject.h"
#include <windowsx.h>
#pragma comment(lib, "imm32.lib")
#include "Lib/ObjectPool.h"

DECLARE_SINGLETON_IN_CPP(CGameMgr);

bool CGameMgr::m_bActiveGame = true;
HIMC CGameMgr::m_hIMC = NULL;
CPlayerObject* CGameMgr::m_playerObject = nullptr;

int x = 100, y = 100;

CGameMgr::CGameMgr()
{

}

CGameMgr::~CGameMgr()
{

}

bool CGameMgr::Init(HINSTANCE hIntance, CONST WCHAR* windowName)
{
	m_hInstance = hIntance;

	wcscpy_s(m_szClassName, L"WindowAPI");
	wcscpy_s(m_szWindowsName, windowName);

	do {
#ifdef _DEBUG
		InitLog(LOG_NO_CONSOLE);
#endif // _DEBUG

		if (SINGLETON(CFPSTimer)->Init(FRAMEPERSECOND) == false)
			break;

		if (CreateWindows() == false)
			break;
		
		if (SINGLETON(CNetworkMgr)->Init(m_hWnd) == false)
			break;

		if (SINGLETON(CRenderMgr)->Init(WINSIZE_X, WINSIZE_Y, COLORBIT_SIZE) == false)
			break;
		
		if (SINGLETON(CObjectPoolMgr)->Init() == false)
			break;

		if (SINGLETON(CObjectPoolMgr)->InitObjectPool<CPlayerObject>(64, true) == false)
			break;

		if (SINGLETON(CObjectMgr)->Init() == false)
			break;
		
		if (SINGLETON(CResourceMgr)->Init() == false)
			break;

		return true;

	} while (0);

	return false;
}

void CGameMgr::Release()
{
	SINGLETON(CResourceMgr)->Release();
	SINGLETON(CObjectMgr)->Release();
	SINGLETON(CObjectPoolMgr)->Release();
	SINGLETON(CRenderMgr)->Release();
	SINGLETON(CNetworkMgr)->Release();
	SINGLETON(CFPSTimer)->Release();
	
	PRO_OUT(L"profiler.txt");

#ifdef _DEBUG
	ReleaseLog();
#endif // _DEBUG		
}

int CGameMgr::runMessageLoop()
{
	MSG msg;

	// 게임을 위한 메시지 루프
	for (;;)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			// 네트워크가 연결된 경우
			if (SINGLETON(CNetworkMgr)->isConnected()) {
				// 게임 처리 함수 호출 !!
				if (RunCycle() == false) {
					break;
				}
			}
		}
	}

	return (int)msg.wParam;
}

bool CGameMgr::CreateWindows()
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = CGameMgr::WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = m_hInstance;
	wcex.hIcon = LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_MY2DGAME));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = m_szClassName;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_MY2DGAME));

	RegisterClassExW(&wcex);

	m_hWnd = CreateWindowW(m_szClassName, m_szWindowsName, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, WINSIZE_X, WINSIZE_Y, nullptr, nullptr, m_hInstance, nullptr);

	if (!m_hWnd)
	{
		return FALSE;
	}

	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);
	SetFocus(m_hWnd);

#pragma region 윈도우 크기 재설정/위치 조정

	RECT WindowRect;
	WindowRect.left = WindowRect.top = 0;
	WindowRect.right = WINSIZE_X;
	WindowRect.bottom = WINSIZE_Y;

	AdjustWindowRectEx(&WindowRect,
		GetWindowStyle(m_hWnd),
		GetMenu(m_hWnd) != NULL,
		GetWindowExStyle(m_hWnd));

	int iX = (GetSystemMetrics(SM_CXSCREEN) / 2) - (WINSIZE_X / 2); //화면의 크기를 얻어서 정 중앙으로
	int iY = (GetSystemMetrics(SM_CYSCREEN) / 2) - (WINSIZE_Y / 2);

	MoveWindow(m_hWnd,
		iX,
		iY,
		WindowRect.right - WindowRect.left,
		WindowRect.bottom - WindowRect.top, TRUE);

#pragma endregion
	// 윈도우 생성 기달림
	Sleep(20);

	return true;
}

LRESULT CALLBACK CGameMgr::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_ACTIVATEAPP:
		m_bActiveGame = (bool)wParam; // wParam에 TRUE가 들어오면 활성화.
		break;

	case WM_IME_KEYDOWN:
	case WM_IME_NOTIFY:
		m_hIMC = ImmAssociateContext(hWnd, NULL);
		ImmAssociateContext(hWnd, m_hIMC);
		return 0;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: 여기에 hdc를 사용하는 그리기 코드를 추가
		SINGLETON(CRenderMgr)->ClearBuffer();
				
		if (m_playerObject != nullptr)
		{
			SINGLETON(CRenderMgr)->RenderTilemap(m_playerObject->GetCurX(), m_playerObject->GetCurY());
		}
		
		SINGLETON(CObjectMgr)->Render();

		SINGLETON(CRenderMgr)->Flip(hdc);

#ifdef _DEBUG	
		WCHAR buf[WCHAR_LEN];
		swprintf_s(buf, L"Logic : %d, Render %d", SINGLETON(CFPSTimer)->GetCurrentLogicFPS(), SINGLETON(CFPSTimer)->GetCurrentRenderFPS());
		TextOut(hdc, 0, 0, buf, wcslen(buf));

#endif 

		EndPaint(hWnd, &ps);
	}
	break;
	
	// 네트워크 메시지
	case WM_NETWORK:
		if (SINGLETON(CNetworkMgr)->NetworkProc(wParam, lParam) == false) {
			AddLog(L"네트워크에 의한 정상 종료\n");			
			PostQuitMessage(0);
		}
		break;

	case WM_DESTROY: // 윈도우 종료시킬 때
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

bool CGameMgr::RunCycle()
{
	// 입력 부분
	if (m_bActiveGame)
	{
		Input();
	}

	// 로직 부분
	bool ret = Update();

	// FPS timer -> 프레임을 넘으면 렌더를 생략한다.
	if (SINGLETON(CFPSTimer)->DontSkipRendering())
	{
		Render();
	}

	return ret;
}

void CGameMgr::Input()
{
	if (m_playerObject != nullptr)
	{
		DWORD dwReadAction = dfACTION_STAND;
		if (GetAsyncKeyState('Z') & 0x8001)
		{
			dwReadAction = dfACTION_ATTACK1;
		}
		else if (GetAsyncKeyState('X') & 0x8001)
		{
			dwReadAction = dfACTION_ATTACK2;
		}
		else if (GetAsyncKeyState('C') & 0x8001)
		{
			dwReadAction = dfACTION_ATTACK3;
		}

		if (dwReadAction != dfACTION_STAND) {
			m_playerObject->InputActionProc(dwReadAction);
		}
		else {
			DWORD dwAction = dfACTION_STAND;
			DWORD dwLRAction = dfACTION_STAND;
			DWORD dwUDAction = dfACTION_STAND;

			if (GetAsyncKeyState(VK_LEFT) & 0x8001)
			{
				dwLRAction = dfACTION_MOVE_LL;
			}
			else if (GetAsyncKeyState(VK_RIGHT) & 0x8001)
			{
				dwLRAction = dfACTION_MOVE_RR;
			}

			if (GetAsyncKeyState(VK_UP) & 0x8001)
			{
				dwUDAction = dfACTION_MOVE_UU;
			}
			else if (GetAsyncKeyState(VK_DOWN) & 0x8001)
			{
				dwUDAction = dfACTION_MOVE_DD;
			}

			if (dwLRAction == dfACTION_MOVE_LL && dwUDAction == dfACTION_MOVE_UU)
			{
				dwAction = dfACTION_MOVE_LU;
			}
			else if (dwLRAction == dfACTION_MOVE_RR && dwUDAction == dfACTION_MOVE_UU)
			{
				dwAction = dfACTION_MOVE_RU;
			}
			else if (dwLRAction == dfACTION_MOVE_RR && dwUDAction == dfACTION_MOVE_DD)
			{
				dwAction = dfACTION_MOVE_RD;
			}
			else if (dwLRAction == dfACTION_MOVE_LL && dwUDAction == dfACTION_MOVE_DD)
			{
				dwAction = dfACTION_MOVE_LD;
			}
			else if (dwLRAction != dfACTION_STAND)
			{
				dwAction = dwLRAction;
			}
			else if (dwUDAction != dfACTION_STAND)
			{
				dwAction = dwUDAction;
			}

			m_playerObject->InputActionProc(dwAction);
		}
	}
}

bool CGameMgr::Update()
{
	// 오브젝트 Run
	bool ret = SINGLETON(CObjectMgr)->Run();

	// 오브젝트 Y축 정렬
	// 이펙트를 가장 뒤로... 나머지는 Y축을 기준으로 정렬
	SINGLETON(CObjectMgr)->YSort();

	// 계속
	if (ret)
	{
		return true;
	}

	// 정상 종료
	return false;
}

void CGameMgr::Render()
{
	InvalidateRect(m_hWnd, NULL, false);
}

void CGameMgr::SetPlayerObject(CPlayerObject* playerObj)
{
	m_playerObject = playerObj;
}