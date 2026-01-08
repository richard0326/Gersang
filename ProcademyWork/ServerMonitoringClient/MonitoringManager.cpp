#include "framework.h"
#include "MonitoringManager.h"
#include "MonitorWindows.h"
#include "MonitorInfo.h"
#include "GraphInfo.h"

DECLARE_SINGLETON_IN_CPP(CMonitoringMgr);

CMonitoringMgr::CMonitoringMgr()
{
}

CMonitoringMgr::~CMonitoringMgr()
{
}

bool CMonitoringMgr::Init(HWND hWnd, HINSTANCE hInstance)
{
	m_hWnd = hWnd;
	m_hInstance = hInstance;

	return true;
}

void CMonitoringMgr::Release()
{
	for (CList<CMonitorWindows*>::iterator iter = m_MonitoringList.begin(); iter != m_MonitoringList.end(); ) {

		delete (*iter);
		iter = m_MonitoringList.erase(iter);
	}
}

void CMonitoringMgr::AddWindow(CONST WCHAR* windowName, int iPosX, int iPosY, int iWidth, int iHeight)
{
	for (CList<CMonitorWindows*>::iterator iter = m_MonitoringList.begin(); iter != m_MonitoringList.end(); iter++) {

		if (wcscmp((*iter)->GetWindowName(), windowName) == 0) {
			return;
		}
	}

	m_MonitoringList.push_back(new CMonitorWindows(windowName, m_hInstance, m_hWnd, iPosX, iPosY, iWidth, iHeight));
}

void CMonitoringMgr::SetMonitoring(CONST WCHAR* windowName, TYPE monitoringType)
{
	for (CList<CMonitorWindows*>::iterator iter = m_MonitoringList.begin(); iter != m_MonitoringList.end(); iter++) {

		if (wcscmp((*iter)->GetWindowName(), windowName) == 0) {
			switch (monitoringType) {
			case LINE:
				(*iter)->SetMonitor(new CGraphInfo());
				break;
			}
		}
	}
}

void CMonitoringMgr::AddMonitoringGraph(CONST WCHAR* windowName, CONST WCHAR* graphName, COLORREF graphColor, int lineWidth, int QueueSize)
{
	for (CList<CMonitorWindows*>::iterator iter = m_MonitoringList.begin(); iter != m_MonitoringList.end(); iter++) {

		if (wcscmp((*iter)->GetWindowName(), windowName) == 0) {
			(*iter)->SetMonitorInfo(new StAddGraphInfo(TYPE::LINE, graphName, graphColor, lineWidth, QueueSize));
			break;
		}
	}
}

void CMonitoringMgr::InsertMonitoringValue(CONST WCHAR* windowName, CONST WCHAR* graphName, int value)
{
	for (CList<CMonitorWindows*>::iterator iter = m_MonitoringList.begin(); iter != m_MonitoringList.end(); iter++) {

		if (wcscmp((*iter)->GetWindowName(), windowName) == 0) {
			(*iter)->InsertData(graphName, value);
			break;
		}
	}
}

void CMonitoringMgr::RenderWindow()
{
	for (CList<CMonitorWindows*>::iterator iter = m_MonitoringList.begin(); iter != m_MonitoringList.end(); iter++) {

		(*iter)->CallRender();
	}
}

void CMonitoringMgr::SetMonitorMaxRange(CONST WCHAR* windowName, int range)
{
	for (CList<CMonitorWindows*>::iterator iter = m_MonitoringList.begin(); iter != m_MonitoringList.end(); iter++) {

		if (wcscmp((*iter)->GetWindowName(), windowName) == 0) {
			(*iter)->SetMaxRange(range);
			break;
		}
	}
}

void CMonitoringMgr::SetMonitorAlertInfo(CONST WCHAR* windowName, int alertNum, ALERT_TYPE alertType)
{
	for (CList<CMonitorWindows*>::iterator iter = m_MonitoringList.begin(); iter != m_MonitoringList.end(); iter++) {

		if (wcscmp((*iter)->GetWindowName(), windowName) == 0) {
			(*iter)->SetAlertInfo(alertNum, alertType);
			break;
		}
	}
}

void CMonitoringMgr::SetAlertOff()
{
	for (CList<CMonitorWindows*>::iterator iter = m_MonitoringList.begin(); iter != m_MonitoringList.end(); iter++) {

		(*iter)->SetAlertOff();
	}
}