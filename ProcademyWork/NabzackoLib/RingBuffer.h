#pragma once

class CRingBuffer
{
private:
	enum {
		DEFAULT_SIZE = 5000,
	};

public:

	CRingBuffer(void);
	CRingBuffer(int iBufferSize);
	~CRingBuffer(void);


	__forceinline int GetBufferSize(void)
	{
		return m_MaxSize;
	}

	__forceinline int GetUseSize(void)
	{
		char* Rear = m_RearPtr;
		char* Front = m_FrontPtr;
		if (Front > Rear)
		{
			return (m_RingEndPtr - Front) + (Rear - m_RingBuf);
		}
		return Rear - Front;
	}

	__forceinline int GetFreeSize(void)
	{
		char* Rear = m_RearPtr;
		char* Front = m_FrontPtr;
		if (Front == Rear)
		{
			return m_MaxSize;
		}
		else if (Front < Rear)
		{
			return m_MaxSize - (Rear - Front);
		}
		return Front - (Rear + 1);
	}

	__forceinline int DirectEnqueueSize(void)
	{
		char* Rear = m_RearPtr;
		char* Front = m_FrontPtr;
		if (Front > Rear)
		{
			return Front - (Rear + 1);
		}
		else if (Front == m_RingBuf) {
			return m_RingEndPtr - (Rear + 1);
		}
		return m_RingEndPtr - Rear;
	}

	__forceinline int DirectEnqueueSize(int* usedSize)
	{
		char* Rear = m_RearPtr;
		char* Front = m_FrontPtr;
		*usedSize = 0;
		if (Front > Rear)
		{
			//*usedSize = (m_RingEndPtr - Front) + (Rear - m_RingBuf);
			return Front - (Rear + 1);
		}
		else if (Front == m_RingBuf) {
			return m_RingEndPtr - (Rear + 1);
		}
		*usedSize = (Front - 1) - m_RingBuf;
		return m_RingEndPtr - Rear;
	}

	__forceinline int DirectDequeueSize(void)
	{
		char* Rear = m_RearPtr;
		char* Front = m_FrontPtr;
		if (Front > Rear) {
			return m_RingEndPtr - Front;
		}
		return Rear - Front;
	}

	__forceinline int DirectDequeueSize(int* usedSize)
	{
		char* Rear = m_RearPtr;
		char* Front = m_FrontPtr;
		if (Front > Rear) {
			*usedSize = (m_RingEndPtr - Front) + (Rear - m_RingBuf);
			return m_RingEndPtr - Front;
		}
		*usedSize = Rear - Front;
		return Rear - Front;
	}

	__forceinline int Enqueue(char* chpData, int iSize)
	{
		char* Rear = m_RearPtr;
		char* Front = m_FrontPtr;
		if ((Rear + 1 == Front) || ((Rear + 1) - m_RealSize == Front))
			return 0;

		char* cpyPtr = Rear;

		for (int i = 0; i < iSize; ++i)
		{
			*cpyPtr = chpData[i];
			cpyPtr = Rear + i + 1;
			if (cpyPtr >= m_RingEndPtr) {
				cpyPtr -= m_RealSize;
			}

			if ((cpyPtr + 1 == Front) || ((cpyPtr + 1) - m_RealSize == Front))
			{
				iSize = i + 1;
				break;
			}
		}

		m_RearPtr = cpyPtr;

		return iSize;
	}

	__forceinline int Dequeue(char* chpDest, int iSize)
	{
		char* Rear = m_RearPtr;
		char* Front = m_FrontPtr;
		if (Front == Rear)
			return 0;

		int dequeueSize = iSize;

		char* cpyPtr = Front;
		for (int i = 0; i < iSize; ++i)
		{
			chpDest[i] = *cpyPtr;
			cpyPtr = Front + i + 1;
			if (cpyPtr >= m_RingEndPtr) {
				cpyPtr -= m_RealSize;
			}

			if (cpyPtr == Rear) {
				dequeueSize = i + 1;
				break;
			}
		}

		m_FrontPtr = cpyPtr;

		return dequeueSize;
	}

	__forceinline int Peek(char* chpDest, int iSize)
	{
		char* Rear = m_RearPtr;
		char* Front = m_FrontPtr;
		if (Front == Rear)
			return 0;

		int cpySize = iSize;
		char* cpyPtr = Front;
		for (int i = 0; i < iSize; ++i)
		{
			chpDest[i] = *cpyPtr;
			cpyPtr = Front + i + 1;
			if (cpyPtr >= m_RingEndPtr) {
				cpyPtr -= m_RealSize;
			}

			if (cpyPtr == Rear) {
				cpySize = i + 1;
				break;
			}
		}

		return cpySize;
	}

	__forceinline bool MoveRear(int iSize)
	{
		if (iSize == 0)
			return false;

		char* Front = m_FrontPtr - 1;
		char* Rear = m_RearPtr;
		char* cmpPtr = m_RearPtr + iSize;

		if (Rear < Front)
		{
			if (Front < cmpPtr)
				return false;
		}
		else
		{
			if (cmpPtr >= m_RingEndPtr) {
				cmpPtr -= m_RealSize;
				if (cmpPtr > Front)
					return false;
			}
		}

		m_RearPtr = cmpPtr;
		return true;
	}

	__forceinline bool MoveFront(int iSize)
	{
		if (iSize == 0)
			return false;

		char* Front = m_FrontPtr;
		char* Rear = m_RearPtr;
		char* cmpPtr = Front + iSize;

		if (Front < Rear)
		{
			if (Rear < cmpPtr)
				return false;
		}
		else
		{
			if (cmpPtr >= m_RingEndPtr) {
				cmpPtr -= m_RealSize;
				if (cmpPtr > Rear)
					return false;
			}
		}

		m_FrontPtr = cmpPtr;

		return true;
	}

	__forceinline void ClearBuffer(void)
	{
		m_RearPtr = m_FrontPtr = m_RingBuf;
	}

	__forceinline char* GetFrontBufferPtr(void)
	{
		return m_FrontPtr;
	}

	__forceinline char* GetRearBufferPtr(void)
	{
		return m_RearPtr;
	}

	__forceinline char* GetBufferPtr(void)
	{
		return m_RingBuf;
	}

private:
	// 큐의 최대 크기
	int		m_MaxSize;
	
	// 링버퍼 배열
	char*	m_RingBuf;
	char*	m_RingEndPtr;

	alignas(64) int		m_RealSize;
	// 데이터의 시작과 마지막 포인터
	char*	m_FrontPtr;
	char*	m_RearPtr;
};