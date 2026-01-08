#include "stdafx.h"
#include "Synchronized.h"

CSynchronizedCS::CSynchronizedCS(CRITICAL_SECTION* cs)
	: m_criticalSection(cs)
{
	EnterCriticalSection(m_criticalSection);
}

CSynchronizedCS::~CSynchronizedCS()
{
	if (m_NotReleased)
	{
		LeaveCriticalSection(m_criticalSection);
	}
}

void CSynchronizedCS::Unlock()
{
	m_NotReleased = false;
	LeaveCriticalSection(m_criticalSection);
}

CSynchronizedSRWShared::CSynchronizedSRWShared(SRWLOCK* pSRWLock)
	: m_pSRWLock(pSRWLock)
{
	AcquireSRWLockShared(m_pSRWLock);
}

CSynchronizedSRWShared::~CSynchronizedSRWShared()
{
	if (m_NotReleased)
	{
		ReleaseSRWLockShared(m_pSRWLock);
	}
}

void CSynchronizedSRWShared::Unlock()
{
	m_NotReleased = false;
	ReleaseSRWLockShared(m_pSRWLock);
}

CSynchronizedSRWExclusive::CSynchronizedSRWExclusive(SRWLOCK* pSRWLock)
	: m_pSRWLock(pSRWLock)
{
	AcquireSRWLockExclusive(m_pSRWLock);
}

CSynchronizedSRWExclusive::~CSynchronizedSRWExclusive()
{
	if (m_NotReleased)
	{
		ReleaseSRWLockExclusive(m_pSRWLock);
	}
}

void CSynchronizedSRWExclusive::Unlock()
{
	m_NotReleased = false;
	ReleaseSRWLockExclusive(m_pSRWLock);
}