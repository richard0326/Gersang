#include "stdafx.h"
#include "MessagePacking.h"
#include "Lib/SerializeBuffer.h"

/**
 * 클라에서 보낼 데이터를 포장하는 곳. 메시지를 패킹하는 함수
 */

void mpMoveStart(CSerializeBuffer* pSerializeBuffer, char dir, unsigned short x, unsigned short y)
{
	BYTE byCode = 0x89;
	BYTE bySize = sizeof(dir) + sizeof(x) + sizeof(y);
	BYTE byType = dfPACKET_CS_MOVE_START;
	*pSerializeBuffer << byCode << bySize << byType << dir << x << y;
}

void mpMoveStop(CSerializeBuffer* pSerializeBuffer, char dir, unsigned short x, unsigned short y)
{
	BYTE byCode = 0x89;
	BYTE bySize = sizeof(dir) + sizeof(x) + sizeof(y);
	BYTE byType = dfPACKET_CS_MOVE_STOP;
	*pSerializeBuffer << byCode << bySize << byType << dir << x << y;
}

void mpAttack1(CSerializeBuffer* pSerializeBuffer, char dir, unsigned short x, unsigned short y)
{
	BYTE byCode = 0x89;
	BYTE bySize = sizeof(dir) + sizeof(x) + sizeof(y);
	BYTE byType = dfPACKET_CS_ATTACK1;
	*pSerializeBuffer << byCode << bySize << byType << dir << x << y;
}

void mpAttack2(CSerializeBuffer* pSerializeBuffer, char dir, unsigned short x, unsigned short y)
{
	BYTE byCode = 0x89;
	BYTE bySize = sizeof(dir) + sizeof(x) + sizeof(y);
	BYTE byType = dfPACKET_CS_ATTACK2;
	*pSerializeBuffer << byCode << bySize << byType << dir << x << y;
}

void mpAttack3(CSerializeBuffer* pSerializeBuffer, char dir, unsigned short x, unsigned short y)
{
	BYTE byCode = 0x89;
	BYTE bySize = sizeof(dir) + sizeof(x) + sizeof(y);
	BYTE byType = dfPACKET_CS_ATTACK3;
	*pSerializeBuffer << byCode << bySize << byType << dir << x << y;
}

void mpEcho(CSerializeBuffer* pSerializeBuffer, char dir, unsigned short x, unsigned short y)
{
	BYTE byCode = 0x89;
	BYTE bySize = sizeof(unsigned int);
	BYTE byType = dfPACKET_CS_ECHO;

	unsigned int millisec = GetTickCount();
	*pSerializeBuffer << byCode << bySize << byType << millisec;
}