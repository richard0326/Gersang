#include "stdafx.h"

#include <cstdio>
#include <Windows.h>
#include <time.h>
#include "Profiler.h"
#include <cassert>

#ifdef _DEBUG
#define DEBUG_ASSERT(wstr)		_wassert(wstr, _CRT_WIDE(__FILE__), __LINE__)
#else
#define DEBUG_ASSERT(wstr)		;
#endif

struct PROFILE
{
	bool			isFlag;				// 프로파일의 사용 여부. (배열시에만)
	wchar_t			szName[64];			// 프로파일 샘플 이름.

	LARGE_INTEGER	lStartTime;			// 프로파일 샘플 실행 시간.

	__int64			iTotalTime;			// 전체 사용시간 카운터 Time.	(출력시 호출회수로 나누어 평균 구함)
	__int64			iMin[2];			// 최소 사용시간 카운터 Time.	(초단위로 계산하여 저장 / [0] 가장최소 [1] 다음 최소 [2])
	__int64			iMax[2];			// 최대 사용시간 카운터 Time.	(초단위로 계산하여 저장 / [0] 가장최대 [1] 다음 최대 [2])

	__int64			iCall;				// 누적 호출 횟수.
};

class CProfiler
{
	enum {
		MAX_PROFILE = 10,
	};
	PROFILE m_ProfileArr[MAX_PROFILE];
	double m_freq;
public:
	CProfiler();
	~CProfiler();

	void Begin(const wchar_t* str);
	void End(const wchar_t* str);
	void OutText(const wchar_t* szFileName);
	void Reset(void);
};

CProfiler::CProfiler()
{
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);

	// 1s -> 1㎲
	m_freq = freq.QuadPart / 1000000.0;
	memset(m_ProfileArr, 0, sizeof(PROFILE) * MAX_PROFILE);
}

CProfiler::~CProfiler()
{

}

void CProfiler::Begin(const wchar_t* str)
{
	for (__int32 i = 0; i < MAX_PROFILE; i++)
	{
		if (m_ProfileArr[i].isFlag == true && wcscmp(m_ProfileArr[i].szName, str) == 0)
		{
			if (m_ProfileArr[i].lStartTime.QuadPart != 0)
			{
				// 오류
				DEBUG_ASSERT(L"Porfile Start Error\n");
			}

			QueryPerformanceCounter(&m_ProfileArr[i].lStartTime);
			return;
		}
	}

	for (__int32 i = 0; i < MAX_PROFILE; i++)
	{
		if (m_ProfileArr[i].isFlag == FALSE)
		{
			m_ProfileArr[i].isFlag = true;
			wcscpy_s(m_ProfileArr[i].szName, str);
			QueryPerformanceCounter(&m_ProfileArr[i].lStartTime);
			return;
		}
	}

	// 오류
	DEBUG_ASSERT(L"Porfile Start Error\n");
}

void CProfiler::End(const wchar_t* str)
{
	LARGE_INTEGER endTime;
	QueryPerformanceCounter(&endTime);

	for (__int32 i = 0; i < MAX_PROFILE; i++)
	{
		if (m_ProfileArr[i].isFlag == true && wcscmp(m_ProfileArr[i].szName, str) == 0)
		{
			if (m_ProfileArr[i].lStartTime.QuadPart == 0)
			{
				// 오류
				DEBUG_ASSERT(L"Porfile End Error\n");
			}

			__int64 iEllapse = (endTime.QuadPart - m_ProfileArr[i].lStartTime.QuadPart);

			m_ProfileArr[i].lStartTime.QuadPart = 0;
			m_ProfileArr[i].iTotalTime += iEllapse;
			m_ProfileArr[i].iCall++;

			if (m_ProfileArr[i].iMax[0] < iEllapse)
			{
				m_ProfileArr[i].iMax[1] = m_ProfileArr[i].iMax[0];
				m_ProfileArr[i].iMax[0] = iEllapse;
			}
			else if(m_ProfileArr[i].iMax[1] < iEllapse)
			{
				m_ProfileArr[i].iMax[1] = iEllapse;
			}

			if (m_ProfileArr[i].iMin[0] == 0 || m_ProfileArr[i].iMin[0] > iEllapse)
			{
				m_ProfileArr[i].iMin[1] = m_ProfileArr[i].iMin[0];
				m_ProfileArr[i].iMin[0] = iEllapse;
			}
			else if (m_ProfileArr[i].iMin[1] == 0 || m_ProfileArr[i].iMin[1] > iEllapse)
			{
				m_ProfileArr[i].iMin[1] = iEllapse;
			}

			return;
		}
	}

	// 오류
	DEBUG_ASSERT(L"Porfile End Error\n");
}

