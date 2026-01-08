#pragma once

class CSerializeBuffer;
__forceinline bool ppGameReqLogin(CSerializeBuffer* pPacket, long long* pAccountNo, char* SessionKey, int* version);

__forceinline bool ppGameReqEcho(CSerializeBuffer* pPacket, long long* pAccountNo, long long* SendTick);