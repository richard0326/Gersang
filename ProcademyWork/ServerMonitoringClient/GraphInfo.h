#pragma once

class CGraph;
class CGraphInfo : public CMonitorInfo {

	enum {
		AROUND_VALUE = 1000, // 출력시 반올림 기준
	};
public:
	CGraphInfo();
	virtual ~CGraphInfo();

	virtual void Render(HDC hdc) override;
	virtual void Push(CONST WCHAR* graphName,int value) override;

	bool AddGraph(CONST WCHAR* graphName, COLORREF graphColor, int penWidth = 2, int queueSize = 20);
	int  GetGraphSize();
	void RenderInfo(HDC hdc, RECT areaRc);
	void AddMaxRange(int maxRange);
	void FindMaxRange();
private:

	CList<CGraph*> m_GraphList;

	HBRUSH		m_BackGroundBrush;
	HPEN		m_BackGroundPen;
	HFONT		m_Font;
	int			m_maxRange;
	int			m_maxCheck;
};