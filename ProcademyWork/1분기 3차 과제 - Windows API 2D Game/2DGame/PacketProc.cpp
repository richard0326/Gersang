#include "stdafx.h"
#include "PacketProc.h"
#include "GameMgr.h"
#include "ObjectMgr.h"
#include "BaseObject.h"
#include "PlayerObject.h"
#include "Lib/SerializeBuffer.h"

/**
 * 서버에서 받은 데이터에 대한 컨텐츠 처리를 진행하는 곳.
 */

bool netPacketProc_CreateMyCharacter(CSerializeBuffer* pSerializeBuffer)
{
	int outID;
	char outDir;
	short outX, outY;
	char outHP;
	*pSerializeBuffer >> outID >> outDir >> outX >> outY >> outHP;

	CPlayerObject* playerObject = (CPlayerObject*)SINGLETON(CObjectMgr)->CreateObject(eTYPE_PLAYER, outID);
	if (playerObject == nullptr)
		return false;
	
	SINGLETON(CGameMgr)->SetPlayerObject(playerObject);
	playerObject->SetPlayerCharacterTrue();
	playerObject->SetPosition(outX, outY);
	playerObject->SetHP(outHP);
	
	if (outDir == dfPACKET_MOVE_DIR_LL) {
		playerObject->SetDirection(false);
	}
	else {
		playerObject->SetDirection(true);
	}
	playerObject->InputActionProc(dfACTION_STAND);
	
	return true;
}

bool netPacketProc_CreateOtherCharacter(CSerializeBuffer* pSerializeBuffer)
{
	int outID;
	char outDir;
	short outX, outY;
	char outHP;
	*pSerializeBuffer >> outID >> outDir >> outX >> outY >> outHP;

	CPlayerObject* playerObject = (CPlayerObject*)SINGLETON(CObjectMgr)->CreateObject(eTYPE_PLAYER, outID);
	if (playerObject == nullptr)
		return false;

	playerObject->SetPosition(outX, outY);
	playerObject->SetHP(outHP);
	
	if (outDir == dfPACKET_MOVE_DIR_LL) {
		playerObject->SetDirection(false);
	}
	else {
		playerObject->SetDirection(true);
	}
	playerObject->InputActionProc(dfACTION_STAND);

	return true;
}

bool netPacketProc_DeleteCharacter(CSerializeBuffer* pSerializeBuffer)
{
	int outID;
	*pSerializeBuffer >> outID;

	if (SINGLETON(CObjectMgr)->EraseByObjectID(outID) == false)
		return false;

	return true;
}

bool netPacketProc_MoveStart(CSerializeBuffer* pSerializeBuffer)
{
	int outID;
	char outDir;
	short outX, outY;
	*pSerializeBuffer >> outID >> outDir >> outX >> outY;

	CPlayerObject* outPlayer;
	if (SINGLETON(CObjectMgr)->FindByObjectID(outID, (CBaseObject**)&outPlayer) == false)
		return false;

	switch (outDir)
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

	outPlayer->SetPosition(outX, outY);

	return true;
}

bool netPacketProc_MoveStop(CSerializeBuffer* pSerializeBuffer)
{
	int outID;
	char outDir;
	short outX, outY;
	*pSerializeBuffer >> outID >> outDir >> outX >> outY;

	CPlayerObject* outPlayer;
	if (SINGLETON(CObjectMgr)->FindByObjectID(outID, (CBaseObject**)&outPlayer) == false)
		return false;

	outPlayer->SetPosition(outX, outY);

	if (outDir == dfPACKET_MOVE_DIR_LL) {
		outPlayer->SetDirection(false);
	}
	else {
		outPlayer->SetDirection(true);
	}
	outPlayer->InputActionProc(dfACTION_STAND);

	return true;
}

