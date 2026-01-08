#include "stdafx.h"
#include "BaseObject.h"
#include "PlayerObject.h"
#include "Animation.h"
#include "Sprite.h"
#include "ResourceMgr.h"
#include "NetworkMgr.h"
#include "Lib/SerializeBuffer.h"

const int default_x_speed = 3;
const int default_y_speed = 2;
constexpr char default_hp = 50;

CPlayerObject::CPlayerObject()
	: m_bPlayerCharacter(false)
	, m_chHP(default_hp)
{
	m_Animation = new CAnimation*[eAni_PLAYER_ANIMATION_MAX - eAni_PLAYER_STAND_L];
	
	for (int i = eAni_PLAYER_STAND_L; i < eAni_PLAYER_ANIMATION_MAX; i++)
	{
		SINGLETON(CResourceMgr)->GetAnimation(i, &m_Animation[i]);
	}

	SINGLETON(CResourceMgr)->GetSprite(eGUAGE_HP, &m_sprite_hp);
	SINGLETON(CResourceMgr)->GetSprite(eSHADOW, &m_sprite_shadow);

	m_iCurAnimation = eAni_PLAYER_STAND_L;
}

CPlayerObject::~CPlayerObject()
{
	delete[] m_Animation;
}

void CPlayerObject::Render()
{
	if (m_bDelObject == false) {
		m_sprite_shadow->Render50(m_iCurX, m_iCurY);
		if (m_bPlayerCharacter)
		{
			m_Animation[m_iCurAnimation]->RenderRed(m_iCurX, m_iCurY);
		}
		else
		{
			m_Animation[m_iCurAnimation]->Render(m_iCurX, m_iCurY);
		}

		m_sprite_hp->Render(m_iCurX - 35, m_iCurY + 9, m_chHP);
	}
}

bool CPlayerObject::Run()
{
	m_Animation[m_iCurAnimation]->NextFrame();

	// 현재 액션을 실행한다.
	ActionProc();

	return true;
}

bool CPlayerObject::IsPlayer()
{
	return m_bPlayerCharacter;
}

void CPlayerObject::SetPlayerCharacterTrue()
{
	m_bPlayerCharacter = true;
}

// Run() 함수 내부에서 호출해주는 실제 객체 처리 부
void CPlayerObject::ActionProc()
{
	//------------------------------------------------------------
	// 몇몇 동작시(공격)의 경우 강제적으로 해당 동작 처리를 완료해아만 한다.
	//------------------------------------------------------------
	switch (m_dwActionCur)
	{
		//------------------------------------------------------------
		// 공격동작 또는 데미지 동작은 에니메이션이 끝날 때까지 강제적으로 에니메이션이 되어야만 하며,
		// 에니메이션이 끝난 후 기본동작으로 자동으로 돌아가야 한다.
		//------------------------------------------------------------
	case dfACTION_ATTACK1:
	case dfACTION_ATTACK2:
	case dfACTION_ATTACK3:
		if (m_Animation[m_iCurAnimation]->IsEndFrame())
		{
			m_Animation[m_iCurAnimation]->LoopResetFrame();
			//------------------------------------------------------------
			// 공격이 끝났었더라면, 액션을 바꿔줘서 연속으로 공격을 할 때 재 전송이 가능하도록 한다.
			//------------------------------------------------------------
			SetActionStand();
		}
		break;
	case dfACTION_MOVE_LL:
		m_bDirCur = false;
		m_iCurX -= default_x_speed;
		if (m_iCurX <= dfRANGE_MOVE_LEFT)
		{
			m_iCurX = dfRANGE_MOVE_LEFT + 1;
		}
		break;
	case dfACTION_MOVE_LU:
		m_bDirCur = false;
		if (m_iCurX - default_x_speed <= dfRANGE_MOVE_LEFT || m_iCurY - default_y_speed <= dfRANGE_MOVE_TOP)
			break;

		m_iCurX -= default_x_speed;
		m_iCurY -= default_y_speed;
		break;
	case dfACTION_MOVE_UU:
		m_iCurY -= default_y_speed;
		if (m_iCurY <= dfRANGE_MOVE_TOP)
		{
			m_iCurY = dfRANGE_MOVE_TOP + 1;
		}
		break;
	case dfACTION_MOVE_RU:
		m_bDirCur = true;
		if (m_iCurX + default_x_speed >= dfRANGE_MOVE_RIGHT || m_iCurY - default_y_speed <= dfRANGE_MOVE_TOP)
			break;

		m_iCurX += default_x_speed;
		m_iCurY -= default_y_speed;
		break;
	case dfACTION_MOVE_RR:
		m_bDirCur = true;
		m_iCurX += default_x_speed;
		if (m_iCurX >= dfRANGE_MOVE_RIGHT)
		{
			m_iCurX = dfRANGE_MOVE_RIGHT + 1;
		}
		break;
	case dfACTION_MOVE_RD:
		m_bDirCur = true;
		if (m_iCurX + default_x_speed >= dfRANGE_MOVE_RIGHT || m_iCurY + default_y_speed >= dfRANGE_MOVE_BOTTOM)
			break;

		m_iCurX += default_x_speed;
		m_iCurY += default_y_speed;
		break;
	case dfACTION_MOVE_DD:
		m_iCurY += default_y_speed;
		if (m_iCurY >= dfRANGE_MOVE_BOTTOM)
		{
			m_iCurY = dfRANGE_MOVE_BOTTOM - 1;
		}
		break;
	case dfACTION_MOVE_LD:
		m_bDirCur = false;
		if (m_iCurX - default_x_speed <= dfRANGE_MOVE_LEFT || m_iCurY + default_y_speed >= dfRANGE_MOVE_BOTTOM)
			break;

		m_iCurX -= default_x_speed;
		m_iCurY += default_y_speed;
		break;
	}

	if (m_iCurX >= dfRANGE_MOVE_RIGHT) {
		m_iCurX = dfRANGE_MOVE_RIGHT - 1;
	}
	if (m_iCurX < 0) {
		m_iCurX = 0;
	}
	if (m_iCurY >= dfRANGE_MOVE_BOTTOM) {
		m_iCurY = dfRANGE_MOVE_BOTTOM - 1;
	}
	if (m_iCurY < 0) {
		m_iCurY = 0;
	}

	//------------------------------------------------------------
	// 애니메이션 설정
	//------------------------------------------------------------
	SetAnimation(m_dwActionCur, m_bDirCur);
}

