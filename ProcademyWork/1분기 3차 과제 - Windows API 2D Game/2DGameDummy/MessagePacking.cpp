#include "stdafx.h"
#include "MessagePacking.h"

/**
 * 클라에서 보낼 데이터를 포장하는 곳. 메시지를 패킹하는 함수
 */

void mpMoveStart(stPACKET_CS_MOVE_START* pPacket, char dir, unsigned short x, unsigned short y)
{
	pPacket->byCode = 0x89;
	pPacket->bySize = sizeof(stPACKET_CS_MOVE_START) - sizeof(stPacket_Header);
	pPacket->byType = dfPACKET_CS_MOVE_START;

	pPacket->Direction = dir;
	pPacket->X = x;
	pPacket->Y = y;
}

void mpMoveStop(stPACKET_CS_MOVE_STOP* pPacket, char dir, unsigned short x, unsigned short y)
{
	pPacket->byCode = 0x89;
	pPacket->bySize = sizeof(stPACKET_CS_MOVE_STOP) - sizeof(stPacket_Header);
	pPacket->byType = dfPACKET_CS_MOVE_STOP;

	pPacket->Direction = dir;
	pPacket->X = x;
	pPacket->Y = y;
}

void mpAttack1(stPACKET_CS_ATTACK1* pPacket, char dir, unsigned short x, unsigned short y)
{
	pPacket->byCode = 0x89;
	pPacket->bySize = sizeof(stPACKET_CS_ATTACK1) - sizeof(stPacket_Header);
	pPacket->byType = dfPACKET_CS_ATTACK1;

	pPacket->Direction = dir;
	pPacket->X = x;
	pPacket->Y = y;
}

void mpAttack2(stPACKET_CS_ATTACK2* pPacket, char dir, unsigned short x, unsigned short y)
{
	pPacket->byCode = 0x89;
	pPacket->bySize = sizeof(stPACKET_CS_ATTACK2) - sizeof(stPacket_Header);
	pPacket->byType = dfPACKET_CS_ATTACK2;

	pPacket->Direction = dir;
	pPacket->X = x;
	pPacket->Y = y;
}

void mpAttack3(stPACKET_CS_ATTACK3* pPacket, char dir, unsigned short x, unsigned short y)
{
	pPacket->byCode = 0x89;
	pPacket->bySize = sizeof(stPACKET_CS_ATTACK3) - sizeof(stPacket_Header);
	pPacket->byType = dfPACKET_CS_ATTACK3;

	pPacket->Direction = dir;
	pPacket->X = x;
	pPacket->Y = y;
}