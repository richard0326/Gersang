#pragma once

class CSerializeBuffer;
__forceinline void mpGameResLogin(CSerializeBuffer* pPacket, BYTE Status, INT64 AccountNo);

__forceinline void mpGameResEcho(CSerializeBuffer* pPacket, INT64 AccountNo, LONGLONG SendTick);