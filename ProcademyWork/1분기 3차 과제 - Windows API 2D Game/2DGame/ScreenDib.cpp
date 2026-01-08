#include "stdafx.h"
#include "ScreenDib.h"

CScreenDib::CScreenDib(int iWidth, int iHeight, int iColorBit)
	: m_bypBuffer(nullptr)
{
	CreateDibBuffer(iWidth, iHeight, iColorBit);
}

CScreenDib::~CScreenDib()
{
	ReleaseDibBuffer();
}

void CScreenDib::CreateDibBuffer(int iWidth, int iHeight, int iColorBit)
{
	if (m_bypBuffer != nullptr)
		delete[] m_bypBuffer;

	m_iWidth = iWidth;
	m_iHeight = iHeight;
	m_iColorBit = iColorBit;
	m_iPitch = (m_iWidth * m_iColorBit / 8 + 3) & ~3;

	m_stDibInfo.bmiHeader.biSize = sizeof(BITMAPINFO);
	m_stDibInfo.bmiHeader.biWidth = m_iWidth;
	m_stDibInfo.bmiHeader.biHeight = -m_iHeight;
	m_stDibInfo.bmiHeader.biPlanes = 1;
	m_stDibInfo.bmiHeader.biBitCount = iColorBit;
	m_stDibInfo.bmiHeader.biCompression = BI_RGB;
	m_stDibInfo.bmiHeader.biSizeImage = m_iPitch * m_iHeight;
	m_stDibInfo.bmiHeader.biXPelsPerMeter = m_stDibInfo.bmiHeader.biYPelsPerMeter = 0;
	m_stDibInfo.bmiHeader.biClrUsed = m_stDibInfo.bmiHeader.biClrImportant = 0;

	m_bypBuffer = new BYTE[m_iPitch * m_iHeight];
	memset(m_bypBuffer, 0, sizeof(BYTE) * m_iPitch * m_iHeight);
}

void CScreenDib::ReleaseDibBuffer(void)
{
	delete[] m_bypBuffer;
}

void CScreenDib::Flip(HWND hWnd, int iX, int iY)
{
	HDC hdc = GetDC(hWnd);

	StretchDIBits(hdc, iX, iY, m_iWidth, m_iHeight, 0, 0, m_iWidth, m_iHeight, m_bypBuffer, &m_stDibInfo, DIB_RGB_COLORS, SRCCOPY);

	ReleaseDC(hWnd, hdc);
}

void CScreenDib::Flip(HDC hdc, int iX, int iY)
{
	StretchDIBits(hdc, iX, iY, m_iWidth, m_iHeight, 0, 0, m_iWidth, m_iHeight, m_bypBuffer, &m_stDibInfo, DIB_RGB_COLORS, SRCCOPY);
}

BYTE* CScreenDib::GetDibBuffer(void)
{
	return m_bypBuffer;
}

int CScreenDib::GetWidth(void)
{
	return m_iWidth;
}

int CScreenDib::GetHeight(void)
{
	return m_iHeight;
}

int CScreenDib::GetPitch(void)
{
	return m_iPitch;
}