// ActionInput을 통해서 들어온 액션 값을 실제로 처리하는 부분. 
// ActionProc 내부에서 호출
void CPlayerObject::InputActionProc(DWORD actionInput)
{
	if (actionInput == m_dwActionCur || actionInput == -1)
	{
		return;
	}

	if (m_bPlayerCharacter) {
		if (m_dwActionCur == dfACTION_ATTACK1 || m_dwActionCur == dfACTION_ATTACK2 || m_dwActionCur == dfACTION_ATTACK3)
		{
			if (!m_Animation[m_iCurAnimation]->IsEndFrame()) {
				return;
			}
			m_Animation[m_iCurAnimation]->LoopResetFrame();
		}
	}
	else {
		if (actionInput == dfACTION_ATTACK1 || actionInput == dfACTION_ATTACK2 || actionInput == dfACTION_ATTACK3)
		{
			//m_Animation[m_iCurAnimation]->LoopResetFrame();
		}
	}

	switch (actionInput)
	{
	case dfACTION_MOVE_LL:
	case dfACTION_MOVE_LU:
	case dfACTION_MOVE_UU:
	case dfACTION_MOVE_RU:
	case dfACTION_MOVE_RR:
	case dfACTION_MOVE_RD:
	case dfACTION_MOVE_DD:
	case dfACTION_MOVE_LD:
		SetActionMove(actionInput);
		break;
	case dfACTION_ATTACK1:
		SetActionAttack1();
		break;
	case dfACTION_ATTACK2:
		SetActionAttack2();
		break;
	case dfACTION_ATTACK3:
		SetActionAttack3();
		break;
	case dfACTION_STAND:
		SetActionStand();
		break;
	}

	SetAnimation(m_dwActionCur, m_bDirCur);
	if (m_bPlayerCharacter) {
		
		SendAction(actionInput);
	}
	
}

bool CPlayerObject::SendAction(DWORD byActionType)
{
	CSerializeBuffer sbuf;

	switch (byActionType)
	{
	case dfACTION_MOVE_LL:
	case dfACTION_MOVE_LU:
	case dfACTION_MOVE_UU:
	case dfACTION_MOVE_RU:
	case dfACTION_MOVE_RR:
	case dfACTION_MOVE_RD:
	case dfACTION_MOVE_DD:
	case dfACTION_MOVE_LD:
	{
		mpMoveStart(&sbuf, byActionType, m_iCurX, m_iCurY);
	}
	break;
	case dfACTION_STAND:
	{
		if (m_bDirCur) {
			mpMoveStop(&sbuf, dfPACKET_MOVE_DIR_RR, m_iCurX, m_iCurY);
		}
		else {
			mpMoveStop(&sbuf, dfPACKET_MOVE_DIR_LL, m_iCurX, m_iCurY);
		}
	}
	break;
	case dfACTION_ATTACK1:
	{
		if (m_bDirCur) {
			mpAttack1(&sbuf, dfPACKET_MOVE_DIR_RR, m_iCurX, m_iCurY);
		}
		else {
			mpAttack1(&sbuf, dfPACKET_MOVE_DIR_LL, m_iCurX, m_iCurY);
		}
	}
	break;
	case dfACTION_ATTACK2:
	{
		if (m_bDirCur) {
			mpAttack2(&sbuf, dfPACKET_MOVE_DIR_RR, m_iCurX, m_iCurY);
		}
		else {
			mpAttack2(&sbuf, dfPACKET_MOVE_DIR_LL, m_iCurX, m_iCurY);
		}
	}
	break;
	case dfACTION_ATTACK3:
	{
		if (m_bDirCur) {
			mpAttack3(&sbuf, dfPACKET_MOVE_DIR_RR, m_iCurX, m_iCurY);
		}
		else {
			mpAttack3(&sbuf, dfPACKET_MOVE_DIR_LL, m_iCurX, m_iCurY);
		}
	}
	break;
	}

	if (SINGLETON(CNetworkMgr)->SendPacket(sbuf.GetBufferPtr(), sbuf.GetDataSize()) == false)
		return false;

	return true;
}

