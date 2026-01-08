#pragma once

#define RandomUINT64		(((UINT64)rand() & 0xFFFF) | (((UINT64)(rand() & 0xFFFF)) << 16) | (((UINT64)(rand() & 0xFFFF)) << 32) | (((UINT64)(rand() & 0xFFFF)) << 48))
#define RandomINT64		(((__int64)rand() & 0xFFFF) | (((__int64)(rand() & 0xFFFF)) << 16) | (((__int64)(rand() & 0xFFFF)) << 32) | (((__int64)(rand() & 0x7FFF)) << 48))

namespace Util {
	bool Init_Util();

	__int32 GetDegreeFromPt1_Pt2(__int32 FromX, __int32 FromY, __int32 ToX, __int32 ToY);
}