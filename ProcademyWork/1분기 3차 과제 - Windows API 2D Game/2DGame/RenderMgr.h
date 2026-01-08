#pragma once

#define dfMAP_WIDTH				100
#define dfMAP_HEIGHT			100

#define dfTILE_WIDTH			64
#define dfTILE_HEIGHT			64

#define dfTILE_DRAW_HORZ		WINSIZE_X / dfTILE_WIDTH
#define dfTILE_DRAW_VERT		WINSIZE_Y / dfTILE_HEIGHT

class CScreenDib;
class CRenderMgr
{
private:
	CRenderMgr();
	~CRenderMgr();

	DECLARE_SINGLETON_IN_HEADER(CRenderMgr)

public:
	bool Init(int iWidth, int iHeight, int iColorBit);
	void Release();

	void ClearBuffer();

	// 출력 관련 함수들
	void Render(BYTE* sprite, int x, int y, int iWidth, int iHeight, int iCenterX, int iCenterY, int iWidthPercentage = 100);

	// 반투명 출력 함수
	void Render50(BYTE* sprite, int x, int y, int iWidth, int iHeight, int iCenterX, int iCenterY);

	// 스프라이트를 Red 톤 출력 함수
	void RenderRed(BYTE* sprite, int x, int y, int iWidth, int iHeight, int iCenterX, int iCenterY);

	// Render Invisible
	void RenderInvisible(BYTE* sprite, int x, int y, int iWidth, int iHeight, int iCenterX, int iCenterY);

	// 최종 출력 함수
	void Flip(HDC hdc);

	void RenderTilemap(int x, int y);

private:
	CScreenDib* m_DibBuffer;
	int m_cameraX = 0;
	int m_cameraY = 0;
};