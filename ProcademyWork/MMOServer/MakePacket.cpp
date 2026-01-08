#include "MakePacket.h"
#include "stdafx.h"

void mpGameResLogin(CSerializeBuffer* pPacket, BYTE Status, INT64 AccountNo)
{
	WORD resType = en_PACKET_CS_GAME_RES_LOGIN;
	(*pPacket) << resType << Status << AccountNo;
}

void mpGameResEcho(CSerializeBuffer* pPacket, INT64 AccountNo, LONGLONG SendTick)
{
	WORD resType = en_PACKET_CS_GAME_RES_ECHO;
	(*pPacket) << resType << AccountNo << SendTick;
}