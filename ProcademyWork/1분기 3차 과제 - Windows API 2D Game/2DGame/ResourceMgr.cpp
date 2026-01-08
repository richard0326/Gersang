#include "stdafx.h"
#include "ResourceMgr.h"
#include "Sprite.h"
#include "Animation.h"

DECLARE_SINGLETON_IN_CPP(CResourceMgr);

CResourceMgr::CResourceMgr()
{

}

CResourceMgr::~CResourceMgr()
{

}

bool CResourceMgr::Init()
{
	if (AddSprite(eMap, L"Sprite_Data/_Map.bmp", 0, 0) == false)
	{
		return false;
	}

	if (AddSprite(eSHADOW, L"Sprite_Data/Shadow.bmp", 32, 4) == false)
	{
		return false;
	}

	if (AddSprite(eGUAGE_HP, L"Sprite_Data/HPGuage.bmp", 0, 0) == false)
	{
		return false;
	}

	if (AddSprite(eTILEMAP, L"Sprite_Data/Tile_01.bmp", 0, 0) == false)
	{
		return false;
	}

	if (AddAnimation(eAni_PLAYER_STAND_L, dfDELAY_STAND, true, ePLAYER_STAND_L01, ePLAYER_STAND_L03))
	{
		AddSpriteInAnimation(eAni_PLAYER_STAND_L, ePLAYER_STAND_L01, L"Sprite_Data/Stand_L_01.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_STAND_L, ePLAYER_STAND_L02, L"Sprite_Data/Stand_L_02.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_STAND_L, ePLAYER_STAND_L03, L"Sprite_Data/Stand_L_03.bmp", 71, 90);
	}

	if (AddAnimation(eAni_PLAYER_STAND_R, dfDELAY_STAND, true, ePLAYER_STAND_R01, ePLAYER_STAND_R03))
	{
		AddSpriteInAnimation(eAni_PLAYER_STAND_R, ePLAYER_STAND_R01, L"Sprite_Data/Stand_R_01.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_STAND_R, ePLAYER_STAND_R02, L"Sprite_Data/Stand_R_02.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_STAND_R, ePLAYER_STAND_R03, L"Sprite_Data/Stand_R_03.bmp", 71, 90);
	}

	if (AddAnimation(eAni_PLAYER_MOVE_L, dfDELAY_MOVE, true, ePLAYER_MOVE_L01, ePLAYER_MOVE_L12))
	{
		AddSpriteInAnimation(eAni_PLAYER_MOVE_L, ePLAYER_MOVE_L01, L"Sprite_Data/Move_L_01.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_MOVE_L, ePLAYER_MOVE_L02, L"Sprite_Data/Move_L_02.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_MOVE_L, ePLAYER_MOVE_L03, L"Sprite_Data/Move_L_03.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_MOVE_L, ePLAYER_MOVE_L04, L"Sprite_Data/Move_L_04.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_MOVE_L, ePLAYER_MOVE_L05, L"Sprite_Data/Move_L_05.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_MOVE_L, ePLAYER_MOVE_L06, L"Sprite_Data/Move_L_06.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_MOVE_L, ePLAYER_MOVE_L07, L"Sprite_Data/Move_L_07.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_MOVE_L, ePLAYER_MOVE_L08, L"Sprite_Data/Move_L_08.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_MOVE_L, ePLAYER_MOVE_L09, L"Sprite_Data/Move_L_09.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_MOVE_L, ePLAYER_MOVE_L10, L"Sprite_Data/Move_L_10.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_MOVE_L, ePLAYER_MOVE_L11, L"Sprite_Data/Move_L_11.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_MOVE_L, ePLAYER_MOVE_L12, L"Sprite_Data/Move_L_12.bmp", 71, 90);
	}

	if (AddAnimation(eAni_PLAYER_MOVE_R, dfDELAY_MOVE, true, ePLAYER_MOVE_R01, ePLAYER_MOVE_R12))
	{
		AddSpriteInAnimation(eAni_PLAYER_MOVE_R, ePLAYER_MOVE_R01, L"Sprite_Data/Move_R_01.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_MOVE_R, ePLAYER_MOVE_R02, L"Sprite_Data/Move_R_02.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_MOVE_R, ePLAYER_MOVE_R03, L"Sprite_Data/Move_R_03.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_MOVE_R, ePLAYER_MOVE_R04, L"Sprite_Data/Move_R_04.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_MOVE_R, ePLAYER_MOVE_R05, L"Sprite_Data/Move_R_05.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_MOVE_R, ePLAYER_MOVE_R06, L"Sprite_Data/Move_R_06.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_MOVE_R, ePLAYER_MOVE_R07, L"Sprite_Data/Move_R_07.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_MOVE_R, ePLAYER_MOVE_R08, L"Sprite_Data/Move_R_08.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_MOVE_R, ePLAYER_MOVE_R09, L"Sprite_Data/Move_R_09.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_MOVE_R, ePLAYER_MOVE_R10, L"Sprite_Data/Move_R_10.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_MOVE_R, ePLAYER_MOVE_R11, L"Sprite_Data/Move_R_11.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_MOVE_R, ePLAYER_MOVE_R12, L"Sprite_Data/Move_R_12.bmp", 71, 90);
	}

	if (AddAnimation(eAni_PLAYER_ATTACK1_L, dfDELAY_ATTACK1, false, ePLAYER_ATTACK1_L01, ePLAYER_ATTACK1_L04))
	{
		AddSpriteInAnimation(eAni_PLAYER_ATTACK1_L, ePLAYER_ATTACK1_L01, L"Sprite_Data/Attack1_L_01.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_ATTACK1_L, ePLAYER_ATTACK1_L02, L"Sprite_Data/Attack1_L_02.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_ATTACK1_L, ePLAYER_ATTACK1_L03, L"Sprite_Data/Attack1_L_03.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_ATTACK1_L, ePLAYER_ATTACK1_L04, L"Sprite_Data/Attack1_L_04.bmp", 71, 90);
	}

	if (AddAnimation(eAni_PLAYER_ATTACK1_R, dfDELAY_ATTACK1, false, ePLAYER_ATTACK1_R01, ePLAYER_ATTACK1_R04))
	{
		AddSpriteInAnimation(eAni_PLAYER_ATTACK1_R, ePLAYER_ATTACK1_R01, L"Sprite_Data/Attack1_R_01.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_ATTACK1_R, ePLAYER_ATTACK1_R02, L"Sprite_Data/Attack1_R_02.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_ATTACK1_R, ePLAYER_ATTACK1_R03, L"Sprite_Data/Attack1_R_03.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_ATTACK1_R, ePLAYER_ATTACK1_R04, L"Sprite_Data/Attack1_R_04.bmp", 71, 90);
	}

	if (AddAnimation(eAni_PLAYER_ATTACK2_L, dfDELAY_ATTACK2, false, ePLAYER_ATTACK2_L01, ePLAYER_ATTACK2_L04))
	{
		AddSpriteInAnimation(eAni_PLAYER_ATTACK2_L, ePLAYER_ATTACK2_L01, L"Sprite_Data/Attack2_L_01.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_ATTACK2_L, ePLAYER_ATTACK2_L02, L"Sprite_Data/Attack2_L_02.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_ATTACK2_L, ePLAYER_ATTACK2_L03, L"Sprite_Data/Attack2_L_03.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_ATTACK2_L, ePLAYER_ATTACK2_L04, L"Sprite_Data/Attack2_L_04.bmp", 71, 90);
	}

	if (AddAnimation(eAni_PLAYER_ATTACK2_R, dfDELAY_ATTACK2, false, ePLAYER_ATTACK2_R01, ePLAYER_ATTACK2_R04))
	{
		AddSpriteInAnimation(eAni_PLAYER_ATTACK2_R, ePLAYER_ATTACK2_R01, L"Sprite_Data/Attack2_R_01.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_ATTACK2_R, ePLAYER_ATTACK2_R02, L"Sprite_Data/Attack2_R_02.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_ATTACK2_R, ePLAYER_ATTACK2_R03, L"Sprite_Data/Attack2_R_03.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_ATTACK2_R, ePLAYER_ATTACK2_R04, L"Sprite_Data/Attack2_R_04.bmp", 71, 90);
	}

	if (AddAnimation(eAni_PLAYER_ATTACK3_L, dfDELAY_ATTACK3, false, ePLAYER_ATTACK3_L01, ePLAYER_ATTACK3_L06))
	{
		AddSpriteInAnimation(eAni_PLAYER_ATTACK3_L, ePLAYER_ATTACK3_L01, L"Sprite_Data/Attack3_L_01.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_ATTACK3_L, ePLAYER_ATTACK3_L02, L"Sprite_Data/Attack3_L_02.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_ATTACK3_L, ePLAYER_ATTACK3_L03, L"Sprite_Data/Attack3_L_03.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_ATTACK3_L, ePLAYER_ATTACK3_L04, L"Sprite_Data/Attack3_L_04.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_ATTACK3_L, ePLAYER_ATTACK3_L05, L"Sprite_Data/Attack3_L_05.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_ATTACK3_L, ePLAYER_ATTACK3_L06, L"Sprite_Data/Attack3_L_06.bmp", 71, 90);
	}

	if (AddAnimation(eAni_PLAYER_ATTACK3_R, dfDELAY_ATTACK3, false, ePLAYER_ATTACK3_R01, ePLAYER_ATTACK3_R06))
	{
		AddSpriteInAnimation(eAni_PLAYER_ATTACK3_R, ePLAYER_ATTACK3_R01, L"Sprite_Data/Attack3_R_01.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_ATTACK3_R, ePLAYER_ATTACK3_R02, L"Sprite_Data/Attack3_R_02.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_ATTACK3_R, ePLAYER_ATTACK3_R03, L"Sprite_Data/Attack3_R_03.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_ATTACK3_R, ePLAYER_ATTACK3_R04, L"Sprite_Data/Attack3_R_04.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_ATTACK3_R, ePLAYER_ATTACK3_R05, L"Sprite_Data/Attack3_R_05.bmp", 71, 90);
		AddSpriteInAnimation(eAni_PLAYER_ATTACK3_R, ePLAYER_ATTACK3_R06, L"Sprite_Data/Attack3_R_06.bmp", 71, 90);
	}

	if (AddAnimation(eNum_EFFECT_SPACK, dfDELAY_EFFECT, false, eEFFECT_SPACK_01, eEFFECT_SPACK_04))
	{
		AddSpriteInAnimation(eNum_EFFECT_SPACK, eEFFECT_SPACK_01, L"Sprite_Data/xSpark_1.bmp", 70, 70);
		AddSpriteInAnimation(eNum_EFFECT_SPACK, eEFFECT_SPACK_02, L"Sprite_Data/xSpark_2.bmp", 70, 70);
		AddSpriteInAnimation(eNum_EFFECT_SPACK, eEFFECT_SPACK_03, L"Sprite_Data/xSpark_3.bmp", 70, 70);
		AddSpriteInAnimation(eNum_EFFECT_SPACK, eEFFECT_SPACK_04, L"Sprite_Data/xSpark_4.bmp", 70, 70);
	}

	return true;
}

void CResourceMgr::Release()
{
	for (auto spriteiter = m_SpriteList.begin(); spriteiter != m_SpriteList.end(); )
	{
		delete (*spriteiter);
		spriteiter = m_SpriteList.erase(spriteiter);
	}

	// 배포용 프로토타입 - 애니메이션 삭제 (프로토타입 패턴)
	for (auto animationiter = m_AnimationList.begin(); animationiter != m_AnimationList.end();)
	{
		delete (*animationiter);
		animationiter = m_AnimationList.erase(animationiter);
	}

	// 복사된 애니메이션 삭제
	for (auto animationCopyiter = m_AnimationPrototypeList.begin(); animationCopyiter != m_AnimationPrototypeList.end();)
	{
		delete (*animationCopyiter);
		animationCopyiter = m_AnimationPrototypeList.erase(animationCopyiter);
	}
}

bool CResourceMgr::GetSprite(int SpriteID, CSprite** out)
{
	for (auto iter = m_SpriteList.begin(); iter != m_SpriteList.end(); ++iter)
	{
		if ((*iter)->CheckID(SpriteID))
		{
			*out = *iter;
			return true;
		}
	}

	return false;
}

bool CResourceMgr::GetAnimation(int AnimationID, CAnimation** out)
{
	for (auto iter = m_AnimationList.begin(); iter != m_AnimationList.end(); ++iter)
	{
		if ((*iter)->CheckID(AnimationID))
		{
			// 애니메이션은 프로토타입의 형태로 복사
			CAnimation* animationCopy = new CAnimation(*(*iter));
			m_AnimationPrototypeList.push_back(animationCopy);
			*out = animationCopy;
			return true;
		}
	}

	return false;
}

bool CResourceMgr::AddSprite(int SpriteID, const wchar_t* fileName, int iCenterX, int iCenterY, CSprite** out)
{
	CSprite* Sprite = new CSprite();
	if (Sprite->ReadSpriteFile(SpriteID, fileName, iCenterX, iCenterY) == false)
	{
		delete Sprite;
		return false;
	}

	m_SpriteList.push_back(Sprite);
	if (out != nullptr)
	{
		*out = Sprite;
	}

	return true;
}

bool CResourceMgr::AddAnimation(int AnimationID, int AnimationDelay, bool isLoop, int iStart, int iEnd)
{
	CAnimation* Animation = new CAnimation();
	if (Animation->Init(AnimationID, AnimationDelay, isLoop) == false)
	{
		return false;
	}
	Animation->SetSprite(iStart, iEnd);

	m_AnimationList.push_back(Animation);
	return true;
}

bool CResourceMgr::AddSpriteInAnimation(int AnimationID, int SpriteID, const wchar_t* fileName, int iCenterX, int iCenterY)
{
	for (auto animationIter = m_AnimationList.begin(); animationIter != m_AnimationList.end(); ++animationIter)
	{
		if ((*animationIter)->CheckID(AnimationID)) {
			CSprite* out = nullptr;
			if (AddSprite(SpriteID, fileName, iCenterX, iCenterY, &out) == false)
				return false;

			(*animationIter)->AddSprite(out);
		}
	}

	return true;
}