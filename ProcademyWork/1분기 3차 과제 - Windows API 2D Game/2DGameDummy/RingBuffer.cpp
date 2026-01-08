#include "stdafx.h"
#include "RingBuffer.h"
#include <iostream>

CRingBuffer::CRingBuffer(void)
{
	m_MaxSize = DEFAULT_SIZE;

	m_RingBuf = new char[m_MaxSize];

	m_RearPtr = m_FrontPtr = m_RingBuf;
	m_RingEndPtr = m_RingBuf + m_MaxSize;
	m_InputSize = 0;
}

CRingBuffer::CRingBuffer(int iBufferSize)
{
	m_MaxSize = iBufferSize;

	m_RingBuf = new char[m_MaxSize];

	m_RearPtr = m_FrontPtr = m_RingBuf;
	m_RingEndPtr = m_RingBuf + m_MaxSize;
	m_InputSize = 0;
}

CRingBuffer::~CRingBuffer(void)
{
	delete[] m_RingBuf;
}

void CRingBuffer::Resize(int size)
{
	// size를 늘리는 경우에만 허용...
	if (m_MaxSize >= size) {
		return;
	}

	m_MaxSize = size;
	char* tempBuf = new char[m_MaxSize];
	if (m_FrontPtr + m_InputSize > m_RingEndPtr)
	{
		int frontSize = m_RingEndPtr - m_FrontPtr;
		memcpy(tempBuf, m_FrontPtr, frontSize);
		memcpy(tempBuf + frontSize, m_RingBuf, m_InputSize - frontSize);
	}
	else {
		memcpy(tempBuf, m_FrontPtr, m_InputSize);
	}

	delete[] m_RingBuf;
	m_FrontPtr = m_RingBuf = tempBuf;
	m_RingEndPtr = m_RingBuf + m_MaxSize;
	m_RearPtr = m_RingBuf + m_InputSize;
}

int CRingBuffer::GetBufferSize(void)
{
	return m_MaxSize;
}

int CRingBuffer::GetUseSize(void)
{
	return m_InputSize;
}

int CRingBuffer::GetFreeSize(void)
{
	return m_MaxSize - m_InputSize;
}

int CRingBuffer::DirectEnqueueSize(void)
{
	int directEnqueueSize = 0;
	if (m_FrontPtr == m_RearPtr && m_InputSize == m_MaxSize) {
		directEnqueueSize = 0;
	}
	else if (m_FrontPtr > m_RearPtr)
	{
		directEnqueueSize = m_FrontPtr - m_RearPtr;
	}
	else {
		directEnqueueSize = m_RingEndPtr - m_RearPtr;
	}
	return directEnqueueSize;
}

int CRingBuffer::DirectDequeueSize(void)
{
	int directDequeueSize = 0;
	if (m_FrontPtr > m_RearPtr) {
		directDequeueSize = m_RingEndPtr - m_FrontPtr;
	}
	else {
		directDequeueSize = m_InputSize;
	}
	return directDequeueSize;
}

int CRingBuffer::Enqueue(char* chpData, int iSize)
{
	if (iSize > m_MaxSize - m_InputSize) {
		Resize(m_InputSize + iSize);
		memcpy(m_RearPtr, chpData, iSize);
		m_RearPtr += iSize;
	}
	else {
		// F <= R 인 경우
		if (m_FrontPtr <= m_RearPtr) {
			// 링버퍼의 크기를 초과하는 경우
			if (m_RearPtr + iSize > m_RingEndPtr) {
				int frontSize = m_RingEndPtr - m_RearPtr;
				memcpy(m_RearPtr, chpData, frontSize);
				memcpy(m_RingBuf, chpData + frontSize, iSize - frontSize);
				m_RearPtr += iSize - m_MaxSize;
			}
			// 정상적인 크기로 들어오는 경우
			else {
				memcpy(m_RearPtr, chpData, iSize);
				m_RearPtr += iSize;
			}
		}
		// R < F 인 경우
		else {
			memcpy(m_RearPtr, chpData, iSize);
			m_RearPtr += iSize;
		}
	}

	if (m_RearPtr >= m_RingEndPtr) {
		m_RearPtr -= m_MaxSize;
	}

	m_InputSize += iSize;
	return iSize;
}

int CRingBuffer::Dequeue(char* chpDest, int iSize)
{
	int dequeueSize = iSize;
	if (iSize > m_InputSize) {
		if (m_FrontPtr + m_InputSize > m_RingEndPtr) {
			int frontSize = m_RingEndPtr - m_FrontPtr;
			memcpy(chpDest, m_FrontPtr, frontSize);
			memcpy(chpDest + frontSize, m_RingBuf, m_InputSize - frontSize);
			m_FrontPtr += m_InputSize - m_MaxSize;
		}
		else {
			memcpy(chpDest, m_FrontPtr, m_InputSize);
			m_FrontPtr += m_InputSize;
		}

		dequeueSize = m_InputSize;
		m_InputSize = 0;
	}
	else {
		if (m_FrontPtr < m_RearPtr) {
			memcpy(chpDest, m_FrontPtr, iSize);
			m_FrontPtr += iSize;
		}
		else {
			if (m_FrontPtr + iSize > m_RingEndPtr) {
				int frontSize = m_RingEndPtr - m_FrontPtr;
				memcpy(chpDest, m_FrontPtr, frontSize);
				memcpy(chpDest + frontSize, m_RingBuf, iSize - frontSize);
				m_FrontPtr += iSize - m_MaxSize;
			}
			else {
				memcpy(chpDest, m_FrontPtr, iSize);
				m_FrontPtr += iSize;
			}
		}
		m_InputSize -= iSize;
	}

	if (m_FrontPtr >= m_RingEndPtr) {
		m_FrontPtr -= m_MaxSize;
	}

	return dequeueSize;
}

int CRingBuffer::Peek(char* chpDest, int iSize)
{
	int cpySize = iSize;
	if (cpySize > m_InputSize) {
		cpySize = m_InputSize;
	}

	if (m_FrontPtr + cpySize > m_RingEndPtr) {
		int frontSize = m_RingEndPtr - m_FrontPtr;
		memcpy(chpDest, m_FrontPtr, frontSize);
		memcpy(chpDest + frontSize, m_RingBuf, cpySize - frontSize);
	}
	else {
		memcpy(chpDest, m_FrontPtr, cpySize);
	}
	return cpySize;
}

bool CRingBuffer::MoveRear(int iSize)
{
	if (iSize > m_MaxSize - m_InputSize) {
		return false;
	}

	m_RearPtr += iSize;
	if (m_RearPtr >= m_RingEndPtr) {
		m_RearPtr -= m_MaxSize;
	}
	m_InputSize += iSize;

	return true;
}

bool CRingBuffer::MoveFront(int iSize)
{
	if (iSize > m_InputSize) {
		return false;
	}

	m_FrontPtr += iSize;
	if (m_FrontPtr >= m_RingEndPtr) {
		m_FrontPtr -= m_MaxSize;
	}
	m_InputSize -= iSize;

	return true;
}

void CRingBuffer::ClearBuffer(void)
{
	m_RearPtr = m_FrontPtr = m_RingBuf;
	m_InputSize = 0;
}

char* CRingBuffer::GetFrontBufferPtr(void)
{
	return m_FrontPtr;
}

char* CRingBuffer::GetRearBufferPtr(void)
{
	return m_RearPtr;
}