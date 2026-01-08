#include "stdafx.h"
#include "Parser.h"
#include "LockFreeTlsPoolA.h"
#include "SerializeBuffer.h"

CLockFreeTlsPoolA<CSerializeBuffer>* CSerializeBuffer::m_tlsPool = nullptr;
int CSerializeBuffer::m_iMaxBufferSize = 0;

CSerializeBuffer::CSerializeBuffer()
{
	m_HeaderBuffer = new char[m_iMaxBufferSize];
}

CSerializeBuffer::~CSerializeBuffer()
{
	delete m_HeaderBuffer;
}

bool CSerializeBuffer::Init()
{
	static bool isInit = false;
	if (isInit == true)
	{
		int* crash = nullptr;
		*crash = 0;
	}
	isInit = true;

	CParser packetParser;

	wchar_t* errMsg = nullptr;
	if (packetParser.ReadBuffer(L"packetInfo.txt", &errMsg) == false)
		return false;

	int PACKET_CHUNK = 0;
	if (packetParser.GetValue(L"Packet", L"PACKET_CHUNK", &PACKET_CHUNK) == false)
		return false;

	int PACKET_NODE = 0;
	if (packetParser.GetValue(L"Packet", L"PACKET_NODE", &PACKET_NODE) == false)
		return false;

	int BUFFER_SIZE = 0;
	if (packetParser.GetValue(L"Packet", L"BUFFER_SIZE", &BUFFER_SIZE) == false)
		return false;

	m_iMaxBufferSize = BUFFER_SIZE;
	if (m_iMaxBufferSize == 0)
	{
		int* crash = nullptr;
		*crash = 0;
	}
	m_tlsPool = new CLockFreeTlsPoolA<CSerializeBuffer>(PACKET_CHUNK, PACKET_NODE, false, true);
}

void CSerializeBuffer::Reset()
{
	// 추가 예정
}

#ifdef MY_DEBUG
void CSerializeBuffer::SetLog(char pos, unsigned long long sendID)
{
	short idx = (InterlockedIncrement16(&m_logCount) - 1) % 16;

	// 브로드캐스팅 되는 경우...
	m_logPacket[idx] = pos;
	m_sendID[idx] = sendID;
}
#endif

void CSerializeBuffer::DecreaseRefCount()
{
#ifdef MY_DEBUG
	long refCount = InterlockedDecrement((LONG*)&m_RefCount);
	if (0 == refCount)
	{
		m_tlsPool->Free(this);
	}
	else if (refCount < 0)
	{
		int* crash = nullptr;
		*crash = 0;
	}
#else
	if (0 == InterlockedDecrement((LONG*)&m_RefCount))
	{
		m_tlsPool->Free(this);
	}
#endif // MY_DEBUG
}