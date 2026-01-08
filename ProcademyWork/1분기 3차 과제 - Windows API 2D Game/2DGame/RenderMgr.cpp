#include "stdafx.h"
#include "RenderMgr.h"
#include "ScreenDib.h"
#include "Sprite.h"
#include "Animation.h"
#include "ResourceMgr.h"

DECLARE_SINGLETON_IN_CPP(CRenderMgr);

CRenderMgr::CRenderMgr()
	: m_DibBuffer(nullptr)
{

}

CRenderMgr::~CRenderMgr()
{

}

bool CRenderMgr::Init(int iWidth, int iHeight, int iColorBit)
{
	m_DibBuffer = new CScreenDib(iWidth, iHeight, iColorBit);

	return true;
}

void CRenderMgr::Release()
{
	delete m_DibBuffer;
}

void CRenderMgr::ClearBuffer()
{
	if (m_DibBuffer == nullptr)
		return;

	BYTE* buf = m_DibBuffer->GetDibBuffer();
	if (buf == nullptr)
		return;

	// 화면을 0xFFFFFFFF 흰색으로 채웁니다.
	for (int i = 0; i < m_DibBuffer->GetHeight(); ++i)
	{
		memset(buf, 0xFF, m_DibBuffer->GetPitch());
		buf += m_DibBuffer->GetPitch();
	}
}

void CRenderMgr::Render(BYTE* sprite, int x, int y, int iWidth, int iHeight, int iCenterX, int iCenterY, int iWidthPercentage)
{
	if (m_DibBuffer == nullptr)
		return;

	// 시작 x, y 지점을 설정
	int iDrawX = x - iCenterX - m_cameraX;
	int iDrawY = y - iCenterY - m_cameraY;

	int ClippedX = iDrawX;
	int ClippedY = iDrawY;

	int iSpriteX = 0;
	int iSpriteY = 0;

	int percent_Width = iWidth * iWidthPercentage / 100;
	int iSpriteWidth = percent_Width;
	int iSpriteHeight = iHeight;

	if (iDrawX < 0) {
		iSpriteX = -iDrawX;
		iSpriteWidth += iDrawX;
		ClippedX = 0;
	}
	
	if (iDrawY < 0) {
		iSpriteY = -iDrawY;
		iSpriteHeight += iDrawY;
		ClippedY = 0;
	}

	if (m_DibBuffer->GetWidth() - 1 < iDrawX + percent_Width) {
		iSpriteWidth = m_DibBuffer->GetWidth() - iDrawX;
	}

	if (m_DibBuffer->GetHeight() - 1 < iDrawY + iHeight) {
		iSpriteHeight = m_DibBuffer->GetHeight() - iDrawY;
	}

	// 이미지의 Pitch 값
	int iSpritePitch = iWidth * 4;

	// pDestOrigin은 배경 백버퍼의 복사를 시작하고자하는 시작 위치
	BYTE* pDestOrigin = m_DibBuffer->GetDibBuffer();

	// Sprite의 시작 위치
	BYTE* pScrOrigin = sprite;

	// Pitch 값을 많이 사용하기 때문에 변수로 담아둔다.
	int DibPitch = m_DibBuffer->GetPitch();

	// 좌표를 알맞게 이동 시켜준다.
	pDestOrigin += ClippedX * 4;
	pDestOrigin += ClippedY * DibPitch;

	pScrOrigin += iSpriteX * 4;
	pScrOrigin += iSpriteY * iSpritePitch;

	// 페인트에 유리한 DWORD 형식으로 캐스팅
	// pdwDest는 배경 위치, pdwSrc는 image의 위치
	DWORD* pdwDest = (DWORD*)pDestOrigin;
	DWORD* pdwSrc = (DWORD*)pScrOrigin;

	for (int iCntH = 0; iCntH < iSpriteHeight; ++iCntH)
	{
		for (int iCntW = 0; iCntW < iSpriteWidth; ++iCntW)
		{
			if ((*pdwSrc & 0x00ffffff) != 0x00ffffff)			// 이미지를 복사합니다.
				*pdwDest = *pdwSrc;

			++pdwDest;
			++pdwSrc;
		}

		pDestOrigin += DibPitch;
		pdwDest = (DWORD*)pDestOrigin;

		pScrOrigin += iSpritePitch;
		pdwSrc = (DWORD*)pScrOrigin;
	}
}

