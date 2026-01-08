#pragma once

class CMonitorWindows;
class CMonitoringMgr
{
private:
	CMonitoringMgr();
	~CMonitoringMgr();

	DECLARE_SINGLETON_IN_HEADER(CMonitoringMgr)

public:

	bool Init(HWND hWnd, HINSTANCE hInstance);
	void Release();

	void AddWindow(CONST WCHAR* windowName, int iPosX, int iPosY, int iWidth, int iHeight);
	void SetMonitoring(CONST WCHAR* windowName, TYPE monitoringType);
	void AddMonitoringGraph(CONST WCHAR* windowName, CONST WCHAR* graphName, COLORREF graphColor, int lineWidth, int QueueSize);
	void InsertMonitoringValue(CONST WCHAR* windowName, CONST WCHAR* graphName, int value);
	void RenderWindow();
	void SetMonitorMaxRange(CONST WCHAR* windowName, int range);
	void SetMonitorAlertInfo(CONST WCHAR* windowName, int alertNum, ALERT_TYPE alertType);
	void SetAlertOff();

private:

	HWND		m_hWnd;
	HINSTANCE	m_hInstance;

	CList<CMonitorWindows*> m_MonitoringList;
};