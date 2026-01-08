#include "stdafx.h"
#include "BaseObject.h"
#include "EffectObject.h"
#include "ResourceMgr.h"
#include "Animation.h"

CEffectObject::CEffectObject()
{
	m_Animation = new CAnimation* ();
	SINGLETON(CResourceMgr)->GetAnimation(eNum_EFFECT_SPACK, m_Animation);
}

CEffectObject::~CEffectObject()
{
	delete m_Animation;
}

void CEffectObject::Render()
{
	if (m_bDelObject == false ) {
		(*m_Animation)->RenderRed(m_iCurX, m_iCurY);
	}
}

bool CEffectObject::Run()
{
	(*m_Animation)->NextFrame();
	if((*m_Animation)->IsEndFrame() == true)
	{
		m_bDelObject = true;
	}

	return true;
}