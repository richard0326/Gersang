#include "stdafx.h"
#include <Windows.h>
#include <iostream>
#include "MemoryAllocMgr.h"
#include <time.h>

struct stAllocInfo {
	void* vpAddress;
	size_t	iSize;
	char* cpFilename;
	__int32	iFileline;
	bool	bArray;
};

enum class eErroN {
	NONE,
	NOALLOC,
	ARRAY,
	LEAK,
};

class CMemoryAllocMgr
{
private:
	enum {
		MAX_ALLOC = 50000,
		MAX_FILENAME = 256,
		MAX_BUF = 5000,
	};

	stAllocInfo* m_AllocArr[MAX_ALLOC];

	char m_fileName[MAX_FILENAME];

public:
	CMemoryAllocMgr();
	~CMemoryAllocMgr();

	bool SaveAlloc(void* ptr, size_t size, const char* filename, __int32 fileline, bool isArray = false);
	bool FindAlloc(void* ptr, void** out, stAllocInfo* outInfo, eErroN* errN, bool isArray = false);
	void ReleaseAlloc(void* ptr);

private:
	void LeakCheck();

public:
	void WriteLog(eErroN eErroNum, stAllocInfo* info);
};

static CMemoryAllocMgr g_memAllocMgr;

CMemoryAllocMgr::CMemoryAllocMgr()
{
	for (__int32 i = 0; i < MAX_ALLOC; ++i)
	{
		m_AllocArr[i] = nullptr;
	}

	__time64_t long_time;
	_time64(&long_time);

	tm newtime;
	errno_t err = _localtime64_s(&newtime, &long_time);
	if (err)
	{
		throw "Invalid argument to _localtime64_s. \n";
	}

	sprintf_s(m_fileName, "Alloc_%04d%02d%02d_%02d%02d%02d.txt", newtime.tm_year + 1900, newtime.tm_mon + 1, newtime.tm_mday, newtime.tm_hour, newtime.tm_min, newtime.tm_sec);
}

CMemoryAllocMgr::~CMemoryAllocMgr()
{
	LeakCheck();
}

bool CMemoryAllocMgr::SaveAlloc(void* ptr, size_t size, const char* filename, __int32 fileline, bool isArray)
{
	for (INT32 i = 0; i < MAX_ALLOC; ++i)
	{
		if (m_AllocArr[i] == nullptr)
		{
			m_AllocArr[i] = (stAllocInfo*)malloc(sizeof(stAllocInfo));
			m_AllocArr[i]->cpFilename = (char*)filename;
			m_AllocArr[i]->iSize = size;
			m_AllocArr[i]->vpAddress = ptr;
			m_AllocArr[i]->iFileline = fileline;
			m_AllocArr[i]->bArray = isArray;
			return true;
		}
	}

	return false;
}

bool CMemoryAllocMgr::FindAlloc(void* ptr, void** outPtr, stAllocInfo* outInfo, eErroN* errN, bool isArray)
{
	*errN = eErroN::NONE;

	for (__int32 i = 0; i < MAX_ALLOC; ++i)
	{
		if (m_AllocArr[i] != nullptr && m_AllocArr[i]->vpAddress == ptr)
		{
			// 소멸자가 없는 객체 또는 일반 변수 중
			// bArray 값이 틀린 경우
			if (m_AllocArr[i]->bArray != isArray)
			{
				*outPtr = m_AllocArr[i]->vpAddress;
				*outInfo = *m_AllocArr[i];
				*errN = eErroN::ARRAY;
				return false;
			}

			*outPtr = m_AllocArr[i]->vpAddress;
			return true;
		}
	}

	// delete에 new[]의 형식이 들어온 경우
	for (__int32 i = 0; i < MAX_ALLOC; ++i)
	{
		if (m_AllocArr[i] != nullptr && m_AllocArr[i]->bArray == true)
		{
			void* tmp = (stAllocInfo*)((char*)(m_AllocArr[i]->vpAddress) + sizeof(size_t));
			if (tmp == ptr)
			{
				*outPtr = m_AllocArr[i]->vpAddress;
				*outInfo = *m_AllocArr[i];
				*errN = eErroN::ARRAY;
				return true;
			}
		}
	}

	(*outInfo).vpAddress = ptr;
	*errN = eErroN::NOALLOC;
	return false;
}

