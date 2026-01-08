#pragma once

class CMonitorInfo {
public:
	CMonitorInfo() { }
	virtual ~CMonitorInfo() {}

	virtual void Render(HDC hdc) = 0;
	virtual void Push(CONST WCHAR* graphName, int value) = 0;

	void SetAreaRc(int left, int top, int right, int bottom) {
		m_renderRc.left = left;
		m_renderRc.top = top;
		m_renderRc.right = right;
		m_renderRc.bottom = bottom;
	}

	void SetAreaRc(RECT rc) {
		m_renderRc = rc;
	}

	TYPE GetType() {
		return m_type;
	}

protected:
	TYPE			m_type;
	RECT			m_renderRc;

};