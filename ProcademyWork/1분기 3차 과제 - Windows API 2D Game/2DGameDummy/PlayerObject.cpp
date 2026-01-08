#include "stdafx.h"
#include "BaseObject.h"
#include "PlayerObject.h"
#include "Animation.h"
#include "NetworkMgr.h"

const int default_x_speed = 3;
const int default_y_speed = 2;
constexpr char default_hp = 50;

bool AddAnimation(CAnimation* Animation, int AnimationID, int AnimationDelay, bool isLoop, int iStart, int iEnd);

CPlayerObject::CPlayerObject(int iObjectID, int iObjectType)
	: CBaseObject(iObjectID, iObjectType)
	, m_bPlayerCharacter(false)
	, m_chHP(default_hp)
{
	m_Animation = new CAnimation*[eAni_PLAYER_ANIMATION_MAX - eAni_PLAYER_STAND_L];

	{
		for (int i = 0; i < eAni_PLAYER_ANIMATION_MAX; ++i) {
			m_Animation[i] = new CAnimation();
		}
		AddAnimation(m_Animation[0], eAni_PLAYER_STAND_L, dfDELAY_STAND, true, ePLAYER_STAND_L01, ePLAYER_STAND_L03);
		AddAnimation(m_Animation[1], eAni_PLAYER_STAND_R, dfDELAY_STAND, true, ePLAYER_STAND_R01, ePLAYER_STAND_R03);
		AddAnimation(m_Animation[2], eAni_PLAYER_MOVE_L, dfDELAY_MOVE, true, ePLAYER_MOVE_L01, ePLAYER_MOVE_L12);
		AddAnimation(m_Animation[3], eAni_PLAYER_MOVE_R, dfDELAY_MOVE, true, ePLAYER_MOVE_R01, ePLAYER_MOVE_R12);
		AddAnimation(m_Animation[4], eAni_PLAYER_ATTACK1_L, dfDELAY_ATTACK1, false, ePLAYER_ATTACK1_L01, ePLAYER_ATTACK1_L04);
		AddAnimation(m_Animation[5], eAni_PLAYER_ATTACK1_R, dfDELAY_ATTACK1, false, ePLAYER_ATTACK1_R01, ePLAYER_ATTACK1_R04);
		AddAnimation(m_Animation[6], eAni_PLAYER_ATTACK2_L, dfDELAY_ATTACK2, false, ePLAYER_ATTACK2_L01, ePLAYER_ATTACK2_L04);
		AddAnimation(m_Animation[7], eAni_PLAYER_ATTACK2_R, dfDELAY_ATTACK2, false, ePLAYER_ATTACK2_R01, ePLAYER_ATTACK2_R04);
		AddAnimation(m_Animation[8], eAni_PLAYER_ATTACK3_L, dfDELAY_ATTACK3, false, ePLAYER_ATTACK3_L01, ePLAYER_ATTACK3_L06);
		AddAnimation(m_Animation[9], eAni_PLAYER_ATTACK3_R, dfDELAY_ATTACK3, false, ePLAYER_ATTACK3_R01, ePLAYER_ATTACK3_R06);
	}

	m_iCurAnimation = eAni_PLAYER_STAND_L;
}

// dummy 용 함수
bool AddAnimation(CAnimation* Animation, int AnimationID, int AnimationDelay, bool isLoop, int iStart, int iEnd)
{
	if (Animation->Init(AnimationID, AnimationDelay, isLoop) == false)
	{
		return false;
	}
	Animation->SetSprite(iStart, iEnd);

	return true;
}

CPlayerObject::~CPlayerObject()
{
	for (int i = 0; i < eAni_PLAYER_ANIMATION_MAX; ++i) {
		delete m_Animation[i];
	}

	delete[] m_Animation;
}

void CPlayerObject::Render()
{
	
}

