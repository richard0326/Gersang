#pragma once

// Dib 출력을 위한 객체
class CScreenDib {
public:
	CScreenDib(int iWidth, int iHeight, int iColorBit);
	~CScreenDib();

protected:

	void CreateDibBuffer(int iWidth, int iHeight, int iColorBit);
	void ReleaseDibBuffer(void);

public:

	void Flip(HWND hWnd, int iX = 0, int iY = 0);
	void Flip(HDC hdc, int iX = 0, int iY = 0);

	BYTE* GetDibBuffer(void);
	int GetWidth(void);
	int GetHeight(void);
	int GetPitch(void);

protected:
	BITMAPINFO	m_stDibInfo;
	BYTE*		m_bypBuffer;

	int			m_iWidth;
	int			m_iHeight;
	int			m_iPitch;
	int			m_iColorBit;
	int			m_iBufferSize;
};