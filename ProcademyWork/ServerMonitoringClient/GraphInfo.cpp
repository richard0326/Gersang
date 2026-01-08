#include "framework.h"
#include "MonitorInfo.h"
#include "GraphInfo.h"

class CGraph
{
public :
	CGraph(CONST WCHAR* graphName, COLORREF graphColor, int penWidth = 2, int queueSize = 20) {
		wcscpy_s(m_name, graphName);
		m_LinePen = CreatePen(PS_SOLID, penWidth, graphColor);

		m_Queue.resize(queueSize);
		m_Font = CreateFontW(20, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET, 0, 0, ANTIALIASED_QUALITY, 0, L"맑은 고딕");
	}

	~CGraph() {
		DeleteObject(m_LinePen);
		DeleteObject(m_Font);
	}

	void Render(HDC hdc, RECT renderRc, int maxVal) {

		if (m_Queue.size() >= 2)
		{
			// 그래프 그리기
			__int32 size = m_Queue.size() - 1;
			__int32 timeX = 0;
			__int32 slice = (renderRc.right - renderRc.left) / (m_Queue.maxSize() -1);

			// 시작 위치 설정
			__int32 now = 0;
			m_Queue.front(&now);

			__int32 yVal = ((renderRc.bottom - renderRc.top) * now) / maxVal;

			MoveToEx(hdc, timeX, renderRc.bottom - yVal, NULL);

			for (__int32 i = 0; i < size; i++)
			{
				m_Queue.peek(i + 1, &now);

				timeX += slice;
				yVal = ((renderRc.bottom - renderRc.top) * now) / maxVal;

				HPEN OldPen;
				OldPen = (HPEN)SelectObject(hdc, m_LinePen);

				LineTo(hdc, timeX, renderRc.bottom - yVal);

				SelectObject(hdc, OldPen);
			}
		}
	}

	bool Push(int value, int* out = NULL) {
		bool ret = false;
		if (m_Queue.isFull())
		{
			if (out != NULL) {
				m_Queue.front(out);
			}
			m_Queue.pop();
			ret = true;
		}
		
		m_Queue.push(value);
		return ret;
	}

	CONST WCHAR* GetName()
	{
		return m_name;
	}

	void RenderInfo(HDC hdc, int x, int y) {

		HPEN OldPen;
		OldPen = (HPEN)SelectObject(hdc, m_LinePen);

		MoveToEx(hdc, x + 5, y + 10, NULL);
		LineTo(hdc, x + 15, y + 10);

		SelectObject(hdc, OldPen);

		HFONT OldFont;
		OldFont = (HFONT)SelectObject(hdc, m_Font);
		TextOutW(hdc, x + 20, y, m_name, wcslen(m_name));

		SelectObject(hdc, OldFont);
	}

	int GetMaxValue() {
		int max = 0;
		int size = m_Queue.size();
		int out = 0;
		for (int i = 0; i < size; i++)
		{
			m_Queue.peek(i, &out);
			if (out > max)
				max = out;
		}
		return max;
	}

private:
	WCHAR			m_name[WCHAR_LENGTH];
	CQueue<int>		m_Queue;
	HPEN			m_LinePen;
	HFONT			m_Font;
};

CGraphInfo::CGraphInfo()
	: m_maxRange(0)
	, m_maxCheck(0)
{
	m_type = TYPE::LINE;

	m_BackGroundBrush = CreateSolidBrush(RGB(235, 235, 235));
	m_BackGroundPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
	m_Font = CreateFontW(15, 0, 0, 0, 0, 0, 0, 0, HANGEUL_CHARSET, 0, 0, ANTIALIASED_QUALITY, 0, L"맑은 고딕");
}

CGraphInfo::~CGraphInfo() 
{
	DeleteObject(m_BackGroundBrush);
	DeleteObject(m_BackGroundPen);
	DeleteObject(m_Font);

	for (CList<CGraph*>::iterator iter = m_GraphList.begin(); iter != m_GraphList.end(); )
	{
		delete (*iter);
		iter = m_GraphList.erase(iter);
	}
	
}

