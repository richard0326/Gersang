#include "stdafx.h"
#include "Resource.h"
#include "GameMgr.h"
#include "FPSTimer.h"
#include "NetworkMgr.h"
#include "ObjectMgr.h"
#include "BaseObject.h"
#include "PlayerObject.h"
#include <windowsx.h>
#pragma comment(lib, "imm32.lib")

DECLARE_SINGLETON_IN_CPP(CGameMgr);

bool CGameMgr::m_bActiveGame = true;
HIMC CGameMgr::m_hIMC = NULL;
int g_delayCnt = 0;

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

	srand(0);

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

		if (SINGLETON(CObjectMgr)->Init() == false)
			break;

		return true;

	} while (0);

	return false;
}

void CGameMgr::Release()
{
	SINGLETON(CObjectMgr)->Release();
	SINGLETON(CNetworkMgr)->Release();
	SINGLETON(CFPSTimer)->Release();

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
	wcex.hIcon = LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_MY2DGAMEDUMMY));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = m_szClassName;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_MY2DGAMEDUMMY));

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
		//m_bActiveGame = (bool)wParam; // wParam에 TRUE가 들어오면 활성화.
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

		WCHAR buf[WCHAR_LEN];
		swprintf_s(buf, L"Logic : %d, Render %d\n", SINGLETON(CFPSTimer)->GetCurrentLogicFPS(), SINGLETON(CFPSTimer)->GetCurrentRenderFPS());
		TextOut(hdc, 0, 0, buf, wcslen(buf));
		EndPaint(hWnd, &ps);
	}
	break;
	
	// 네트워크 메시지
	case WM_NETWORK:
		if (SINGLETON(CNetworkMgr)->NetworkProc(wParam, lParam) == false) {
			AddLog(L"에러 메시지 없으면 정상 종료\n");
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
	if (g_delayCnt == FRAMEPERSECOND) {
		SINGLETON(CObjectMgr)->InputActionToDummy();
		g_delayCnt = 0;
	}
	else {
		g_delayCnt++;
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