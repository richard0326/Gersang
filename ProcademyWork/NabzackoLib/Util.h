#pragma once

namespace NabzackoLibrary
{

	// 세션ID(8바이트)의 뒤에 2바이트를 index로 사용하는 define
#define CONVERT_INFO_TO_ID(SessionID, index)		((0x0000ffffffffffff & SessionID) << 16 | (0x000000000000ffff & ((unsigned long long)index)))
	// 세션ID(8바이트)의 뒤에 2바이트인 index를 추출하는 define
#define	CONVERT_ID_TO_INDEX(SessionID)				(0x000000000000ffff & SessionID)

	// MultiByte 문자열에서 Wide Character 문자열로 변환하는 함수 (내부 동적 할당)
	void ConvertMultiByteToWideChar(const char* multibyteStr, wchar_t** ppAllocWidebyte);
	// MultiByte 문자열에서 Wide Character 문자열로 변환하는 함수
	void ConvertMultiByteToWideChar_NoAlloc(const char* multibyteStr, wchar_t* pWidebyte, int iLen);
	// Wide Character문자열에서 Multibyte 문자열로 변환하는 함수 (내부 동적 할당)
	void ConvertWideCharToMultiByte(const wchar_t* widebyteStr, char** ppAllocMultibyte);
	// Wide Character문자열에서 Multibyte 문자열로 변환하는 함수
	void ConvertWideCharToMultiByte_NoAlloc(const wchar_t* widebyteStr, char* pMultibyte, int iLen);
}