void CGraphInfo::Render(HDC hdc)
{
	// 배경 그리기
	HBRUSH OldBrush;
	OldBrush = (HBRUSH)SelectObject(hdc, m_BackGroundBrush);

	HPEN	hPen = CreatePen(PS_NULL, 0, 0);
	HPEN	hOldPen = (HPEN)SelectObject(hdc, hPen);

	Rectangle(hdc, m_renderRc.left, m_renderRc.top, m_renderRc.right, m_renderRc.bottom);

	SelectObject(hdc, OldBrush);
	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);

	// 그리드 선 그리기
	__int32 lineDiv = 4;
	__int32 lineY = (m_renderRc.bottom - m_renderRc.top) / lineDiv;

	hOldPen = (HPEN)SelectObject(hdc, m_BackGroundPen);
	for (__int32 i = 1; i < lineDiv; i++)
	{
		MoveToEx(hdc, 1, m_renderRc.top + i * lineY, NULL);
		LineTo(hdc, m_renderRc.right - 1, m_renderRc.top + i * lineY);
	}
	SelectObject(hdc, hOldPen);

	int maxRange = m_maxRange;
	if (m_maxRange == 0)
	{
		// 반올림 기준
		maxRange = m_maxCheck + AROUND_VALUE - ((m_maxCheck + AROUND_VALUE) % AROUND_VALUE);
	}

	HFONT OldFont;
	OldFont = (HFONT)SelectObject(hdc, m_Font);
	for (__int32 i = 0; i < lineDiv; i++) {
		WCHAR buf[WCHAR_LENGTH];
		swprintf_s(buf, L"%d", maxRange / lineDiv * (lineDiv - i));
		
		TextOutW(hdc, m_renderRc.left + 3, m_renderRc.top + i * lineY, buf, wcslen(buf));
	}
	SelectObject(hdc, OldFont);

	for (CList<CGraph*>::iterator iter = m_GraphList.begin(); iter != m_GraphList.end(); iter++)
	{
		(*iter)->Render(hdc, m_renderRc, maxRange);
	}
}

void CGraphInfo::Push(CONST WCHAR* graphName, int value)
{
	for (CList<CGraph*>::iterator iter = m_GraphList.begin(); iter != m_GraphList.end(); iter++)
	{
		if (wcscmp((*iter)->GetName(), graphName) == 0)
		{
			(*iter)->Push(value);
			break;
		}
	}
}

bool CGraphInfo::AddGraph(CONST WCHAR* graphName, COLORREF graphColor, int penWidth, int queueSize)
{
	for (CList<CGraph*>::iterator iter = m_GraphList.begin(); iter != m_GraphList.end(); iter++)
	{
		// 같은 이름의 그래프가 있는 경우
		if (wcscmp((*iter)->GetName(), graphName) == 0)
			return false;
	}

	m_GraphList.push_back(new CGraph(graphName, graphColor, penWidth, queueSize));
	return true;
}

int  CGraphInfo::GetGraphSize()
{
	return m_GraphList.size();
}

void CGraphInfo::RenderInfo(HDC hdc, RECT areaRc)
{
	HPEN	hPen = CreatePen(PS_NULL, 0, 0);
	HPEN	hOldPen = (HPEN)SelectObject(hdc, hPen);
	Rectangle(hdc, areaRc.left - 1, areaRc.top, areaRc.right, areaRc.bottom);
	SelectObject(hdc, hOldPen);
	DeleteObject(hPen);

	__int32 height = 10;
	for (CList<CGraph*>::iterator iter = m_GraphList.begin(); iter != m_GraphList.end(); iter++)
	{
		(*iter)->RenderInfo(hdc, areaRc.left, areaRc.top + height);		
		height += 20;
	}
}

void CGraphInfo::AddMaxRange(int maxRange)
{
	m_maxRange = maxRange;
}

void CGraphInfo::FindMaxRange()
{
	if (m_maxRange == 0)
	{
		int max = 0;
		for (CList<CGraph*>::iterator iter = m_GraphList.begin(); iter != m_GraphList.end(); iter++)
		{
			int ret = (*iter)->GetMaxValue();
			if (max < ret)
				max = ret;
		}
		m_maxCheck = max;
	}
}