bool CPlayerObject::GetDirection()
{
	return m_bDirCur;
}

void CPlayerObject::SetDirection(bool bRight)
{
	m_bDirCur = bRight;
}

char CPlayerObject::GetHP()
{
	return m_chHP;
}

void CPlayerObject::SetHP(char chHP)
{
	m_chHP = chHP;
	if (m_chHP <= 0) {
		m_bDelObject = true;
	}
}

// 애니메이션 변경 함수들
void CPlayerObject::SetActionAttack1()
{
	m_dwActionOld = m_dwActionCur;
	m_dwActionCur = dfACTION_ATTACK1;
}

void CPlayerObject::SetActionAttack2()
{
	m_dwActionOld = m_dwActionCur;
	m_dwActionCur = dfACTION_ATTACK2;
}

void CPlayerObject::SetActionAttack3()
{
	m_dwActionOld = m_dwActionCur;
	m_dwActionCur = dfACTION_ATTACK3;
}

void CPlayerObject::SetActionMove(DWORD move)
{
	m_dwActionOld = m_dwActionCur;
	m_dwActionCur = move;
}

void CPlayerObject::SetActionStand()
{
	m_dwActionOld = m_dwActionCur;
	m_dwActionCur = dfACTION_STAND;
}

void CPlayerObject::SetAnimation(int ActionID, bool bRight)
{
	switch (ActionID)
	{
	case dfACTION_MOVE_LL:
	case dfACTION_MOVE_LU:
	case dfACTION_MOVE_UU:
	case dfACTION_MOVE_RU:
	case dfACTION_MOVE_RR:
	case dfACTION_MOVE_RD:
	case dfACTION_MOVE_DD:
	case dfACTION_MOVE_LD:
		if (bRight)
		{
			m_iCurAnimation = eAni_PLAYER_MOVE_R;
		}
		else {
			m_iCurAnimation = eAni_PLAYER_MOVE_L;
		}
		break;
	case dfACTION_ATTACK1:
		if (bRight)
		{
			m_iCurAnimation = eAni_PLAYER_ATTACK1_R;
		}
		else {
			m_iCurAnimation = eAni_PLAYER_ATTACK1_L;
		}
		break;
	case dfACTION_ATTACK2:
		if (bRight)
		{
			m_iCurAnimation = eAni_PLAYER_ATTACK2_R;
		}
		else {
			m_iCurAnimation = eAni_PLAYER_ATTACK2_L;
		}
		break;
	case dfACTION_ATTACK3:
		if (bRight)
		{
			m_iCurAnimation = eAni_PLAYER_ATTACK3_R;
		}
		else {
			m_iCurAnimation = eAni_PLAYER_ATTACK3_L;
		}
		break;
	case dfACTION_STAND:
		if (bRight)
		{
			m_iCurAnimation = eAni_PLAYER_STAND_R;
		}
		else {
			m_iCurAnimation = eAni_PLAYER_STAND_L;
		}
		break;
	}
}

void CPlayerObject::SetHitEffect(int effectX, int effectY)
{
	switch (m_iCurAnimation)
	{
	case eAni_PLAYER_ATTACK1_L:
	case eAni_PLAYER_ATTACK1_R:
	case eAni_PLAYER_ATTACK2_L:
	case eAni_PLAYER_ATTACK2_R:
	case eAni_PLAYER_ATTACK3_L:
	case eAni_PLAYER_ATTACK3_R:
		m_Animation[m_iCurAnimation]->SetHitEffect(effectX, effectY);
		break;
	default:
		AddLog(L"WrongEffect %d, %d\n", m_iObjectID, m_iCurAnimation);
		break;
	}
}

void CPlayerObject::ResetAnimation()
{
	switch (m_iCurAnimation)
	{
	case eAni_PLAYER_ATTACK1_L:
	case eAni_PLAYER_ATTACK1_R:
	case eAni_PLAYER_ATTACK2_L:
	case eAni_PLAYER_ATTACK2_R:
	case eAni_PLAYER_ATTACK3_L:
	case eAni_PLAYER_ATTACK3_R:
		m_Animation[m_iCurAnimation]->LoopResetFrame();
		break;
	default:
		AddLog(L"Wrong Reset %d, %d\n", m_iObjectID, m_iCurAnimation);
		break;
	}
}