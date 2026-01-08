#include "stdafx.h"
#include <Windows.h>
#include <iostream>
#include <ctime>

#include "Log.h"

#define LOG_WINDOW_TITLE	L"Log Window"
#define LOG_WINDOW_STARTX		1000
#define LOG_WINDOW_STARTY		0
#define LOG_WINDOW_SIZEX		500
#define LOG_WINDOW_SIZEY		768

class CLog
{
public:
	CLog(void);
	~CLog(void);

	// 로그 창 설정
	void	logInit(unsigned int target, const wchar_t* fileName = NULL);

	// 로그 찍기
	int		addlog(const wchar_t* szBuff);

	// 정리
	void	release(void);

private:
	// 파일로 에러 메세지 저장
	void	errorFile(const wchar_t* strmsg, ...);

	// 로그 윈도우 생성
	static void	createLogWindow();

	static LRESULT CALLBACK wndProc(HWND hWnd, UINT uMsg, WPARAM wparam, LPARAM lparam);
	static DWORD WINAPI WindowLogThread(LPVOID lpThreadParameter);
private:

	unsigned int	m_target;
	wchar_t			m_filename[MAX_PATH];
	static HWND			m_wndLog;
	static HWND			m_wndListBox;
};

CLog g_logger;

HWND CLog::m_wndLog = NULL;
HWND CLog::m_wndListBox = NULL;

void InitLog(unsigned int target, const wchar_t* fileName)
{
	g_logger.logInit(target, fileName);
}

int AddLog(const wchar_t* strmsg, ...)
{
	wchar_t	szBuff[256];

	va_list ap;
	va_start(ap, strmsg);

	wvsprintf(szBuff, strmsg, ap);

	va_end(ap);

	return g_logger.addlog(szBuff);
}

void ReleaseLog()
{
	g_logger.release();
}

CLog::CLog(void)
	: m_target(LOG_NONE)
{
	m_filename[0] = NULL;
}


CLog::~CLog(void)
{
	release();
}


// 로그 창 설정
void CLog::logInit(unsigned int target, const wchar_t* fileName)
{
	_wsetlocale(LC_ALL, L"korean");
	m_target = target;

	// 파일 이름 저장
	if (target & LOG_FILE && fileName != NULL)
	{
		wcscpy_s(m_filename, MAX_PATH, fileName);
	}
	else
	{
		m_filename[0] = NULL;
	}

	if (m_target & LOG_FILE)
	{
		// 새로 파일 만들기
		wchar_t szfile[256];
		if (m_filename[0] != NULL)
		{
			wsprintf(szfile, L"%s", m_filename);
		}
		else
		{
			SYSTEMTIME		st;
			GetLocalTime(&st);
			wsprintf(szfile, L"ERROR_LOG_%d년%d월%d일.txt", st.wYear, st.wMonth, st.wDay);
		}

		FILE* fp = NULL;
		_wfopen_s(&fp, szfile, L"w");
		fclose(fp);
	}

	if (target & LOG_WINDOW)
	{
		CloseHandle(CreateThread(NULL, 0, WindowLogThread, nullptr, 0, nullptr));
		Sleep(100);
	}

	addlog(L"=== Begin Log ===\n");
}

// 로그 찍기
int CLog::addlog(const wchar_t* szBuff)
{
#ifdef _CONSOLE
	// 콘솔 출력일 경우
	if (m_target & LOG_CONSOLE)
	{
		wprintf(L"%s", szBuff);
	}
#endif

	// file 출력일 경우
	if (m_target & LOG_FILE)
	{
		errorFile(L"%s", szBuff);
	}

	// 로그 윈도우에 출력할 경우
	if (m_target & LOG_WINDOW)
	{
		wchar_t	tempBuff[256] = { 0, };
		for (int i = 0; szBuff[i] != '\0'; ++i) {
			if (szBuff[i] == '\n') {
				tempBuff[i] = '\0';
				break;
			}
			tempBuff[i] = szBuff[i];
		}

		SendMessage(m_wndListBox, LB_ADDSTRING, 0, (LPARAM)tempBuff);
		UINT32 n = SendMessage(m_wndListBox, LB_GETCOUNT, 0, 0L) - 1;
		SendMessage(m_wndListBox, LB_SETCURSEL, (WPARAM)n, 0L);
	}

	return 1;
}

// 파일로 에러 메세지 저장
void CLog::errorFile(const wchar_t* strmsg, ...)
{
	wchar_t szfile[256];

	if (m_filename[0] != NULL)
	{
		wsprintf(szfile, L"%s", m_filename);
	}
	else
	{
		SYSTEMTIME		st;
		GetLocalTime(&st);
		wsprintf(szfile, L"ERROR_LOG_%d년%d월%d일.txt", st.wYear, st.wMonth, st.wDay);
	}

	// C 스타일의 파일 입출력
	FILE* fp = NULL;
	_wfopen_s(&fp, szfile, L"at");

	// 에러처리
	if (fp != NULL)
	{
		wchar_t szBuff[256];

		va_list		ap;

		va_start(ap, strmsg);

		wvsprintf(szBuff, strmsg, ap);

		va_end(ap);

		// 파일 쓰기
		fputws(szBuff, fp);

		fclose(fp);
	}
}

// 정리
void CLog::release(void)
{
	if (m_wndLog != NULL)
	{
		DestroyWindow(m_wndLog);
	}
}

// 로그 윈도우 생성
void CLog::createLogWindow()
{
	POINT	pos;
	SIZE	size;
	RECT	rc;

	WNDCLASS	wc;

	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = (WNDPROC)CLog::wndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"LogWnd";

	RegisterClass(&wc);

	size.cx = LOG_WINDOW_SIZEX;
	size.cy = LOG_WINDOW_SIZEY;

	pos.x = LOG_WINDOW_STARTX + 8;
	pos.y = LOG_WINDOW_STARTY;

	HWND		hParentWnd = NULL;
	HINSTANCE	hInst = NULL;

	m_wndLog = CreateWindow(
		L"LogWnd",
		LOG_WINDOW_TITLE,
		WS_POPUP | WS_CAPTION | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		pos.x, pos.y, size.cx, size.cy, hParentWnd, NULL,
		hInst, NULL);

	GetClientRect(m_wndLog, &rc);
	m_wndListBox = CreateWindow(L"listbox", L"", WS_CHILD | WS_VSCROLL | WS_HSCROLL,
		0, 0, rc.right, rc.bottom,
		m_wndLog, NULL, GetModuleHandle(NULL), NULL);

	ShowWindow(m_wndLog, SW_SHOW);
	ShowWindow(m_wndListBox, SW_SHOW);
}

LRESULT CALLBACK CLog::wndProc(HWND hWnd, UINT uMsg, WPARAM wparam, LPARAM lparam)
{
	switch (uMsg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
	}
	break;

	case WM_DESTROY: // 윈도우 종료시킬 때
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, uMsg, wparam, lparam);
	}
	return 0;
}

DWORD WINAPI CLog::WindowLogThread(LPVOID lpThreadParameter)
{
	CLog::createLogWindow();

	// 메세지 처리
	MSG		Message;  // 메세지 구조체

	// 메세지 루프 ( 사용자로부터의 메세지를 처리한다. )
	while (GetMessage(&Message, 0, 0, 0))
	{
		TranslateMessage(&Message); // 키보드 입력 메세지 관리 ( a 키 누르면 z 티가 눌렸다는 메세지 발생)
		DispatchMessage(&Message);	// 메세지를 가지고 프로시져 함수 호출하는 역할.
	}

	return Message.wParam;
}