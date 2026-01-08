#include "stdafx.h"
#include "Packet.h"


CPacket::CPacket()
{
	ZeroMemory(&m_stHeader, sizeof(m_stHeader));
	m_stHeader.byCode = dfPACKET_CODE;
}

CPacket::~CPacket()
{

}

char* CPacket::GetHeaderPtr()
{
	return (char*)&m_stHeader;
}

int		CPacket::GetHeaderSize()
{
	return sizeof(m_stHeader);
}

void	CPacket::SetMsgType(short msgType)
{
	m_stHeader.wMsgType = msgType;
}

short	CPacket::GetMsgType()
{
	return m_stHeader.wMsgType;
}

short	CPacket::GetPayloadSize()
{
	return m_stHeader.wPayloadSize;
}

void	CPacket::FullPacked()
{
	char* pCheckSum = (char*)&m_stHeader.wMsgType;
	int checkSum = (*pCheckSum) + (*(pCheckSum + 1));
	unsigned char* bufferPtr = (unsigned char*)m_chpBuffer;

	for (int i = 0; i < m_iDataSize; ++i)
	{
		checkSum += bufferPtr[i];
	}

	m_stHeader.byCheckSum = checkSum % 256;

	m_stHeader.wPayloadSize = m_iDataSize;
}

bool	CPacket::CheckPacking()
{
	if (m_stHeader.byCode != dfPACKET_CODE)
		return false;

	char* pCheckSum = (char*)&m_stHeader.wMsgType;
	int checkSum = (*pCheckSum) + (*(pCheckSum + 1));
	unsigned char* bufferPtr = (unsigned char*)m_chpBuffer;

	for (int i = 0; i < m_iDataSize; ++i)
	{
		checkSum += bufferPtr[i];
	}
	checkSum %= 256;

	if (checkSum != m_stHeader.byCheckSum || m_stHeader.wPayloadSize != m_iDataSize)
	{
		return false;
	}

	return true;
}