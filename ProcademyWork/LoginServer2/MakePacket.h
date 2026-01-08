#pragma once

class CSerializeBuffer;
__forceinline void mpLoginResLogin(CSerializeBuffer* pPacket, INT64 AccountNo, BYTE Status, WCHAR* pID, WCHAR* pNickname, WCHAR* pGameServerIP, USHORT GameServerPort, WCHAR* pChatServerIP, USHORT ChatServerPort);