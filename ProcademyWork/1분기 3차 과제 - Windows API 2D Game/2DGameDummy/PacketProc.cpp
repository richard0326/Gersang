#include "stdafx.h"
#include "PacketProc.h"
#include "GameMgr.h"
#include "ObjectMgr.h"
#include "BaseObject.h"
#include "PlayerObject.h"

/**
 * 서버에서 받은 데이터에 대한 컨텐츠 처리를 진행하는 곳.
 */

bool netPacketProc_CreateMyCharacter(char* pPacketBuffer, int* ID)
{
	stPACKET_SC_CREATE_MY_CHARACTER* pPacket = (stPACKET_SC_CREATE_MY_CHARACTER*)pPacketBuffer;

	CPlayerObject* playerObject = (CPlayerObject*)SINGLETON(CObjectMgr)->CreateObject(eTYPE_PLAYER, pPacket->ID);
	if (playerObject == nullptr)
		return false;
	
	*ID = pPacket->ID;
	playerObject->SetPlayerCharacterTrue();
	playerObject->SetPosition(pPacket->X, pPacket->Y);
	playerObject->SetHP(pPacket->HP);
	
	if (pPacket->Direction == dfPACKET_MOVE_DIR_LL) {
		playerObject->SetDirection(false);
	}
	else {
		playerObject->SetDirection(true);
	}
	playerObject->InputActionProc(dfACTION_STAND);
	
	return true;
}

bool netPacketProc_CreateOtherCharacter(char* pPacketBuffer)
{
	// 더미라서 없어도 됨.
	//stPACKET_SC_CREATE_OTHER_CHARACTER* pPacket = (stPACKET_SC_CREATE_OTHER_CHARACTER*)pPacketBuffer;
	//CPlayerObject* playerObject = (CPlayerObject*)SINGLETON(CObjectMgr)->CreateObject(eTYPE_PLAYER, pPacket->ID);
	//if (playerObject == nullptr)
	//	return false;

	//playerObject->SetPosition(pPacket->X, pPacket->Y);
	//playerObject->SetHP(pPacket->HP);

	//if (pPacket->Direction == dfPACKET_MOVE_DIR_LL) {
	//	playerObject->SetDirection(false);
	//}
	//else {
	//	playerObject->SetDirection(true);
	//}
	//playerObject->InputActionProc(dfACTION_STAND);

	return true;
}

bool netPacketProc_DeleteCharacter(char* pPacketBuffer)
{
	stPACKET_SC_DELETE_CHARACTER* pPacket = (stPACKET_SC_DELETE_CHARACTER*)pPacketBuffer;

	if (SINGLETON(CObjectMgr)->EraseByObjectID(pPacket->ID) == false)
		return false;

	return true;
}

bool netPacketProc_MoveStart(char* pPacketBuffer)
{
	stPACKET_SC_MOVE_START* pPacket = (stPACKET_SC_MOVE_START*)pPacketBuffer;

	CPlayerObject* outPlayer;
	if (SINGLETON(CObjectMgr)->FindByObjectID(pPacket->ID, (CBaseObject**)&outPlayer) == false)
		return false;

	switch (pPacket->Direction)
	{
	case dfPACKET_MOVE_DIR_LL:
		outPlayer->InputActionProc(dfACTION_MOVE_LL);
		break;
	case dfPACKET_MOVE_DIR_LU:
		outPlayer->InputActionProc(dfACTION_MOVE_LU);
		break;
	case dfPACKET_MOVE_DIR_UU:
		outPlayer->InputActionProc(dfACTION_MOVE_UU);
		break;
	case dfPACKET_MOVE_DIR_RU:
		outPlayer->InputActionProc(dfACTION_MOVE_RU);
		break;
	case dfPACKET_MOVE_DIR_RR:
		outPlayer->InputActionProc(dfACTION_MOVE_RR);
		break;
	case dfPACKET_MOVE_DIR_RD:
		outPlayer->InputActionProc(dfACTION_MOVE_RD);
		break;
	case dfPACKET_MOVE_DIR_DD:
		outPlayer->InputActionProc(dfACTION_MOVE_DD);
		break;
	case dfPACKET_MOVE_DIR_LD:
		outPlayer->InputActionProc(dfACTION_MOVE_LD);
		break;
	default:
		return false;
	}

	outPlayer->SetPosition(pPacket->X, pPacket->Y);

	return true;
}

