#include "PacketProc.h"
#include "stdafx.h"

bool ppLoginReqLogin(CSerializeBuffer* pPacket, long long* pAccountNo, char* SessionKey)
{
	if (pPacket->GetDataSize() != 72)
		return false;

	(*pPacket) >> *pAccountNo;

	pPacket->GetData(SessionKey, 64 * sizeof(char));

	return true;
}