#include "stdafx.h"

#include <Windows.h>
namespace NabzackoLibrary
{
	void ConvertMultiByteToWideChar(const char* multibyteStr, wchar_t** ppAllocWidebyte)
	{
		int iLen = MultiByteToWideChar(CP_ACP, 0, multibyteStr, strlen(multibyteStr), NULL, NULL);
		(*ppAllocWidebyte) = new wchar_t[iLen + 1];
		ZeroMemory(*ppAllocWidebyte, sizeof(wchar_t) * (iLen + 1));
		MultiByteToWideChar(CP_ACP, 0, multibyteStr, strlen(multibyteStr), *ppAllocWidebyte, iLen);
	}

	void ConvertMultiByteToWideChar_NoAlloc(const char* multibyteStr, wchar_t* pWidebyte, int iLen)
	{
		// 부족하면 부족한 만큼만 복사함.
		int copyLen = MultiByteToWideChar(CP_ACP, 0, multibyteStr, strlen(multibyteStr), pWidebyte, iLen);
		pWidebyte[copyLen] = '\0';
	}

	void ConvertWideCharToMultiByte(const wchar_t* widebyteStr, char** ppAllocMultibyte)
	{
		int iLen = WideCharToMultiByte(CP_ACP, 0, widebyteStr, -1, NULL, 0, NULL, NULL);
		(*ppAllocMultibyte) = new char[iLen + 1];
		ZeroMemory(*ppAllocMultibyte, iLen + 1);
		WideCharToMultiByte(CP_ACP, 0, widebyteStr, wcslen(widebyteStr), *ppAllocMultibyte, iLen, NULL, NULL);
	}

	void ConvertWideCharToMultiByte_NoAlloc(const wchar_t* widebyteStr, char* pMultibyte, int iLen)
	{
		// 부족하면 부족한 만큼만 복사함.
		int copyLen = WideCharToMultiByte(CP_ACP, 0, widebyteStr, wcslen(widebyteStr), pMultibyte, iLen, NULL, NULL);
		pMultibyte[copyLen] = '\0';
	}
}