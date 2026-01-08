#pragma once

/**
 * 서버에서 받은 데이터에 대한 컨텐츠 처리를 진행하는 곳.
 */

bool netPacketProc_CreateMyCharacter(char* pPacketBuffer, int* ID);

bool netPacketProc_CreateOtherCharacter(char* pPacketBuffer);

bool netPacketProc_DeleteCharacter(char* pPacketBuffer);

bool netPacketProc_MoveStart(char* pPacketBuffer);

bool netPacketProc_MoveStop(char* pPacketBuffer);

bool netPacketProc_Attack1(char* pPacketBuffer);

bool netPacketProc_Attack2(char* pPacketBuffer);

bool netPacketProc_Attack3(char* pPacketBuffer);

bool netPacketProc_Damage(char* pPacketBuffer);