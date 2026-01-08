#include "stdafx.h"
#include "Sprite.h"
#include "RenderMgr.h"

CSprite::CSprite()
	: m_iCenterX(0)
	, m_iCenterY(0)
	, m_iHeight(0)
	, m_iPitch(0)
	, m_iWidth(0)
	, m_pImage(nullptr)
	, m_iSpriteID(-1)
{

}

CSprite::~CSprite()
{
	delete[] m_pImage;
}

bool CSprite::ReadSpriteFile(int SpriteID, const wchar_t* fileName, int iCenterX, int iCenterY)
{
	m_iSpriteID = SpriteID;

	// 스프라이트 파일을 읽어서 동적 할당에 내용을 채워서 밖으로 보냅니다.
	m_iCenterX = iCenterX;
	m_iCenterY = iCenterY;

	// BITMAPFILEHEADER와 BITMAPINFOHEADER의 데이터를 읽습니다.
	BITMAPFILEHEADER stFileHeader;
	BITMAPINFOHEADER stInfoHeader;

	FILE* pFile;
	_wfopen_s(&pFile, fileName, L"rb");
	fread(&stFileHeader, sizeof(stFileHeader), 1, pFile);
	fread(&stInfoHeader, sizeof(stInfoHeader), 1, pFile);

	m_iWidth = stInfoHeader.biWidth;
	m_iHeight = stInfoHeader.biHeight;

	// 이미지 정보도 채워줍니다.
	m_iPitch = (m_iWidth * (stInfoHeader.biBitCount / 8) + 3) & ~3;
	auto iImageSize = m_iPitch * m_iHeight;
	m_pImage = new BYTE[iImageSize];

	// 파일에 읽기 위한 동적 할당...
	auto pTemp = new BYTE[iImageSize]; 
	fread(pTemp, iImageSize, 1, pFile);

	BYTE* pTempDest = m_pImage;
	BYTE* pTempSrc = pTemp + iImageSize - m_iPitch;

	// 거꾸로 저장되어 있는 이미지를 뒤집지 않은 상태로 저장합니다.
	for (int iCnt = 0; iCnt < m_iHeight; ++iCnt) {

		memcpy(pTempDest, pTempSrc, m_iPitch);
		pTempDest += m_iPitch;
		pTempSrc -= m_iPitch;
	}

	delete[] pTemp;
	fclose(pFile);

	return true;
}

void CSprite::Render(int x, int y, int iWidthPercentage)
{
	SINGLETON(CRenderMgr)->Render(m_pImage, x, y, m_iWidth, m_iHeight, m_iCenterX, m_iCenterY, iWidthPercentage);
}

void CSprite::RenderRed(int x, int y)
{
	SINGLETON(CRenderMgr)->RenderRed(m_pImage, x, y, m_iWidth, m_iHeight, m_iCenterX, m_iCenterY);
}

void CSprite::Render50(int x, int y)
{
	SINGLETON(CRenderMgr)->Render50(m_pImage, x, y, m_iWidth, m_iHeight, m_iCenterX, m_iCenterY);
}

bool CSprite::CheckID(int SpriteID)
{
	if (m_iSpriteID == SpriteID)
	{
		return true;
	}
	return false;
}