void CProfiler::OutText(const wchar_t* szFileName)
{
	FILE* fp;
	if (_wfopen_s(&fp, szFileName, L"wb") != 0)
	{
		// 오류
		DEBUG_ASSERT(L"Porfile File Open Error\n");
	}

	wchar_t totalbuf[2000];
	memset(totalbuf, 0, sizeof(totalbuf));
	__int32 size = 0;

	wchar_t intro[3][150] = {
		L"---------------------------------------------------------------------------------------------------------------\n",
		L"        Name        |         Average         |          Min           |         Max            |     Call    | \n",
		L"---------------------------------------------------------------------------------------------------------------\n",
	};

	for (__int32 i = 0; i < 3; i++)
	{
		wcscat_s(totalbuf + size, wcslen(intro[i]) * sizeof(wchar_t), intro[i]);
		size += wcslen(intro[i]);
	}

	for (__int32 i = 0; i < MAX_PROFILE; i++)
	{
		if (m_ProfileArr[i].isFlag == true && m_ProfileArr[i].iCall != 0)
		{
			wchar_t tempBuf[1000];
			swprintf_s(tempBuf, L"%20s | %20.4lf㎲ | %20.4lf㎲ | %20.4lf㎲ | %10lld\n", m_ProfileArr[i].szName, 
				m_ProfileArr[i].iTotalTime / m_freq / (double)m_ProfileArr[i].iCall,
				((m_ProfileArr[i].iMin[0] + m_ProfileArr[i].iMin[1]) / 2.0) / m_freq,
				((m_ProfileArr[i].iMax[0] + m_ProfileArr[i].iMax[1]) / 2.0) / m_freq,
				m_ProfileArr[i].iCall);

			wcscat_s(totalbuf + size, wcslen(tempBuf) * sizeof(wchar_t), tempBuf);
			size += wcslen(tempBuf);
		}
	}

	if (fwrite(totalbuf, size * sizeof(wchar_t), 1, fp) != 1)
	{
		// 오류
		fclose(fp);
		DEBUG_ASSERT(L"Porfile File Write Error\n");
	}

	fclose(fp);
}

void CProfiler::Reset(void)
{
	for (__int32 i = 0; i < MAX_PROFILE; i++)
	{
		if (m_ProfileArr[i].isFlag == true)
		{
			m_ProfileArr[i].lStartTime.QuadPart = 0;
			m_ProfileArr[i].iTotalTime = 0;
			m_ProfileArr[i].iMax[0] = m_ProfileArr[i].iMax[1] = 0;
			m_ProfileArr[i].iMin[0] = m_ProfileArr[i].iMin[1] = 0;
			m_ProfileArr[i].iCall = 0;
		}
	}
}

CProfiler g_Profiler;

void ProfileBegin(const wchar_t* szName)
{
	g_Profiler.Begin(szName);
}

void ProfileEnd(const wchar_t* szName)
{
	g_Profiler.End(szName);
}

void ProfileDataOutText(const wchar_t* szFileName)
{
	g_Profiler.OutText(szFileName);
}

void ProfileReset(void)
{
	g_Profiler.Reset();
}