bool netPacketProc_Attack1(CSerializeBuffer* pSerializeBuffer)
{
	int outID;
	char outDir;
	short outX, outY;
	*pSerializeBuffer >> outID >> outDir >> outX >> outY;

	CPlayerObject* outPlayer;
	if (SINGLETON(CObjectMgr)->FindByObjectID(outID, (CBaseObject**)&outPlayer) == false)
		return false;

	outPlayer->SetPosition(outX, outY);

	if (outDir == dfPACKET_MOVE_DIR_LL) {
		outPlayer->SetDirection(false);
	}
	else {
		outPlayer->SetDirection(true);
	}
	outPlayer->InputActionProc(dfACTION_ATTACK1);
	outPlayer->ResetAnimation();

	return true;
}

bool netPacketProc_Attack2(CSerializeBuffer* pSerializeBuffer)
{
	int outID;
	char outDir;
	short outX, outY;
	*pSerializeBuffer >> outID >> outDir >> outX >> outY;

	CPlayerObject* outPlayer;
	if (SINGLETON(CObjectMgr)->FindByObjectID(outID, (CBaseObject**)&outPlayer) == false)
		return false;
	
	outPlayer->SetPosition(outX, outY);

	if (outDir == dfPACKET_MOVE_DIR_LL) {
		outPlayer->SetDirection(false);
	}
	else {
		outPlayer->SetDirection(true);
	}
	outPlayer->InputActionProc(dfACTION_ATTACK2);
	outPlayer->ResetAnimation();

	return true;
}

bool netPacketProc_Attack3(CSerializeBuffer* pSerializeBuffer)
{
	int outID;
	char outDir;
	short outX, outY;
	*pSerializeBuffer >> outID >> outDir >> outX >> outY;

	CPlayerObject* outPlayer;
	if (SINGLETON(CObjectMgr)->FindByObjectID(outID, (CBaseObject**)&outPlayer) == false)
		return false;

	outPlayer->SetPosition(outX, outY);

	if (outDir == dfPACKET_MOVE_DIR_LL) {
		outPlayer->SetDirection(false);
	}
	else {
		outPlayer->SetDirection(true);
	}
	outPlayer->InputActionProc(dfACTION_ATTACK3);
	outPlayer->ResetAnimation();

	return true;
}

bool netPacketProc_Damage(CSerializeBuffer* pSerializeBuffer)
{
	int outAttackID;
	int outDamageID;
	char outDamageHP;
	*pSerializeBuffer >> outAttackID >> outDamageID >> outDamageHP;

	AddLog(L"attID: %d, defID: %d, HP: %d\n", outAttackID, outDamageID, outDamageHP);

	CPlayerObject* attackPlayer;
 	if (SINGLETON(CObjectMgr)->FindByObjectID(outAttackID, (CBaseObject**)&attackPlayer) == false)
		return false;

	CPlayerObject* damagePlayer;
	if (SINGLETON(CObjectMgr)->FindByObjectID(outDamageID, (CBaseObject**)&damagePlayer) == false)
		return false;

	attackPlayer->SetHitEffect(damagePlayer->GetCurX(), damagePlayer->GetCurY() - 47);
	damagePlayer->SetHP(outDamageHP);

	return true;
}

bool netPacketProc_Sync(CSerializeBuffer* pSerializeBuffer)
{
	int outID;
	short outX;
	short outY;
	*pSerializeBuffer >> outID >> outX >> outY;

	CPlayerObject* player;
	if (SINGLETON(CObjectMgr)->FindByObjectID(outID, (CBaseObject**)&player) == false)
		return false;

	player->SetPosition(outX, outY);
	AddLog(L"Sync : %d, %d ms\n", outX, outY);

	return true;
}

bool netPacketProc_Echo(CSerializeBuffer* pSerializeBuffer)
{
	int outEcho;
	*pSerializeBuffer >> outEcho;

	unsigned int millisec = GetTickCount();
	AddLog(L"Round Trip Time : %u ms\n", millisec - outEcho);

	return true;
}