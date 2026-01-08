#pragma once

class CSprite
{
	friend class CRenderMgr;

public:
	CSprite();
	~CSprite();

	bool ReadSpriteFile(int SpriteID, const wchar_t* fileName, int iCenterX, int iCenterY);

	void Render(int x, int y, int iWidthPercentage = 100);
	void RenderRed(int x, int y);
	void Render50(int x, int y);

	bool CheckID(int SpriteID);
private:
	int m_iSpriteID;

	BYTE* m_pImage;
	int m_iWidth;
	int m_iHeight;
	int m_iPitch;

	int m_iCenterX;	// 중점 x
	int m_iCenterY;	// 중점 y
};