void CMemoryAllocMgr::ReleaseAlloc(void* ptr)
{
	for (__int32 i = 0; i < MAX_ALLOC; ++i)
	{
		if (m_AllocArr[i] != nullptr && m_AllocArr[i]->vpAddress == ptr)
		{
			free(m_AllocArr[i]);
			m_AllocArr[i] = nullptr;
			return;
		}
	}
}

void CMemoryAllocMgr::LeakCheck()
{
	for (__int32 i = 0; i < MAX_ALLOC; ++i)
	{
		if (m_AllocArr[i] != NULL)
		{
			WriteLog(eErroN::LEAK, m_AllocArr[i]);
		}
	}
}

void CMemoryAllocMgr::WriteLog(eErroN eErroNum, stAllocInfo* info)
{
	if (eErroNum == eErroN::NONE)
	{
		return;
	}

	FILE* fp = nullptr;
	errno_t err = 1;

	// 파일을 읽을때까지 계속 접근한다.
	while (!(fopen_s(&fp, m_fileName, "ab") == 0)) {}

	char buf[MAX_BUF];
	__int32 size = 0;
	switch (eErroNum)
	{
	case eErroN::ARRAY:
		size = sprintf_s(buf, MAX_BUF, "ARRAY [0x%p][%Id] %s : %d\n", info->vpAddress, info->iSize, info->cpFilename, info->iFileline);
		break;
	case eErroN::LEAK:
		size = sprintf_s(buf, MAX_BUF, "LEAK [0x%p][%Id] %s : %d\n", info->vpAddress, info->iSize, info->cpFilename, info->iFileline);
		break;
	case eErroN::NOALLOC:
		size = sprintf_s(buf, MAX_BUF, "NOALLOC [0x%p] \n", info->vpAddress);
		break;
	}

	if (size == -1 || fwrite(buf, size, 1, fp) != 1)
	{
		fclose(fp);
		throw "CMemoryAllocMgr : 여기서 오류나면 답이 없다...!\n";
	}

	fclose(fp);
}

#ifdef new
#undef new

void* operator new (size_t size)
{
	void* vptr = malloc(size);
	if (g_memAllocMgr.SaveAlloc(vptr, size, "표준 라이브러리", 0) == false)
	{
		return nullptr;
	}

	return vptr;
}

void* operator new[](size_t size)
{
	void* vptr = malloc(size);
	if (g_memAllocMgr.SaveAlloc(vptr, size, "표준 라이브러리", 0, true) == false)
	{
		return nullptr;
	}

	return vptr;
}

void* operator new (size_t size, const char* File, int Line)
{
	void* vptr = malloc(size);
	if (g_memAllocMgr.SaveAlloc(vptr, size, File, Line) == false)
	{
		return nullptr;
	}

	return vptr;
}

void* operator new[](size_t size, const char* File, int Line)
{
	void* vptr = malloc(size);
	if (g_memAllocMgr.SaveAlloc(vptr, size, File, Line, true) == false)
	{
		return nullptr;
	}

	return vptr;
}
#endif

// 할당하지 않으면 문제가 생김...
// 내부적으로는 채워 넣지 말것!!!
void operator delete (void* p, const char* File, int Line)
{

}
void operator delete[](void* p, const char* File, int Line)
{
}

// 실제로 사용할 delete
void operator delete (void* p)
{
	void* ptr = nullptr;
	stAllocInfo info;
	eErroN errNo;
	bool ret = g_memAllocMgr.FindAlloc(p, &ptr, &info, &errNo);

	g_memAllocMgr.WriteLog(errNo, &info);

	if (ret == true)
	{
		free(ptr);
		g_memAllocMgr.ReleaseAlloc(ptr);
	}
}

void operator delete[](void* p)
{
	void* ptr = nullptr;
	stAllocInfo info;
	eErroN errNo;
	bool ret = g_memAllocMgr.FindAlloc(p, &ptr, &info, &errNo, true);

	g_memAllocMgr.WriteLog(errNo, &info);

	if (ret == true)
	{
		free(ptr);
		g_memAllocMgr.ReleaseAlloc(ptr);
	}
}