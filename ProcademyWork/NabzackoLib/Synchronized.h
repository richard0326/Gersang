#pragma once


// 자동으로 동기화
// Critical Section을 자동화
class CSynchronizedCS
{
public:
	CSynchronizedCS(CRITICAL_SECTION* cs);
	~CSynchronizedCS();

	// 수동으로 해제하고 싶은 경우 사용
	void Unlock();

private:
	LPCRITICAL_SECTION m_criticalSection;
	bool m_NotReleased = true;
};

// SRWShared를 자동화
class CSynchronizedSRWShared
{
public:
	CSynchronizedSRWShared(SRWLOCK* pSRWLock);
	~CSynchronizedSRWShared();

	// 수동으로 해제하고 싶은 경우 사용
	void Unlock();

private:
	SRWLOCK* m_pSRWLock;
	bool m_NotReleased = true;
};

// SRWExclusive를 자동화
class CSynchronizedSRWExclusive
{
public:
	CSynchronizedSRWExclusive(SRWLOCK* pSRWLock);
	~CSynchronizedSRWExclusive();

	// 수동으로 해제하고 싶은 경우 사용
	void Unlock();

private:
	SRWLOCK* m_pSRWLock;
	bool m_NotReleased = true;
};