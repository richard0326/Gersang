#include "MakePacket.h"
#include "stdafx.h"

void mpLoginResLogin(CSerializeBuffer* pPacket, INT64 AccountNo, BYTE Status, WCHAR* pID, WCHAR* pNickname, 
	WCHAR* pGameServerIP, USHORT GameServerPort, WCHAR* pChatServerIP, USHORT ChatServerPort)
{
	WORD resType = en_PACKET_CS_LOGIN_RES_LOGIN;
	(*pPacket) << resType << AccountNo << Status;
	pPacket->PutData((char*)pID, 20 * sizeof(wchar_t));
	pPacket->PutData((char*)pNickname, 20 * sizeof(wchar_t));
	pPacket->PutData((char*)pGameServerIP, 16 * sizeof(wchar_t));
	(*pPacket) << GameServerPort;
	pPacket->PutData((char*)pChatServerIP, 16 * sizeof(wchar_t));
	(*pPacket) << ChatServerPort;
}