#include "PacketProc.h"
#include "stdafx.h"

bool ppGameReqLogin(CSerializeBuffer* pPacket, long long* pAccountNo, char* SessionKey, int* version)
{
	if (pPacket->GetDataSize() != 76)
		return false;

	(*pPacket) >> *pAccountNo;

	pPacket->GetData(SessionKey, 64 * sizeof(char));
	
	(*pPacket) >> *version;

	return true;
}

bool ppGameReqEcho(CSerializeBuffer* pPacket, long long* pAccountNo, long long* SendTick)
{
	if (pPacket->GetDataSize() != 16)
		return false;

	(*pPacket) >> *pAccountNo >> *SendTick;

	return true;
}