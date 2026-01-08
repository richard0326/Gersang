#include "stdafx.h"

#include <cstdio>
#include <Windows.h>
#include <time.h>
#include "Profiler.h"
#include "Synchronized.h"

class CProfiler
{
	struct st_ProfileInfo
	{
		bool			isFlag;				// 프로파일의 사용 여부. (배열시에만)
		bool			isReset;
		wchar_t			szName[64];			// 프로파일 샘플 이름.

		LARGE_INTEGER	lStartTime;			// 프로파일 샘플 실행 시간.

		__int64			iTotalTime;			// 전체 사용시간 카운터 Time.	(출력시 호출회수로 나누어 평균 구함)
		__int64			iMin[2];			// 최소 사용시간 카운터 Time.	(초단위로 계산하여 저장 / [0] 가장최소 [1] 다음 최소 [2])
		__int64			iMax[2];			// 최대 사용시간 카운터 Time.	(초단위로 계산하여 저장 / [0] 가장최대 [1] 다음 최대 [2])

		__int64			iCall;				// 누적 호출 횟수.
	};

	enum {
		MAX_PROFILE = 20,
	};
public:
	CProfiler();
	~CProfiler();

	void Begin(const wchar_t* str);
	void End(const wchar_t* str);
	void OutText(const wchar_t* szFileName);
	void Reset(void);
private:
	st_ProfileInfo m_ProfileArr[MAX_PROFILE];
	double m_freq;
	SRWLOCK m_srwlock;
};

class CProfilerManager
{
	struct st_ProfilerSet
	{
		bool isUsed = false;
		CProfiler profiler;
	};

	enum {
		MAX_THREAD_CNT = 20,
	};
public:
	CProfilerManager();
	~CProfilerManager();

	void Begin(const wchar_t* str);
	void End(const wchar_t* str);
	void OutText(const wchar_t* szFileName);
	void Reset(void);

private:
	DWORD m_tlsIndex = 0;
	CProfiler* m_pProfilerArr[MAX_THREAD_CNT];
};

CProfiler::CProfiler()
{
	InitializeSRWLock(&m_srwlock);

	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);

	// 1s -> 1㎲
	m_freq = freq.QuadPart / 1000000.0;
	memset(m_ProfileArr, 0, sizeof(st_ProfileInfo) * MAX_PROFILE);
}

CProfiler::~CProfiler()
{

}

