#pragma once

class CSerializeBuffer;
void mpCreateMyCharacter(CSerializeBuffer* pPacket, DWORD dwSessionID, BYTE byDir, short shX, short shY, char chHP);

void mpCreateOtherCharacter(CSerializeBuffer* pPacket, DWORD dwSessionID, BYTE byDir, short shX, short shY, char chHP);

void mpDeleteCharacter(CSerializeBuffer* pPacket, DWORD dwSessionID);

void mpMoveStart(CSerializeBuffer* pPacket, DWORD dwSessionID, BYTE byDir, short shX, short shY);

void mpMoveStop(CSerializeBuffer* pPacket, DWORD dwSessionID, BYTE byDir, short shX, short shY);

void mpAttack1(CSerializeBuffer* pPacket, DWORD dwSessionID, BYTE byDir, short shX, short shY);

void mpAttack2(CSerializeBuffer* pPacket, DWORD dwSessionID, BYTE byDir, short shX, short shY);

void mpAttack3(CSerializeBuffer* pPacket, DWORD dwSessionID, BYTE byDir, short shX, short shY);

void mpDamage(CSerializeBuffer* pPacket, DWORD dwAttackID, DWORD dwDamageID, char chHP);

void mpSync(CSerializeBuffer* pPacket, DWORD dwSessionID, short shX, short shY);

void mpEcho(CSerializeBuffer* pPacket, DWORD dwTime);