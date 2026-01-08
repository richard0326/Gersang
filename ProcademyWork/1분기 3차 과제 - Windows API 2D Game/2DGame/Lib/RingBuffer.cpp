#include "stdafx.h"
#include "RingBuffer.h"
#include <iostream>

CRingBuffer::CRingBuffer(void)
{
	m_MaxSize = DEFAULT_SIZE;
	m_RealSize = m_MaxSize + 1;

	m_RingBuf = new char[m_RealSize];

	m_RearPtr = m_FrontPtr = m_RingBuf;
	m_RingEndPtr = m_RingBuf + m_RealSize;
}

CRingBuffer::CRingBuffer(int iBufferSize)
{
	m_MaxSize = iBufferSize;
	m_RealSize = m_MaxSize + 1;

	m_RingBuf = new char[m_RealSize];

	m_RearPtr = m_FrontPtr = m_RingBuf;
	m_RingEndPtr = m_RingBuf + m_RealSize;
}

CRingBuffer::~CRingBuffer(void)
{
	delete[] m_RingBuf;
}

int CRingBuffer::GetBufferSize(void)
{
	return m_MaxSize;
}

int CRingBuffer::GetUseSize(void)
{
	char* Rear = m_RearPtr;
	char* Front = m_FrontPtr;
	if (Front > Rear)
	{
		return (m_RingEndPtr - Front) + (Rear - m_RingBuf);
	}
	return Rear - Front;
}

int CRingBuffer::GetFreeSize(void)
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

int CRingBuffer::DirectEnqueueSize(void)
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

int CRingBuffer::DirectDequeueSize(void)
{
	char* Rear = m_RearPtr;
	char* Front = m_FrontPtr;
	if (Front > Rear) {
		return m_RingEndPtr - Front;
	}
	return Rear - Front;
}

int CRingBuffer::Enqueue(char* chpData, int iSize)
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

int CRingBuffer::Dequeue(char* chpDest, int iSize)
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

int CRingBuffer::Peek(char* chpDest, int iSize)
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

bool CRingBuffer::MoveRear(int iSize)
{
	if (iSize == 0)
		return true;

	char* cmpPtr = m_RearPtr + iSize;
	if (cmpPtr >= m_RingEndPtr) {
		cmpPtr -= m_RealSize;
	}

	m_RearPtr = cmpPtr;
	return true;
}

bool CRingBuffer::MoveFront(int iSize)
{
	if (iSize == 0)
		return true;

	char* cmpPtr = m_FrontPtr + iSize;
	if (cmpPtr >= m_RingEndPtr) {
		cmpPtr -= m_RealSize;
	}

	m_FrontPtr = cmpPtr;

	return true;
}

void CRingBuffer::ClearBuffer(void)
{
	m_RearPtr = m_FrontPtr = m_RingBuf;
}

char* CRingBuffer::GetFrontBufferPtr(void)
{
	return m_FrontPtr;
}

char* CRingBuffer::GetRearBufferPtr(void)
{
	return m_RearPtr;
}