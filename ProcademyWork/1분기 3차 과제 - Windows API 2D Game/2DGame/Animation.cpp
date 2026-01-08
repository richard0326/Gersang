#include "stdafx.h"
#include "Animation.h"
#include "Sprite.h"
#include "ResourceMgr.h"
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
	m_SpriteQueue.resize(20);
}

CAnimation::~CAnimation()
{
}

CAnimation::CAnimation(CAnimation& animation)
{
	// 큐 내부의 동적 생산된 부분이 겹쳐서 새로 resize를 해줘야함.
	m_SpriteQueue.resize(20);
	for(int i = 0; i < animation.m_SpriteQueue.size(); i++)
	{
		CSprite* out;
		animation.m_SpriteQueue.peek(i, &out);
		m_SpriteQueue.push(out);
	}

	m_iAnimationID = animation.m_iAnimationID;
	m_bIsLoop = animation.m_bIsLoop;
	m_iFrameDelay = animation.m_iFrameDelay;
	m_bEndFrame = false;
	m_iDelayCount = 0;

	m_iSpriteNow = m_iSpriteStart = animation.m_iSpriteStart;
	m_iSpriteEnd = animation.m_iSpriteEnd;
}

void CAnimation::Render(int x, int y, int iWidthPercentage)
{
	CSprite* out;
	if (m_SpriteQueue.peek(m_iSpriteNow - m_iSpriteStart, &out))
	{
		out->Render(x, y, iWidthPercentage);
	}
}

void CAnimation::RenderRed(int x, int y)
{
	CSprite* out;
	if (m_SpriteQueue.peek(m_iSpriteNow - m_iSpriteStart, &out))
	{
		out->RenderRed(x, y);
	}
}

void CAnimation::Render50(int x, int y)
{
	CSprite* out;
	if (m_SpriteQueue.peek(m_iSpriteNow - m_iSpriteStart, &out))
	{
		out->Render50(x, y);
	}
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
bool CAnimation::AddSprite(CSprite* sprite)
{
	m_SpriteQueue.push(sprite);

	return true;
}

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

	switch (m_iAnimationID)
	{
	case eAni_PLAYER_ATTACK1_L:
		if (m_iSpriteNow == ePLAYER_ATTACK1_L01 && m_iDelayCount == m_iFrameDelay)
		{
			CBaseObject* baseObj = SINGLETON(CObjectMgr)->CreateObject(eTYPE_EFFECT);
			baseObj->SetPosition(m_effectX, m_effectY);
			m_hitEffect = false;
		}
		break;
	case eAni_PLAYER_ATTACK1_R:
		if (m_iSpriteNow == ePLAYER_ATTACK1_R01 && m_iDelayCount == m_iFrameDelay)
		{
			CBaseObject* baseObj = SINGLETON(CObjectMgr)->CreateObject(eTYPE_EFFECT);
			baseObj->SetPosition(m_effectX, m_effectY);
			m_hitEffect = false;
		}
		break;
	case eAni_PLAYER_ATTACK2_L:
		if (m_iSpriteNow == ePLAYER_ATTACK2_L01 && m_iDelayCount == m_iFrameDelay)
		{
			CBaseObject* baseObj = SINGLETON(CObjectMgr)->CreateObject(eTYPE_EFFECT);
			baseObj->SetPosition(m_effectX, m_effectY);
			m_hitEffect = false;
		}
		break;
	case eAni_PLAYER_ATTACK2_R:
		if (m_iSpriteNow == ePLAYER_ATTACK2_R01 && m_iDelayCount == m_iFrameDelay)
		{
			CBaseObject* baseObj = SINGLETON(CObjectMgr)->CreateObject(eTYPE_EFFECT);
			baseObj->SetPosition(m_effectX, m_effectY);
			m_hitEffect = false;
		}
		break;
	case eAni_PLAYER_ATTACK3_L:
		if (m_iSpriteNow == ePLAYER_ATTACK3_L02 && m_iDelayCount == m_iFrameDelay)
		{
			CBaseObject* baseObj = SINGLETON(CObjectMgr)->CreateObject(eTYPE_EFFECT);
			baseObj->SetPosition(m_effectX, m_effectY);
			m_hitEffect = false;
		}
		break;
	case eAni_PLAYER_ATTACK3_R:
		if (m_iSpriteNow == ePLAYER_ATTACK3_R02 && m_iDelayCount == m_iFrameDelay)
		{
			CBaseObject* baseObj = SINGLETON(CObjectMgr)->CreateObject(eTYPE_EFFECT);
			baseObj->SetPosition(m_effectX, m_effectY);
			m_hitEffect = false;
		}
		break;
	}
}