void CProfiler::Begin(const wchar_t* str)
{
	CSynchronizedSRWShared srwlockShared(&m_srwlock);

	for (__int32 i = 0; i < MAX_PROFILE; i++)
	{
		if (m_ProfileArr[i].isFlag == true && wcscmp(m_ProfileArr[i].szName, str) == 0)
		{
			if (m_ProfileArr[i].lStartTime.QuadPart != 0)
			{
				// 오류
				wprintf(L"Porfile Start Error\n");

				// Crash
				int* crash = nullptr;
				*crash = 0;
			}

			if (m_ProfileArr[i].isReset == true)
				m_ProfileArr[i].isReset = false;

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
	wprintf(L"Porfile Begin Error\n");

	// Crash - 배열이 부족한 경우 일로 올 수 있음.
	int* crash = nullptr;
	*crash = 0;
}

void CProfiler::End(const wchar_t* str)
{
	CSynchronizedSRWShared srwlockShared(&m_srwlock);

	LARGE_INTEGER endTime;
	QueryPerformanceCounter(&endTime);

	for (__int32 i = 0; i < MAX_PROFILE; i++)
	{
		if (m_ProfileArr[i].isFlag == true && wcscmp(m_ProfileArr[i].szName, str) == 0)
		{
			if (m_ProfileArr[i].isReset == true)
			{
				m_ProfileArr[i].isReset = false;
				return;
			}

			if (m_ProfileArr[i].lStartTime.QuadPart == 0)
			{
				// 오류
				wprintf(L"Porfile End Error\n");

				// Crash
				int* crash = nullptr;
				*crash = 0;
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
	wprintf(L"Porfile End Error\n");

	// Crash
	int* crash = nullptr;
	*crash = 0;
}

void CProfiler::OutText(const wchar_t* szFileName)
{
	CSynchronizedSRWExclusive srwlockShared(&m_srwlock);

	FILE* fp;
	if (_wfopen_s(&fp, szFileName, L"ab") != 0)
	{
		// 오류
		wprintf(L"Porfile File Open Error\n");

		// Crash
		int* crash = nullptr;
		*crash = 0;
	}

	wchar_t intro[3][150] = {
		L"---------------------------------------------------------------------------------------------------------------\n",
		L"        Name        |         Average         |          Min           |         Max            |     Call    | \n",
		L"---------------------------------------------------------------------------------------------------------------\n",
	};

	for (__int32 i = 0; i < 3; i++)
	{
		if(fwrite(intro[i], wcslen(intro[i]) * sizeof(wchar_t), 1, fp) != 1)
		{
			// 오류
			fclose(fp);
			wprintf(L"Profile File intro Write Error\n");

			// Crash
			int* crash = nullptr;
			*crash = 0;
		}
	}
	
	for (__int32 i = 0; i < MAX_PROFILE; i++)
	{
		if (m_ProfileArr[i].isFlag == true && m_ProfileArr[i].iCall != 0)
		{
			wchar_t tempBuf[1000];
			if (m_ProfileArr[i].iCall == 1)
			{
				swprintf_s(tempBuf, L"%20s | %20.4lf㎲ | %20.4lf㎲ | %20.4lf㎲ | %10lld\n", m_ProfileArr[i].szName,
					m_ProfileArr[i].iTotalTime / m_freq / (double)m_ProfileArr[i].iCall,
					m_ProfileArr[i].iMin[0] / m_freq,
					m_ProfileArr[i].iMax[0] / m_freq,
					m_ProfileArr[i].iCall);
			}
			else
			{
				swprintf_s(tempBuf, L"%20s | %20.4lf㎲ | %20.4lf㎲ | %20.4lf㎲ | %10lld\n", m_ProfileArr[i].szName,
					m_ProfileArr[i].iTotalTime / m_freq / (double)m_ProfileArr[i].iCall,
					((m_ProfileArr[i].iMin[0] + m_ProfileArr[i].iMin[1]) / 2.0) / m_freq,
					((m_ProfileArr[i].iMax[0] + m_ProfileArr[i].iMax[1]) / 2.0) / m_freq,
					m_ProfileArr[i].iCall);
			}

			if (fwrite(tempBuf, wcslen(tempBuf) * sizeof(wchar_t), 1, fp) != 1)
			{
				// 오류
				fclose(fp);
				wprintf(L"Porfile File Write Error\n");

				// Crash
				int* crash = nullptr;
				*crash = 0;
			}
		}
	}	

	fclose(fp);
}

void CProfiler::Reset(void)
{
	CSynchronizedSRWExclusive srwlockShared(&m_srwlock);

	for (__int32 i = 0; i < MAX_PROFILE; i++)
	{
		if (m_ProfileArr[i].isFlag == true)
		{
			m_ProfileArr[i].lStartTime.QuadPart = 0;
			m_ProfileArr[i].iTotalTime = 0;
			m_ProfileArr[i].iMax[0] = m_ProfileArr[i].iMax[1] = 0;
			m_ProfileArr[i].iMin[0] = m_ProfileArr[i].iMin[1] = 0;
			m_ProfileArr[i].iCall = 0;
			m_ProfileArr[i].isReset = true;
		}
	}
}

CProfilerManager::CProfilerManager()
{
	m_tlsIndex = TlsAlloc();
	if (m_tlsIndex == TLS_OUT_OF_INDEXES)
	{
		int* crash = nullptr;
		*crash = 0;
	}
}

CProfilerManager::~CProfilerManager()
{
	TlsFree(m_tlsIndex);
}

void CProfilerManager::Begin(const wchar_t* str)
{	
	CProfiler* pProfiler = (CProfiler*)TlsGetValue(m_tlsIndex);
	if (pProfiler == nullptr)
	{
		pProfiler = new CProfiler();
		TlsSetValue(m_tlsIndex, pProfiler);

		int i = 0;
		for (i = 0; i < MAX_THREAD_CNT; i++)
		{
			if (0 == InterlockedCompareExchange((unsigned long long*) & m_pProfilerArr[i],
				(unsigned long long)pProfiler, 0))
			{
				break;
			}
		}

		if (i == MAX_THREAD_CNT)
		{
			int* crash = nullptr;
			*crash = 0;
		}
	}

	pProfiler->Begin(str);
}

void CProfilerManager::End(const wchar_t* str)
{
	CProfiler* pProfiler = (CProfiler*)TlsGetValue(m_tlsIndex);
	if (pProfiler == nullptr)
	{
		int* crash = nullptr;
		*crash = 0;
	}

	pProfiler->End(str);
}

void CProfilerManager::OutText(const wchar_t* szFileName)
{
	for (int i = 0; i < MAX_THREAD_CNT; i++)
	{
		if (m_pProfilerArr[i] == nullptr)
			break;

		m_pProfilerArr[i]->OutText(szFileName);
	}
}

void CProfilerManager::Reset(void)
{
	for (int i = 0; i < MAX_THREAD_CNT; i++)
	{
		if (m_pProfilerArr[i] == nullptr)
			break;

		m_pProfilerArr[i]->Reset();
	}
}

CProfilerManager g_ProfilerMgr;

void ProfileBegin(const wchar_t* szName)
{
	g_ProfilerMgr.Begin(szName);
}

void ProfileEnd(const wchar_t* szName)
{
	g_ProfilerMgr.End(szName);
}

void ProfileDataOutText(const wchar_t* szFileName)
{
	g_ProfilerMgr.OutText(szFileName);
}

void ProfileReset(void)
{
	g_ProfilerMgr.Reset();
}