// 반투명 출력 함수
void CRenderMgr::Render50(BYTE* sprite, int x, int y, int iWidth, int iHeight, int iCenterX, int iCenterY)
{
	if (m_DibBuffer == nullptr)
		return;

	// 시작 x, y 지점을 설정
	int iDrawX = x - iCenterX - m_cameraX;
	int iDrawY = y - iCenterY - m_cameraY;

	int ClippedX = iDrawX;
	int ClippedY = iDrawY;

	int iSpriteX = 0;
	int iSpriteY = 0;

	int iSpriteWidth = iWidth;
	int iSpriteHeight = iHeight;

	if (iDrawX < 0) {
		iSpriteX = -iDrawX;
		iSpriteWidth += iDrawX;
		ClippedX = 0;
	}

	if (iDrawY < 0) {
		iSpriteY = -iDrawY;
		iSpriteHeight += iDrawY;
		ClippedY = 0;
	}

	if (m_DibBuffer->GetWidth() - 1 < iDrawX + iWidth) {
		iSpriteWidth = m_DibBuffer->GetWidth() - iDrawX;
	}

	if (m_DibBuffer->GetHeight() - 1 < iDrawY + iHeight) {
		iSpriteHeight = m_DibBuffer->GetHeight() - iDrawY;
	}

	// 이미지의 Pitch 값
	int iSpritePitch = iWidth * 4;

	// pDestOrigin은 배경 백버퍼의 복사를 시작하고자하는 시작 위치
	BYTE* pDestOrigin = m_DibBuffer->GetDibBuffer();

	// Sprite의 시작 위치
	BYTE* pScrOrigin = sprite;

	// Pitch 값을 많이 사용하기 때문에 변수로 담아둔다.
	int DibPitch = m_DibBuffer->GetPitch();

	// 좌표를 알맞게 이동 시켜준다.
	pDestOrigin += ClippedX * 4;
	pDestOrigin += ClippedY * DibPitch;

	pScrOrigin += iSpriteX * 4;
	pScrOrigin += iSpriteY * iSpritePitch;

	// 페인트에 유리한 DWORD 형식으로 캐스팅
	// pdwDest는 배경 위치, pdwSrc는 image의 위치
	DWORD* pdwDest = (DWORD*)pDestOrigin;
	DWORD* pdwSrc = (DWORD*)pScrOrigin;

	for (int iCntH = 0; iCntH < iSpriteHeight; ++iCntH)
	{
		for (int iCntW = 0; iCntW < iSpriteWidth; ++iCntW)
		{
			if ((*pdwSrc & 0x00ffffff) != 0x00ffffff) {
				
				DWORD DestRed = (((*pdwSrc & 0x00ff0000) + (*pdwDest & 0x00ff0000)) >> 1) & 0x00ff0000;
				DWORD DestGreen = (((*pdwSrc & 0x0000ff00) + (*pdwDest & 0x0000ff00)) >> 1) & 0x0000ff00;
				DWORD DestBlue = (((*pdwSrc & 0x000000ff) + (*pdwDest & 0x000000ff)) >> 1) & 0x000000ff;
				
				*pdwDest = DestRed | DestGreen | DestBlue;
			}

			++pdwDest;
			++pdwSrc;
		}

		pDestOrigin += DibPitch;
		pdwDest = (DWORD*)pDestOrigin;

		pScrOrigin += iSpritePitch;
		pdwSrc = (DWORD*)pScrOrigin;
	}
}

