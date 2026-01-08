#pragma once

/**
 * 서버에서 받은 데이터에 대한 컨텐츠 처리를 진행하는 곳.
 */
class CSerializeBuffer;
bool netPacketProc_CreateMyCharacter(CSerializeBuffer* pSerializeBuffer);

bool netPacketProc_CreateOtherCharacter(CSerializeBuffer* pSerializeBuffer);

bool netPacketProc_DeleteCharacter(CSerializeBuffer* pSerializeBuffer);

bool netPacketProc_MoveStart(CSerializeBuffer* pSerializeBuffer);

bool netPacketProc_MoveStop(CSerializeBuffer* pSerializeBuffer);

bool netPacketProc_Attack1(CSerializeBuffer* pSerializeBuffer);

bool netPacketProc_Attack2(CSerializeBuffer* pSerializeBuffer);

bool netPacketProc_Attack3(CSerializeBuffer* pSerializeBuffer);

bool netPacketProc_Damage(CSerializeBuffer* pSerializeBuffer);

bool netPacketProc_Sync(CSerializeBuffer* pSerializeBuffer);

bool netPacketProc_Echo(CSerializeBuffer* pSerializeBuffer);