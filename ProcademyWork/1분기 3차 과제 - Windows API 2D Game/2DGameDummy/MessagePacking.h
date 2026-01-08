#pragma once

/**
 * 클라에서 보낼 데이터를 포장하는 곳. 메시지를 패킹하는 함수
 */

void mpMoveStart(stPACKET_CS_MOVE_START* pPacket, char dir, unsigned short x, unsigned short y);

void mpMoveStop(stPACKET_CS_MOVE_STOP* pPacket, char dir, unsigned short x, unsigned short y);

void mpAttack1(stPACKET_CS_ATTACK1* pPacket, char dir, unsigned short x, unsigned short y);

void mpAttack2(stPACKET_CS_ATTACK2* pPacket, char dir, unsigned short x, unsigned short y);

void mpAttack3(stPACKET_CS_ATTACK3* pPacket, char dir, unsigned short x, unsigned short y);