// 스프라이트를 Red 톤 출력 함수
void CRenderMgr::RenderRed(BYTE* sprite, int x, int y, int iWidth, int iHeight, int iCenterX, int iCenterY)
{
	if (m_DibBuffer == nullptr)
		return;

	// 시작 x, y 지점을 설정
	int iDrawX = x - iCenterX - m_cameraX;
	int iDrawY = y - iCenterY - m_cameraY;

	int ClippedX = iDrawX;
	int ClippedY = iDrawY;

	int iSpriteX = 0;
	int iSpriteY = 0;

	int iSpriteWidth = iWidth;
	int iSpriteHeight = iHeight;

	if (iDrawX < 0) {
		iSpriteX = -iDrawX;
		iSpriteWidth += iDrawX;
		ClippedX = 0;
	}

	if (iDrawY < 0) {
		iSpriteY = -iDrawY;
		iSpriteHeight += iDrawY;
		ClippedY = 0;
	}

	if (m_DibBuffer->GetWidth() - 1 < iDrawX + iWidth) {
		iSpriteWidth = m_DibBuffer->GetWidth() - iDrawX;
	}

	if (m_DibBuffer->GetHeight() - 1 < iDrawY + iHeight) {
		iSpriteHeight = m_DibBuffer->GetHeight() - iDrawY;
	}

	// 이미지의 Pitch 값
	int iSpritePitch = iWidth * 4;

	// pDestOrigin은 배경 백버퍼의 복사를 시작하고자하는 시작 위치
	BYTE* pDestOrigin = m_DibBuffer->GetDibBuffer();

	// Sprite의 시작 위치
	BYTE* pScrOrigin = sprite;

	// Pitch 값을 많이 사용하기 때문에 변수로 담아둔다.
	int DibPitch = m_DibBuffer->GetPitch();

	// 좌표를 알맞게 이동 시켜준다.
	pDestOrigin += ClippedX * 4;
	pDestOrigin += ClippedY * DibPitch;

	pScrOrigin += iSpriteX * 4;
	pScrOrigin += iSpriteY * iSpritePitch;

	// 페인트에 유리한 DWORD 형식으로 캐스팅
	// pdwDest는 배경 위치, pdwSrc는 image의 위치
	DWORD* pdwDest = (DWORD*)pDestOrigin;
	DWORD* pdwSrc = (DWORD*)pScrOrigin;

	for (int iCntH = 0; iCntH < iSpriteHeight; ++iCntH)
	{
		for (int iCntW = 0; iCntW < iSpriteWidth; ++iCntW)
		{
			if ((*pdwSrc & 0x00ffffff) != 0x00ffffff) {
				*pdwDest = (*pdwSrc | 0x00ff0000);	// 빨강만 찐하게!
			}

			++pdwDest;
			++pdwSrc;
		}

		pDestOrigin += DibPitch;
		pdwDest = (DWORD*)pDestOrigin;

		pScrOrigin += iSpritePitch;
		pdwSrc = (DWORD*)pScrOrigin;
	}
}

