#include "stdafx.h"
#include "Animation.h"
#include "ObjectMgr.h"
#include "BaseObject.h"

CAnimation::CAnimation()
	: m_iDelayCount(0)
	, m_iFrameDelay(0)
	, m_bEndFrame(false)
	, m_iSpriteStart(0)
	, m_iSpriteNow(0)
	, m_iSpriteEnd(0)
	, m_iAnimationID(0)
	, m_bIsLoop(false)
{
}

CAnimation::~CAnimation()
{
}

CAnimation::CAnimation(CAnimation& animation)
{
	m_iAnimationID = animation.m_iAnimationID;
	m_bIsLoop = animation.m_bIsLoop;
	m_iFrameDelay = animation.m_iFrameDelay;
	m_bEndFrame = false;
	m_iDelayCount = 0;
	m_hitEffect = false;

	m_iSpriteNow = m_iSpriteStart = animation.m_iSpriteStart;
	m_iSpriteEnd = animation.m_iSpriteEnd;
}

void CAnimation::Render(int x, int y, int iWidthPercentage)
{
}

void CAnimation::RenderRed(int x, int y)
{
}

void CAnimation::Render50(int x, int y)
{
}

bool CAnimation::Init(int AnimationID, int FrameDelay, bool isLoop)
{
	m_iAnimationID = AnimationID;
	m_iFrameDelay = FrameDelay;
	m_bIsLoop = isLoop;
	m_hitEffect = false;

	return true;
}

bool CAnimation::CheckID(int AnimationID)
{
	if (m_iAnimationID == AnimationID)
	{
		return true;
	}
	return false;
}

// 프레임 관련
void CAnimation::NextFrame() {
	if (!m_bEndFrame)
	{
		++m_iDelayCount;
		if (m_iFrameDelay == m_iDelayCount)
		{
			ActiveEffect();
			if (m_iSpriteNow == m_iSpriteEnd)
			{
				m_iSpriteNow = m_iSpriteStart;
				m_bEndFrame = !m_bIsLoop;
			}
			else
			{
				++m_iSpriteNow;
			}
			m_iDelayCount = 0;
		}
	}
}

bool CAnimation::IsEndFrame() 
{
	return m_bEndFrame;
}

void CAnimation::LoopResetFrame()
{
	m_iSpriteNow = m_iSpriteStart;
	m_iDelayCount = 0;
	m_bEndFrame = m_bIsLoop;
	m_hitEffect = false;
}

// 스프라이트 관련
int CAnimation::GetSprite() 
{ 
	return m_iSpriteNow; 
}

void CAnimation::SetSprite(int iStart, int iEnd)
{
	m_iSpriteNow = m_iSpriteStart = iStart;
	m_iSpriteEnd = iEnd;
}

void CAnimation::SetHitEffect(int effectX, int effectY)
{
	m_effectX = effectX;
	m_effectY = effectY;
	m_hitEffect = true;
}

void CAnimation::ActiveEffect()
{
	if (m_hitEffect == false)
		return;

	m_hitEffect = false;
}