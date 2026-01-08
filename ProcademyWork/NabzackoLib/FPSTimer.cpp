#include "stdafx.h"
#include "FPSTimer.h"
#include <time.h>

DECLARE_SINGLETON_IN_CPP(CFPSTimer);

// QueryPerformanceTimer를 사용하고 싶을때 1로 설정해주시면 됩니다.
#if 1
#define QUERYPERFORMANCETIMER_ON
#endif

#ifndef QUERYPERFORMANCETIMER_ON
// timeGetTime() 에서 필요한 라이브러리
#include <mmsystem.h>
#pragma comment(lib, "Winmm.lib")

#endif

/**
 * 1 QueryPerformanceFrequency : 100 nano sec
 * 1 timeGetTime : 1 Milli Sec
 * 
 * 둘의 시간 단위가 다르기 때문에 알맞은 단위로 변환해주기 위한 상수
 */
constexpr __int64 QPF2MS = 10'000; // QueryPerformanceFrequency To MilliSec
constexpr __int64 MS2QPF = 10'000; // MilliSec To QueryPerformanceFrequency

CFPSTimer::CFPSTimer()
	: m_Prev(0)
	, m_freq(0)
	, m_Frame(0)
	, m_FramePerSecond(0)
	, m_PrevSec(0)
	, m_countLogicFPS(0)
	, m_PreRenderCountFPS(0)
	, m_PreLogicCountFPS(0)
	, m_PreLoopCountFPS(0)
	, m_accumulateTime(0)
	, m_countRenderFPS(0)
	, m_countLoopFPS(0)
	, m_SecCheck(false)
{

}

CFPSTimer::~CFPSTimer()
{

}

bool CFPSTimer::Init(__int64 FramePerSecond)
{
	// 초기 FPS 값
	m_FramePerSecond = FramePerSecond;

#ifdef QUERYPERFORMANCETIMER_ON
	// 1 s의 기준이 되는 frequency
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	m_freq = (__int64)freq.QuadPart;

	LARGE_INTEGER prev;
	QueryPerformanceCounter(&prev);

	// 비교할때 사용될 변수
	m_PrevSec = m_Prev = (__int64)prev.QuadPart;

#else
	// 1 ms 단위로 시간을 측정하기 위해 사용합니다.
	timeBeginPeriod(1);
	
	// 나머지 연산에서 너무 많은 값이 손실되지 않도록 큰 값을 곱해줍니다.
	m_freq = (__int64)1000 * MS2QPF;
	m_PrevSec = m_Prev = (__int64)timeGetTime() * MS2QPF;
	
#endif
	// frame per Sec : 1 프레임
	m_Frame = (m_freq / m_FramePerSecond);
	if (m_Frame > m_freq || m_Frame <= 0)
	{
		return FALSE;
	}

	return TRUE;
}


void CFPSTimer::Release()
{
#ifndef QUERYPERFORMANCETIMER_ON
	// 1 ms 단위의 system timer 시간을 해제합니다.
	timeEndPeriod(1);
#endif
}

bool CFPSTimer::CheckFrame()
{
#ifdef QUERYPERFORMANCETIMER_ON
	// 프레임 시간에 도달했는지 체크
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);

	// 1초에 몇 프레임 진행되는지 체크
	__int64 cmpSec = (__int64)now.QuadPart - m_PrevSec;
	__int64 cmp = (__int64)now.QuadPart - m_Prev;
#else
	__int64 now = (__int64)timeGetTime() * MS2QPF;

	// 1초에 몇 프레임 진행되는지 체크
	__int64 cmpSec = now - m_PrevSec;
	__int64 cmp = now - m_Prev;
#endif
	// 1초를 지난 경우
	if (cmpSec >= m_freq)
	{
		__int64 times = cmpSec / m_freq;
		m_PrevSec += m_freq * times;
		m_PreLogicCountFPS = m_countLogicFPS;
		m_countLogicFPS = 0;
		m_PreLoopCountFPS = m_countLoopFPS;
		m_countLoopFPS = 0;
		m_SecCheck = true;
	}

	++m_countLoopFPS;

	if (cmp < m_Frame)
	{
		return false;
	}

	// 로직에 대한 횟수
	++m_countLogicFPS;
	// 이전 프레임 구하기
	m_Prev += m_Frame;

	return true;
}

/**
 * DontSkipRendering() : "출력을 스킵하지 마세요!" 함수
 */
bool CFPSTimer::DontSkipRendering()
{	
#ifdef QUERYPERFORMANCETIMER_ON
	// 프레임 시간에 도달했는지 체크
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);

	// 1초에 몇 프레임 진행되는지 체크
	__int64 cmpSec = (__int64)now.QuadPart - m_PrevSec;
	__int64 cmp = (__int64)now.QuadPart - m_Prev;
#else
	__int64 now = (__int64)timeGetTime() * MS2QPF;

	// 1초에 몇 프레임 진행되는지 체크
	__int64 cmpSec = now - m_PrevSec;
	__int64 cmp = now - m_Prev;
#endif
	// 1초를 지난 경우
	if (cmpSec >= m_freq)
	{
		__int64 times = cmpSec / m_freq;
		m_PrevSec += m_freq * times;
		m_PreLogicCountFPS = m_countLogicFPS;
		m_countLogicFPS = 0;
		m_PreRenderCountFPS = m_countRenderFPS;
		m_countRenderFPS = 0;
		m_SecCheck = true;
	}

	// 로직에 대한 횟수
	++m_countLogicFPS;
	// 이전 프레임 구하기
	m_Prev += m_Frame;

	// 누적 시간이 프레임 보다 큰 경우
	if (m_accumulateTime > m_Frame)
	{
		m_accumulateTime -= m_Frame;
		return false;
	}
	// delta time이 프레임 보다 크거나 같은 경우
	else if (cmp >= m_Frame) {
		m_accumulateTime += cmp - m_Frame;
	}
	// 1 frame 보다 작은 경우, 남은 frame 만큼 Sleep을 진행합니다.
	else {
		Sleep((m_Frame - cmp) / QPF2MS);
	}

	// 출력에 대한 횟수
	++m_countRenderFPS;
	return true;
}

__int64 CFPSTimer::GetCurrentLogicFPS()
{
	return m_PreLogicCountFPS;
}

__int64 CFPSTimer::GetCurrentRenderFPS()
{
	return m_PreRenderCountFPS;
}

__int64 CFPSTimer::GetCurrentLoopFPS()
{
	return m_PreLoopCountFPS;
}

bool CFPSTimer::CheckSec()
{
	if (m_SecCheck)
	{
		m_SecCheck = false;
		return true;
	}
	return false;
}