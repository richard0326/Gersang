// ServerMonitoringClient.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "ServerMonitoringClient.h"

#include "MonitorWindows.h"
#include "MonitoringManager.h"

#include <Mmsystem.h>
#pragma comment( lib, "Winmm.lib" )

enum {
    TIMER_INTERVAL = 100,
    RANDOM_RANGE = 150,
    WINSIZE_X = 1025,
    WINSIZE_Y = 768,
    ALERT_INTERVAL = 1000,
};

HINSTANCE g_hInst;
HWND g_hWnd;
POINT g_prev;

RECT g_WinodowRc;
bool g_TimerOnOff = false;

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
bool Init();
void Release();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    WCHAR WindowName[20] = L"NabzackoWindow";
    WCHAR WindowTitleName[50] = L"Server Monitoring Made By Nabzacko";

    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SERVERMONITORINGCLIENT));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = WindowName;
    wcex.hIconSm = NULL;

    RegisterClassExW(&wcex);

    // 애플리케이션 초기화를 수행합니다:
    g_hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.
    int winPosX = CW_USEDEFAULT;
    int winPosY = 0;
    int winWidth = WINSIZE_X;
    int winHeight = WINSIZE_Y;
    g_WinodowRc.left = g_WinodowRc.right = 0;
    g_WinodowRc.right = WINSIZE_X;
    g_WinodowRc.bottom = WINSIZE_Y;

    HWND hWnd = CreateWindowW(WindowName, WindowTitleName, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        winPosX, winPosY, winWidth, winHeight, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        DWORD ret = GetLastError();
        return FALSE;
    }

    g_hWnd = hWnd;
    if (Init() == false)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;

    // 기본 메시지 루프입니다:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    Release();

    return (int)msg.wParam;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 애플리케이션 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        break;

    case WM_TIMER:
        switch (wParam) {
        case 1:
        {
            static int a = 1000;
            SINGLETON(CMonitoringMgr)->InsertMonitoringValue(L"동시접속자", L"부여서버", a * 2 + RandomINT64 % 200 - 100);
            SINGLETON(CMonitoringMgr)->InsertMonitoringValue(L"동시접속자", L"고구려서버", a + RandomINT64 % 200 - 100);

            a+= 20;
            if (a == 5000)
                a = 1000;

            static int b = 0;
            SINGLETON(CMonitoringMgr)->InsertMonitoringValue(L"CPU", L"CPU 사용량", b);
            b++;
            if (b == 100)
                b = 0;

            // 타이머를 넣어서 1분, 5분에 한번 출력하게끔도 설정할 수 있을 것 같다.
            SINGLETON(CMonitoringMgr)->RenderWindow();
        }
            break;

        case 2:
            // Alert
            if (g_TimerOnOff == true)
            {
                SINGLETON(CMonitoringMgr)->SetAlertOff();
                g_TimerOnOff = false;
                KillTimer(g_hWnd, 2);
                InvalidateRect(hWnd, NULL, false);
            }
            break;
        }

        break;

    case UM_ALERT:
        if (g_TimerOnOff == false)
        {
            PlaySound((LPCTSTR)SND_ALIAS_SYSTEMHAND, NULL, SND_ALIAS_ID | SND_ASYNC);
            g_TimerOnOff = true;
            SetTimer(hWnd, 2, ALERT_INTERVAL, NULL);
            InvalidateRect(hWnd, NULL, false);
        }
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        if (g_TimerOnOff == true) {
            HBRUSH allert = CreateSolidBrush(RGB(255, 100, 100));
            FillRect(hdc, &g_WinodowRc, allert);
            DeleteObject(allert);
        }
        else {
            PatBlt(hdc, 0, 0, WINSIZE_X, WINSIZE_Y, BLACKNESS);
        }

        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

bool Init()
{
    if (Util::Init_Util() == false)
    {
        return false;
    }
    
    if (SINGLETON(CMonitoringMgr)->Init(g_hWnd, g_hInst) == false)
    {
        return false;
    }

    SINGLETON(CMonitoringMgr)->AddWindow(L"동시접속자", 10, 10, 600, 200);
    SINGLETON(CMonitoringMgr)->SetMonitoring(L"동시접속자", LINE);
    SINGLETON(CMonitoringMgr)->AddMonitoringGraph(L"동시접속자", L"부여서버", RGB(248, 118, 109), 2, 50);
    SINGLETON(CMonitoringMgr)->AddMonitoringGraph(L"동시접속자", L"고구려서버", RGB(108, 118, 109), 2, 50);
    SINGLETON(CMonitoringMgr)->SetMonitorMaxRange(L"동시접속자", 0);
    SINGLETON(CMonitoringMgr)->SetMonitorAlertInfo(L"동시접속자", 8000, OVER);

    SINGLETON(CMonitoringMgr)->AddWindow(L"CPU", 10, 220, 200, 200);
    SINGLETON(CMonitoringMgr)->SetMonitoring(L"CPU", LINE);
    SINGLETON(CMonitoringMgr)->AddMonitoringGraph(L"CPU", L"CPU 사용량", RGB(100, 255, 109), 2, 30);
    SINGLETON(CMonitoringMgr)->SetMonitorMaxRange(L"CPU", 100);
    SINGLETON(CMonitoringMgr)->SetMonitorAlertInfo(L"CPU", 90, OVER);

    SetTimer(g_hWnd, 1, TIMER_INTERVAL, NULL);

    return true;
}

void Release()
{
    SINGLETON(CMonitoringMgr)->Release();
    KillTimer(g_hWnd, 1);
}