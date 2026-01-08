#include "framework.h"
#include "Resource.h"
#include "MonitorWindows.h"
#include "MonitorInfo.h"
#include "GraphInfo.h"

CMonitorWindows::CMonitorWindows(CONST WCHAR* graphName, HINSTANCE hInstance, HWND hWndParent, int iPosX, int iPosY, int iWidth, int iHeight)
	:m_monitorInfo(NULL)
{
	wcscpy_s(m_szWindowClass, graphName);

	m_iWindowPosX = iPosX;
	m_iWindowPosY = iPosY;
	m_iWindowWidth = iWidth;
	m_iWindowHeight = iHeight;
	m_hWndParent = hWndParent;
	m_hInstance = hInstance;

	// 그래프 영역
	m_graphArea.left = 0;
	m_graphArea.right = m_iWindowWidth;
	m_graphArea.top = TITLE_SIZE - 1;
	m_graphArea.bottom = m_iWindowHeight;
	
	m_infoArea.left = m_infoArea.right = m_iWindowWidth;
	m_infoArea.top = TITLE_SIZE - 1;
	m_infoArea.bottom = m_iWindowHeight;

	// 타이틀 영역
	m_titleArea.left = 0;
	m_titleArea.right = m_iWindowWidth;
	m_titleArea.top = 0;
	m_titleArea.bottom = TITLE_SIZE;
	
	// 경보음
	m_isAlertOn = false;
	m_AlertNum = 0;
	m_AlertType = OFF;

	m_TitleBackGroundBrush = CreateSolidBrush(RGB(221, 221, 221));
	m_hTitleFont = CreateFontW(20, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET, 0, 0, ANTIALIASED_QUALITY, 0, L"맑은 고딕");

	CreateMonitorWindow();

	SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);

	// DC 초기화
	CreateWindowDC();

	SetBkMode(m_hMemDC, TRANSPARENT);
}

CMonitorWindows::~CMonitorWindows()
{
	DeleteObject(m_LinePen);
	DeleteObject(m_hTitleFont);

	SelectObject(m_hMemDC, m_hMemBitmapOld);
	DeleteObject(m_hMemBitmap);
	DeleteObject(m_hMemDC);

	DestroyWindow(m_hWnd);

	if (m_monitorInfo != NULL) {
		delete m_monitorInfo;
	}
}


/////////////////////////////////////////////////////////
// 윈도우 프로시저
/////////////////////////////////////////////////////////
LRESULT CALLBACK CMonitorWindows::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		CMonitorWindows* _this = (CMonitorWindows*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
		if (_this != NULL)
		{
			_this->Render(hdc);
		}

		EndPaint(hWnd, &ps);
	}
	break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

/////////////////////////////////////////////////////////
// 데이터 넣기.
/////////////////////////////////////////////////////////
BOOL	CMonitorWindows::InsertData(CONST WCHAR* graphName, int iData)
{
	if (m_isAlertOn == false) 
	{
		if ((m_AlertType == OVER && m_AlertNum < iData) ||
			(m_AlertType == SAME && m_AlertNum == iData) ||
			(m_AlertType == UNDER && m_AlertNum > iData))
		{
			SendMessage(m_hWndParent, UM_ALERT, 0, 0);
			m_isAlertOn = true;
		}
	}
	m_monitorInfo->Push(graphName, iData);
	
	return true;
}

void CMonitorWindows::CreateMonitorWindow()
{
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = CMonitorWindows::WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = m_hInstance;
	wcex.hIcon = LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_SERVERMONITORINGCLIENT));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = m_szWindowClass;
	wcex.hIconSm = NULL;

	RegisterClassExW(&wcex);

	m_hWnd = CreateWindowW(m_szWindowClass, NULL, WS_CHILD | WS_VISIBLE ,
		m_iWindowPosX, m_iWindowPosY, m_iWindowWidth, m_iWindowHeight, m_hWndParent, NULL, m_hInstance, NULL);
}

void CMonitorWindows::CreateWindowDC()
{
	//-------------------------------------
	// 메모리DC 생성 부분
	//-------------------------------------
	RECT Rect;
	GetClientRect(m_hWnd, &Rect);

	HDC hdc = GetDC(m_hWnd);
	m_hMemDC = CreateCompatibleDC(hdc);
	m_hMemBitmap = CreateCompatibleBitmap(hdc, Rect.right, Rect.bottom);
	m_hMemBitmapOld = (HBITMAP)SelectObject(m_hMemDC, m_hMemBitmap);
	ReleaseDC(m_hWnd, hdc);

	PatBlt(m_hMemDC, 0, 0, Rect.right, Rect.bottom, WHITENESS);
}

