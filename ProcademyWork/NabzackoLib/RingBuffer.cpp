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
	if (iBufferSize == 0)
	{
		printf("RingBuffer Size Zero\n");
		iBufferSize = DEFAULT_SIZE;
	}

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