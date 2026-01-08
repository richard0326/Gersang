#include "stdafx.h"
#include "MakePacket.h"

void mpCreateMyCharacter(CSerializeBuffer* pPacket, DWORD dwSessionID, BYTE byDir, short shX, short shY, char chHP)
{
	st_PACKET_HEADER stPacketHeader;
	stPacketHeader.byCode = dfPACKET_CODE;
	stPacketHeader.bySize = sizeof(DWORD) + sizeof(BYTE) + sizeof(short) + sizeof(short) + sizeof(char);
	stPacketHeader.byType = dfPACKET_SC_CREATE_MY_CHARACTER;
	pPacket->Clear();
	pPacket->PutData((char*)&stPacketHeader, sizeof(st_PACKET_HEADER));
	*pPacket << dwSessionID;
	*pPacket << byDir;
	*pPacket << shX;
	*pPacket << shY;
	*pPacket << chHP;
}

void mpCreateOtherCharacter(CSerializeBuffer* pPacket, DWORD dwSessionID, BYTE byDir, short shX, short shY, char chHP)
{
	st_PACKET_HEADER stPacketHeader;
	stPacketHeader.byCode = dfPACKET_CODE;
	stPacketHeader.bySize = sizeof(DWORD) + sizeof(BYTE) + sizeof(short) + sizeof(short) + sizeof(char);
	stPacketHeader.byType = dfPACKET_SC_CREATE_OTHER_CHARACTER;
	pPacket->Clear();
	pPacket->PutData((char*)&stPacketHeader, sizeof(st_PACKET_HEADER));
	*pPacket << dwSessionID;
	*pPacket << byDir;
	*pPacket << shX;
	*pPacket << shY;
	*pPacket << chHP;
}

void mpDeleteCharacter(CSerializeBuffer* pPacket, DWORD dwSessionID)
{
	st_PACKET_HEADER stPacketHeader;
	stPacketHeader.byCode = dfPACKET_CODE;
	stPacketHeader.bySize = sizeof(DWORD);
	stPacketHeader.byType = dfPACKET_SC_DELETE_CHARACTER;
	pPacket->Clear();
	pPacket->PutData((char*)&stPacketHeader, sizeof(st_PACKET_HEADER));
	*pPacket << dwSessionID;
}

void mpMoveStart(CSerializeBuffer* pPacket, DWORD dwSessionID, BYTE byDir, short shX, short shY)
{
	st_PACKET_HEADER stPacketHeader;
	stPacketHeader.byCode = dfPACKET_CODE;
	stPacketHeader.bySize = sizeof(DWORD) + sizeof(BYTE) + sizeof(short) + sizeof(short);
	stPacketHeader.byType = dfPACKET_SC_MOVE_START;
	pPacket->Clear();
	pPacket->PutData((char*)&stPacketHeader, sizeof(st_PACKET_HEADER));
	*pPacket << dwSessionID;
	*pPacket << byDir;
	*pPacket << shX;
	*pPacket << shY;
}

void mpMoveStop(CSerializeBuffer* pPacket, DWORD dwSessionID, BYTE byDir, short shX, short shY)
{
	st_PACKET_HEADER stPacketHeader;
	stPacketHeader.byCode = dfPACKET_CODE;
	stPacketHeader.bySize = sizeof(DWORD) + sizeof(BYTE) + sizeof(short) + sizeof(short);
	stPacketHeader.byType = dfPACKET_SC_MOVE_STOP;
	pPacket->Clear();
	pPacket->PutData((char*)&stPacketHeader, sizeof(st_PACKET_HEADER));
	*pPacket << dwSessionID;
	*pPacket << byDir;
	*pPacket << shX;
	*pPacket << shY;
}

void mpAttack1(CSerializeBuffer* pPacket, DWORD dwSessionID, BYTE byDir, short shX, short shY)
{
	st_PACKET_HEADER stPacketHeader;
	stPacketHeader.byCode = dfPACKET_CODE;
	stPacketHeader.bySize = sizeof(DWORD) + sizeof(BYTE) + sizeof(short) + sizeof(short);
	stPacketHeader.byType = dfPACKET_SC_ATTACK1;
	pPacket->Clear();
	pPacket->PutData((char*)&stPacketHeader, sizeof(st_PACKET_HEADER));
	*pPacket << dwSessionID;
	*pPacket << byDir;
	*pPacket << shX;
	*pPacket << shY;
}

void mpAttack2(CSerializeBuffer* pPacket, DWORD dwSessionID, BYTE byDir, short shX, short shY)
{
	st_PACKET_HEADER stPacketHeader;
	stPacketHeader.byCode = dfPACKET_CODE;
	stPacketHeader.bySize = sizeof(DWORD) + sizeof(BYTE) + sizeof(short) + sizeof(short);
	stPacketHeader.byType = dfPACKET_SC_ATTACK2;
	pPacket->Clear();
	pPacket->PutData((char*)&stPacketHeader, sizeof(st_PACKET_HEADER));
	*pPacket << dwSessionID;
	*pPacket << byDir;
	*pPacket << shX;
	*pPacket << shY;
}

void mpAttack3(CSerializeBuffer* pPacket, DWORD dwSessionID, BYTE byDir, short shX, short shY)
{
	st_PACKET_HEADER stPacketHeader;
	stPacketHeader.byCode = dfPACKET_CODE;
	stPacketHeader.bySize = sizeof(DWORD) + sizeof(BYTE) + sizeof(short) + sizeof(short);
	stPacketHeader.byType = dfPACKET_SC_ATTACK3;
	pPacket->Clear();
	pPacket->PutData((char*)&stPacketHeader, sizeof(st_PACKET_HEADER));
	*pPacket << dwSessionID;
	*pPacket << byDir;
	*pPacket << shX;
	*pPacket << shY;
}

void mpDamage(CSerializeBuffer* pPacket, DWORD dwAttackID, DWORD dwDamageID, char chHP)
{
	st_PACKET_HEADER stPacketHeader;
	stPacketHeader.byCode = dfPACKET_CODE;
	stPacketHeader.bySize = sizeof(DWORD) + sizeof(DWORD) + sizeof(char);
	stPacketHeader.byType = dfPACKET_SC_DAMAGE;
	pPacket->Clear();
	pPacket->PutData((char*)&stPacketHeader, sizeof(st_PACKET_HEADER));
	*pPacket << dwAttackID;
	*pPacket << dwDamageID;
	*pPacket << chHP;
}

void mpSync(CSerializeBuffer* pPacket, DWORD dwPlayerID, short shX, short shY)
{
	st_PACKET_HEADER stPacketHeader;
	stPacketHeader.byCode = dfPACKET_CODE;
	stPacketHeader.bySize = sizeof(DWORD) + sizeof(short) + sizeof(short);
	stPacketHeader.byType = dfPACKET_SC_SYNC;
	pPacket->Clear();
	pPacket->PutData((char*)&stPacketHeader, sizeof(st_PACKET_HEADER));
	*pPacket << dwPlayerID;
	*pPacket << shX;
	*pPacket << shY;
}

void mpEcho(CSerializeBuffer* pPacket, DWORD dwTime)
{
	st_PACKET_HEADER stPacketHeader;
	stPacketHeader.byCode = dfPACKET_CODE;
	stPacketHeader.bySize = sizeof(DWORD);
	stPacketHeader.byType = dfPACKET_SC_ECHO;
	pPacket->Clear();
	pPacket->PutData((char*)&stPacketHeader, sizeof(st_PACKET_HEADER));
	*pPacket << dwTime;
}