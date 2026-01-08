#pragma once

class CSprite;
class CAnimation
{
public:
	CAnimation();
	~CAnimation();
	CAnimation(CAnimation& animation);

	void Render(int x, int y, int iWidthPercentage = 100);
	void RenderRed(int x, int y);
	void Render50(int x, int y);

	bool Init(int AnimationID, int FrameDelay, bool isLoop);
	bool CheckID(int AnimationID);

	// 프레임 관련
	void NextFrame();
	bool IsEndFrame();
	void LoopResetFrame();

	// 스프라이트 관련
	bool AddSprite(CSprite* sprite);
	int GetSprite();
	void SetSprite(int iStart, int iEnd);

	// 이펙트 처리
	void SetHitEffect(int effectX, int effectY);
	void ActiveEffect();
private:
	int		m_iAnimationID;
	CQueue<CSprite*> m_SpriteQueue;

	// 프레임 관련
	int		m_iDelayCount;
	int		m_iFrameDelay;
	bool	m_bEndFrame;
	bool	m_bIsLoop;

	// 스프라이트 관련
	int		m_iSpriteStart;
	int		m_iSpriteNow;
	int		m_iSpriteEnd;

	bool	m_hitEffect = false;
	int		m_effectX = 0;
	int		m_effectY = 0;
};