bool CPlayerObject::Run()
{
	// 현재 액션을 실행한다.
	ActionProc();

	m_Animation[m_iCurAnimation]->NextFrame();

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

	if (m_bPlayerCharacter && (m_dwActionCur == dfACTION_ATTACK1 || m_dwActionCur == dfACTION_ATTACK2 || m_dwActionCur == dfACTION_ATTACK3))
	{
		if (!m_Animation[m_iCurAnimation]->IsEndFrame()) {
			return;
		}
		else {
			m_Animation[m_iCurAnimation]->LoopResetFrame();
		}
	}
	else if (m_dwActionCur == dfACTION_ATTACK1 || m_dwActionCur == dfACTION_ATTACK2 || m_dwActionCur == dfACTION_ATTACK3) {
		AddLog(L"공격인데 들어오는 상황 %d %d\n", m_dwActionCur, actionInput);
		m_Animation[m_iCurAnimation]->LoopResetFrame();
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
	char packetBuf[20];
	int packetSize = 0;

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
		packetSize = sizeof(stPACKET_CS_MOVE_START);
		// byActionType 값이 사실은 dfPACKET은 아니기 때문에 조심해야함.
		mpMoveStart((stPACKET_CS_MOVE_START*)packetBuf, byActionType, m_iCurX, m_iCurY);
	}
	break;
	case dfACTION_STAND:
	{
		packetSize = sizeof(stPACKET_CS_MOVE_STOP);
		if (m_bDirCur) {
			mpMoveStop((stPACKET_CS_MOVE_STOP*)packetBuf, dfPACKET_MOVE_DIR_RR, m_iCurX, m_iCurY);
		}
		else {
			mpMoveStop((stPACKET_CS_MOVE_STOP*)packetBuf, dfPACKET_MOVE_DIR_LL, m_iCurX, m_iCurY);
		}
	}
	break;
	case dfACTION_ATTACK1:
	{
		packetSize = sizeof(stPACKET_CS_ATTACK1);
		if (m_bDirCur) {
			mpAttack1((stPACKET_CS_ATTACK1*)packetBuf, dfPACKET_MOVE_DIR_RR, m_iCurX, m_iCurY);
		}
		else {
			mpAttack1((stPACKET_CS_ATTACK1*)packetBuf, dfPACKET_MOVE_DIR_LL, m_iCurX, m_iCurY);
		}
	}
	break;
	case dfACTION_ATTACK2:
	{
		packetSize = sizeof(stPACKET_CS_ATTACK2);
		if (m_bDirCur) {
			mpAttack2((stPACKET_CS_ATTACK2*)packetBuf, dfPACKET_MOVE_DIR_RR, m_iCurX, m_iCurY);
		}
		else {
			mpAttack2((stPACKET_CS_ATTACK2*)packetBuf, dfPACKET_MOVE_DIR_LL, m_iCurX, m_iCurY);
		}
	}
	break;
	case dfACTION_ATTACK3:
	{
		packetSize = sizeof(stPACKET_CS_ATTACK3);
		if (m_bDirCur) {
			mpAttack3((stPACKET_CS_ATTACK3*)packetBuf, dfPACKET_MOVE_DIR_RR, m_iCurX, m_iCurY);
		}
		else {
			mpAttack3((stPACKET_CS_ATTACK3*)packetBuf, dfPACKET_MOVE_DIR_LL, m_iCurX, m_iCurY);
		}
	}
	break;
	}

	if (SINGLETON(CNetworkMgr)->SendPacket(m_iObjectID, packetBuf, packetSize) == false)
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

		// 여기서 바로 종료되지 않기 때문에 중복 호출될 수 있다.
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
		m_Animation[m_iCurAnimation]->SetHitEffect(effectX, effectY);
		break;
	case eAni_PLAYER_ATTACK1_R:
		m_Animation[m_iCurAnimation]->SetHitEffect(effectX, effectY);
		break;
	case eAni_PLAYER_ATTACK2_L:
		m_Animation[m_iCurAnimation]->SetHitEffect(effectX, effectY);
		break;
	case eAni_PLAYER_ATTACK2_R:
		m_Animation[m_iCurAnimation]->SetHitEffect(effectX, effectY);
		break;
	case eAni_PLAYER_ATTACK3_L:
		m_Animation[m_iCurAnimation]->SetHitEffect(effectX, effectY);
		break;
	case eAni_PLAYER_ATTACK3_R:
		m_Animation[m_iCurAnimation]->SetHitEffect(effectX, effectY);
		break;
	default:
		AddLog(L"WrongEffect %d, %d\n", m_iObjectID, m_iCurAnimation);
		break;
	}
}