#pragma once

class CSprite;
class CAnimation;
class CResourceMgr
{
private:
	enum {
		FILE_LENGTH = 100,
	};

	CResourceMgr();
	~CResourceMgr();

	DECLARE_SINGLETON_IN_HEADER(CResourceMgr)

public:
	bool Init();
	void Release();

	//stANIMATION* GetAnimation(SPRITE_AREANUM spriteNum);
	bool GetSprite(int SpriteID, CSprite** out);
	bool GetAnimation(int AnimationID, CAnimation** out);

private:
	bool AddSprite(int SpriteID, const wchar_t* fileName, int iCenterX, int iCenterY, CSprite** out = nullptr);
	bool AddAnimation(int AnimationID, int AnimationDelay, bool isLoop, int iStart, int iEnd);
	bool AddSpriteInAnimation(int AnimationID, int SpriteID, const wchar_t* fileName, int iCenterX, int iCenterY);

	CList<CSprite*>			m_SpriteList;
	CList<CAnimation*>		m_AnimationList;

	CList<CAnimation*>		m_AnimationPrototypeList;
};