// Render Invisible
void CRenderMgr::RenderInvisible(BYTE* sprite, int x, int y, int iWidth, int iHeight, int iCenterX, int iCenterY)
{
	if (m_DibBuffer == nullptr)
		return;

	// 시작 x, y 지점을 설정
	int iDrawX = x - iCenterX - m_cameraX;
	int iDrawY = y - iCenterY - m_cameraY;

	int ClippedX = iDrawX;
	int ClippedY = iDrawY;

	int iSpriteX = 0;
	int iSpriteY = 0;

	int iSpriteWidth = iWidth;
	int iSpriteHeight = iHeight;

	if (iDrawX < 0) {
		iSpriteX = -iDrawX;
		iSpriteWidth += iDrawX;
		ClippedX = 0;
	}

	if (iDrawY < 0) {
		iSpriteY = -iDrawY;
		iSpriteHeight += iDrawY;
		ClippedY = 0;
	}

	if (m_DibBuffer->GetWidth() - 1 < iDrawX + iWidth) {
		iSpriteWidth = m_DibBuffer->GetWidth() - iDrawX;
	}

	if (m_DibBuffer->GetHeight() - 1 < iDrawY + iHeight) {
		iSpriteHeight = m_DibBuffer->GetHeight() - iDrawY;
	}

	// 이미지의 Pitch 값
	int iSpritePitch = iWidth * 4;

	// pDestOrigin은 배경 백버퍼의 복사를 시작하고자하는 시작 위치
	BYTE* pDestOrigin = m_DibBuffer->GetDibBuffer();

	// Sprite의 시작 위치
	BYTE* pScrOrigin = sprite;

	// Pitch 값을 많이 사용하기 때문에 변수로 담아둔다.
	int DibPitch = m_DibBuffer->GetPitch();

	// 좌표를 알맞게 이동 시켜준다.
	pDestOrigin += ClippedX * 4;
	pDestOrigin += ClippedY * DibPitch;

	pScrOrigin += iSpriteX * 4;
	pScrOrigin += iSpriteY * iSpritePitch;

	// 페인트에 유리한 DWORD 형식으로 캐스팅
	// pdwDest는 배경 위치, pdwSrc는 image의 위치
	DWORD* pdwDest = (DWORD*)pDestOrigin;
	DWORD* pdwSrc = (DWORD*)pScrOrigin;

	for (int iCntH = 0; iCntH < iSpriteHeight; ++iCntH)
	{
		for (int iCntW = 0; iCntW < iSpriteWidth; ++iCntW)
		{
			if ((*pdwSrc & 0x00ffffff) != 0x00ffffff) {
				DWORD DestRed = (*pdwSrc & 0x00ff0000) >> 16;
				DWORD DestGreen = (*pdwSrc & 0x0000ff00) >> 8;
				DWORD DestBlue = (*pdwSrc & 0x000000ff);
				int iD =  (DestRed + DestGreen + DestBlue) / 20;
				*pdwDest = *(pdwDest + iD) & 0xf0f0f0f0;
			}

			++pdwDest;
			++pdwSrc;
		}

		pDestOrigin += DibPitch;
		pdwDest = (DWORD*)pDestOrigin;

		pScrOrigin += iSpritePitch;
		pdwSrc = (DWORD*)pScrOrigin;
	}
}

// 최종 출력 함수
void CRenderMgr::Flip(HDC hdc)
{
	if (m_DibBuffer == nullptr)
		return;

	m_DibBuffer->Flip(hdc);
}

void CRenderMgr::RenderTilemap(int x, int y)
{
	CSprite* tile = nullptr;
	if (SINGLETON(CResourceMgr)->GetSprite(eTILEMAP, &tile))
	{
		// 현재 카메라 좌표
		y = y - 50;
		m_cameraX = x - WINSIZE_X / 2;
		m_cameraY = y - WINSIZE_Y / 2;

		int FixX = m_cameraX - (m_cameraX % dfTILE_WIDTH);
		int FixY = m_cameraY - (m_cameraY % dfTILE_HEIGHT);

		if (x < WINSIZE_X / 2) {
			m_cameraX = 0;
			FixX = 0;
		}

		if (dfRANGE_MOVE_RIGHT - WINSIZE_X / 2 <= x) {
			m_cameraX = dfRANGE_MOVE_RIGHT - WINSIZE_X - 1;
			FixX = m_cameraX - (m_cameraX % dfTILE_WIDTH);
		}

		if (y < WINSIZE_Y / 2) {
			m_cameraY = 0;
			FixY = 0;
		}

		if (dfRANGE_MOVE_BOTTOM - WINSIZE_Y / 2 <= y) {
			m_cameraY = dfRANGE_MOVE_BOTTOM - WINSIZE_Y - 1;
			FixY = m_cameraY - (m_cameraY % dfTILE_HEIGHT);
		}

		for (int iCntV = 0; iCntV <= dfTILE_DRAW_VERT + 1; iCntV++)
		{
			for (int iCntH = 0; iCntH <= dfTILE_DRAW_HORZ; iCntH++)
			{
				tile->Render(dfTILE_WIDTH * iCntH + FixX, dfTILE_HEIGHT * iCntV + FixY);
			}
		}
	}
}