bool netPacketProc_MoveStop(char* pPacketBuffer)
{
	stPACKET_SC_MOVE_STOP* pPacket = (stPACKET_SC_MOVE_STOP*)pPacketBuffer;

	CPlayerObject* outPlayer;
	if (SINGLETON(CObjectMgr)->FindByObjectID(pPacket->ID, (CBaseObject**)&outPlayer) == false)
		return false;

	outPlayer->SetPosition(pPacket->X, pPacket->Y);

	if (pPacket->Direction == dfPACKET_MOVE_DIR_LL) {
		outPlayer->SetDirection(false);
	}
	else {
		outPlayer->SetDirection(true);
	}
	outPlayer->InputActionProc(dfACTION_STAND);

	return true;
}

bool netPacketProc_Attack1(char* pPacketBuffer)
{
	stPACKET_SC_ATTACK1* pPacket = (stPACKET_SC_ATTACK1*)pPacketBuffer;

	CPlayerObject* outPlayer;
	if (SINGLETON(CObjectMgr)->FindByObjectID(pPacket->ID, (CBaseObject**)&outPlayer) == false)
		return false;

	outPlayer->SetPosition(pPacket->X, pPacket->Y);

	if (pPacket->Direction == dfPACKET_MOVE_DIR_LL) {
		outPlayer->SetDirection(false);
	}
	else {
		outPlayer->SetDirection(true);
	}
	outPlayer->InputActionProc(dfACTION_ATTACK1);

	return true;
}

bool netPacketProc_Attack2(char* pPacketBuffer)
{
	stPACKET_SC_ATTACK2* pPacket = (stPACKET_SC_ATTACK2*)pPacketBuffer;

	CPlayerObject* outPlayer;
	if (SINGLETON(CObjectMgr)->FindByObjectID(pPacket->ID, (CBaseObject**)&outPlayer) == false)
		return false;
	
	outPlayer->SetPosition(pPacket->X, pPacket->Y);

	if (pPacket->Direction == dfPACKET_MOVE_DIR_LL) {
		outPlayer->SetDirection(false);
	}
	else {
		outPlayer->SetDirection(true);
	}
	outPlayer->InputActionProc(dfACTION_ATTACK2);

	return true;
}

bool netPacketProc_Attack3(char* pPacketBuffer)
{
	stPACKET_SC_ATTACK3* pPacket = (stPACKET_SC_ATTACK3*)pPacketBuffer;

	CPlayerObject* outPlayer;
	if (SINGLETON(CObjectMgr)->FindByObjectID(pPacket->ID, (CBaseObject**)&outPlayer) == false)
		return false;

	outPlayer->SetPosition(pPacket->X, pPacket->Y);

	if (pPacket->Direction == dfPACKET_MOVE_DIR_LL) {
		outPlayer->SetDirection(false);
	}
	else {
		outPlayer->SetDirection(true);
	}
	outPlayer->InputActionProc(dfACTION_ATTACK3);

	return true;
}

bool netPacketProc_Damage(char* pPacketBuffer)
{
	stPACKET_SC_DAMAGE* pPacket = (stPACKET_SC_DAMAGE*)pPacketBuffer;

	CPlayerObject* attackPlayer;
	if (SINGLETON(CObjectMgr)->FindByObjectID(pPacket->AttackID, (CBaseObject**)&attackPlayer) == false)
		return false;

	CPlayerObject* damagePlayer;
	if (SINGLETON(CObjectMgr)->FindByObjectID(pPacket->DamageID, (CBaseObject**)&damagePlayer) == false)
		return false;

	attackPlayer->SetHitEffect(damagePlayer->GetCurX(), damagePlayer->GetCurY() - 47);
	damagePlayer->SetHP(pPacket->DamageHP);

	return true;
}