void CMonitorWindows::Render(HDC _hdc)
{
	// 하얗게 초기화
	PatBlt(m_hMemDC, 0, 0, m_iWindowWidth, m_iWindowHeight, WHITENESS);

	// 제목 그리기
	HBRUSH OldBrush;

	HPEN	hPen = CreatePen(PS_NULL, 0, 0);
	HPEN	hOldPen = (HPEN)SelectObject(m_hMemDC, hPen);

	if (m_isAlertOn == true) {
		HBRUSH allert = CreateSolidBrush(RGB(255, 0, 0));
		OldBrush = (HBRUSH)SelectObject(m_hMemDC, allert);
		Rectangle(m_hMemDC, m_titleArea.left, m_titleArea.top, m_titleArea.right, m_titleArea.bottom);

		SelectObject(m_hMemDC, OldBrush);
		DeleteObject(allert);
	}
	else {
		OldBrush = (HBRUSH)SelectObject(m_hMemDC, m_TitleBackGroundBrush);
		Rectangle(m_hMemDC, m_titleArea.left, m_titleArea.top, m_titleArea.right, m_titleArea.bottom);

		SelectObject(m_hMemDC, OldBrush);
	}
	SelectObject(m_hMemDC, hOldPen);
	DeleteObject(hPen);

	// 글자 출력하기
	HFONT OldFont = (HFONT)SelectObject(m_hMemDC, m_hTitleFont);
	TextOutW(m_hMemDC, m_titleArea.left + 10, m_titleArea.top + 5, m_szWindowClass, wcslen(m_szWindowClass));

	SelectObject(m_hMemDC, OldFont);

	// 그래프 그리기
	m_monitorInfo->Render(m_hMemDC);

	// 줄이 여러개 있는 경우 - 특수한 케이스...
	if (m_monitorInfo->GetType() == TYPE::LINE)
	{
		CGraphInfo* monitor = (CGraphInfo*)m_monitorInfo;
		if (monitor->GetGraphSize() >= 2)
		{
			monitor->RenderInfo(m_hMemDC, m_infoArea);
		}
	}

	BitBlt(_hdc, 0, 0, m_iWindowWidth, m_iWindowHeight, m_hMemDC, 0, 0, SRCCOPY);
}

void CMonitorWindows::SetMonitor(CMonitorInfo* monitor)
{
	if (m_monitorInfo != NULL)
	{
		delete m_monitorInfo;
	}
	m_monitorInfo = monitor;
	monitor->SetAreaRc(m_graphArea);
}

void CMonitorWindows::SetMonitorInfo(VOID* _info)
{
	TYPE* type = (TYPE*)_info;

	if (*type != m_monitorInfo->GetType()) {
		delete _info;
		return;
	}

	switch (*type) {
	case TYPE::LINE: {
		StAddGraphInfo* info = (StAddGraphInfo*)_info;
		CGraphInfo* monitor = (CGraphInfo*)m_monitorInfo;
		if (monitor->GetGraphSize() == 1)
		{
			m_graphArea.right -= INFO_WIDTH;
			m_infoArea.left -= INFO_WIDTH;
			monitor->SetAreaRc(m_graphArea);
		}
		monitor->AddGraph(info->graphName, info->graphColor, info->penWidth, info->queueSize);
	}
		break;
	}

	delete _info;
}

void CMonitorWindows::CallRender()
{
	if (m_monitorInfo != NULL && m_monitorInfo->GetType() == TYPE::LINE)
	{
		CGraphInfo* monitor = (CGraphInfo*)m_monitorInfo;
		monitor->FindMaxRange();
	}

	InvalidateRect(m_hWnd, NULL, false);
}

void CMonitorWindows::SetMaxRange(int maxRange)
{
	if (m_monitorInfo != NULL && m_monitorInfo->GetType() == TYPE::LINE)
	{
		CGraphInfo* monitor = (CGraphInfo*)m_monitorInfo;
		monitor->AddMaxRange(maxRange);
	}
}

void CMonitorWindows::SetAlertOff()
{
	m_isAlertOn = false;
}

void CMonitorWindows::SetAlertInfo(int alertNum, ALERT_TYPE alertType)
{
	m_AlertNum = alertNum;
	m_AlertType = alertType;
}

CONST WCHAR* CMonitorWindows::GetWindowName()
{
	return m_szWindowClass;
}
