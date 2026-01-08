#pragma once

class CMonitorInfo;
class CMonitorWindows
{
public:

	enum {
		MAX_QUEUE = 20,
		TITLE_SIZE = 30,
		INFO_WIDTH = 100,
	};

public:

	CMonitorWindows(CONST WCHAR* graphName, HINSTANCE hInstance, HWND hWndParent, int iPosX, int iPosY, int iWidth, int iHeight);
	~CMonitorWindows();

	/////////////////////////////////////////////////////////
	// 윈도우 프로시저
	/////////////////////////////////////////////////////////
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	/////////////////////////////////////////////////////////
	// 데이터 넣기.
	/////////////////////////////////////////////////////////
	BOOL	InsertData(CONST WCHAR* graphName, int iData);

protected:


	//------------------------------------------------------
	// 윈도우 초기화
	//------------------------------------------------------
	void CreateMonitorWindow();
	void CreateWindowDC();

	//------------------------------------------------------
	// 출력
	//------------------------------------------------------
	void Render(HDC _hdc);

public:
	//------------------------------------------------------
	// 모니터 설정
	//------------------------------------------------------
	void SetMonitor(CMonitorInfo* monitor);
	void SetMonitorInfo(VOID* info);
	void CallRender();
	void SetMaxRange(int maxRange);
	void SetAlertOff();
	void SetAlertInfo(int alertNum, ALERT_TYPE alertType);
	CONST WCHAR* GetWindowName();
private:

	//------------------------------------------------------
	// 부모 윈도우 핸들, 내 윈도우 핸들, 인스턴스 핸들
	//------------------------------------------------------
	HWND			m_hWndParent;
	HWND			m_hWnd;
	HINSTANCE		m_hInstance;
	WCHAR			m_szWindowClass[WCHAR_LENGTH];
	__int32			m_iWindowPosX;
	__int32			m_iWindowPosY;
	__int32			m_iWindowWidth;
	__int32			m_iWindowHeight;

	//------------------------------------------------------
	// 윈도우 위치,크기,색상, 그래프 타입 등.. 자료
	//------------------------------------------------------
	TYPE		m_enGraphType;

	//------------------------------------------------------
	// 더블 버퍼링용 메모리 DC, 메모리 비트맵
	//------------------------------------------------------
	HDC		m_hMemDC;
	HBITMAP	m_hMemBitmap;
	HBITMAP m_hMemBitmapOld;

	//------------------------------------------------------
	// 출력용 GDI 오브젝트
	//------------------------------------------------------
	HPEN		m_LinePen;
	HBRUSH		m_TitleBackGroundBrush;
	HFONT		m_hTitleFont;
	RECT		m_graphArea;
	RECT		m_titleArea;
	RECT		m_infoArea;

	//------------------------------------------------------
	// 데이터
	//------------------------------------------------------
	CMonitorInfo*			m_monitorInfo;

	//------------------------------------------------------
	// 경보음
	//------------------------------------------------------
	bool		m_isAlertOn;
	int			m_AlertNum;
	ALERT_TYPE	m_AlertType;
};
