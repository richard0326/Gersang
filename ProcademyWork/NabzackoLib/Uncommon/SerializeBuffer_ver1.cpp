#include "stdafx.h"
#include "SerializeBuffer.h"

CSerializeBuffer::CSerializeBuffer()
{
	m_iBufferSize = eBUFFER_DEFAULT;

	m_iDataSize = 0;

	m_chpBuffer = new char[m_iBufferSize];
	m_FrontPtr = m_BackPtr = m_chpBuffer;
}

CSerializeBuffer::CSerializeBuffer(int iBufferSize)
{
	m_iBufferSize = iBufferSize;

	m_iDataSize = 0;

	m_chpBuffer = new char[m_iBufferSize];
	m_FrontPtr = m_BackPtr = m_chpBuffer;
}

CSerializeBuffer::~CSerializeBuffer()
{
	delete[] m_chpBuffer;
}

//////////////////////////////////////////////////////////////////////////
// 패킷 청소.
//
// Parameters: 없음.
// Return: 없음.
//////////////////////////////////////////////////////////////////////////
void	CSerializeBuffer::Clear(void)
{
	m_iDataSize = 0;
	m_FrontPtr = m_BackPtr = m_chpBuffer;
}

//////////////////////////////////////////////////////////////////////////
// 버퍼 사이즈 얻기.
//
// Parameters: 없음.
// Return: (int)패킷 버퍼 사이즈 얻기.
//////////////////////////////////////////////////////////////////////////
int	CSerializeBuffer::GetBufferSize(void)
{
	return m_iBufferSize;
}

//////////////////////////////////////////////////////////////////////////
// 현재 사용중인 사이즈 얻기.
//
// Parameters: 없음.
// Return: (int)사용중인 데이타 사이즈.
//////////////////////////////////////////////////////////////////////////
int		CSerializeBuffer::GetDataSize(void)
{
	return m_iDataSize;
}

//////////////////////////////////////////////////////////////////////////
// 버퍼 포인터 얻기.
//
// Parameters: 없음.
// Return: (char *)버퍼 포인터.
//////////////////////////////////////////////////////////////////////////
char* CSerializeBuffer::GetBufferPtr(void)
{
	return m_chpBuffer;
}

//////////////////////////////////////////////////////////////////////////
// 버퍼 Pos 이동. (음수이동은 안됨)
// GetBufferPtr 함수를 이용하여 외부에서 강제로 버퍼 내용을 수정할 경우 사용. 
//
// Parameters: (int) 이동 사이즈.
// Return: (int) 이동된 사이즈.
//////////////////////////////////////////////////////////////////////////
int		CSerializeBuffer::MoveWritePos(int iSize)
{
	if (iSize <= 0 || m_iDataSize + iSize > m_iBufferSize) {
		throw CSerializeBufException(L"MoveWritePos err\n");
	}

	m_BackPtr += iSize;
	m_iDataSize += iSize;

	return iSize;
}

int		CSerializeBuffer::MoveReadPos(int iSize)
{
	if (iSize <= 0 || iSize > m_iDataSize) {
		throw CSerializeBufException(L"MoveReadPos err\n");
	}

	m_FrontPtr += iSize;
	m_iDataSize -= iSize;

	return iSize;
}

/* ============================================================================= */
// 연산자 오버로딩
/* ============================================================================= */
CSerializeBuffer& CSerializeBuffer::operator = (CSerializeBuffer& clSrcPacket)
{
	delete[] m_chpBuffer;

	m_iBufferSize = clSrcPacket.m_iBufferSize;
	m_iDataSize = clSrcPacket.m_iDataSize;

	m_chpBuffer = new char[m_iBufferSize];
	for (int copyLoop = 0; copyLoop < m_iBufferSize; ++copyLoop)
	{
		m_chpBuffer[copyLoop] = clSrcPacket.m_chpBuffer[copyLoop];
	}
	m_FrontPtr = clSrcPacket.m_FrontPtr;
	m_BackPtr = clSrcPacket.m_BackPtr;

	return *this;
}

//////////////////////////////////////////////////////////////////////////
// 넣기.	각 변수 타입마다 모두 만듬.
//////////////////////////////////////////////////////////////////////////
CSerializeBuffer& CSerializeBuffer::operator << (unsigned char uchValue)
{
	if (m_iBufferSize - m_iDataSize < sizeof(uchValue)) {
		throw CSerializeBufException(L"operator << (unsigned char) error\n");
	}

	*(unsigned char*)m_BackPtr = uchValue;
	m_BackPtr += sizeof(unsigned char);
	m_iDataSize += sizeof(unsigned char);
	return *this;
}

CSerializeBuffer& CSerializeBuffer::operator << (char chValue)
{
	if (m_iBufferSize - m_iDataSize < sizeof(chValue)) {
		throw CSerializeBufException(L"operator << (char) error\n");
	}

	*(char*)m_BackPtr = chValue;
	m_BackPtr += sizeof(char);
	m_iDataSize += sizeof(char);
	return *this;
}

CSerializeBuffer& CSerializeBuffer::operator << (unsigned short ushValue)
{
	if (m_iBufferSize - m_iDataSize < sizeof(ushValue)) {
		throw CSerializeBufException(L"operator << (unsigned short) error\n");
	}

	*(unsigned short*)m_BackPtr = ushValue;
	m_BackPtr += sizeof(unsigned short);
	m_iDataSize += sizeof(unsigned short);
	return *this;
}

CSerializeBuffer& CSerializeBuffer::operator << (short shValue)
{
	if (m_iBufferSize - m_iDataSize < sizeof(shValue)) {
		throw CSerializeBufException(L"operator << (short) error\n");
	}

	*(short*)m_BackPtr = shValue;
	m_BackPtr += sizeof(short);
	m_iDataSize += sizeof(short);
	return *this;
}

CSerializeBuffer& CSerializeBuffer::operator << (unsigned int uintValue)
{
	if (m_iBufferSize - m_iDataSize < sizeof(uintValue)) {
		throw CSerializeBufException(L"operator << (unsigned int) error\n");
	}

	*(unsigned int*)m_BackPtr = uintValue;
	m_BackPtr += sizeof(unsigned int);
	m_iDataSize += sizeof(unsigned int);
	return *this;
}

CSerializeBuffer& CSerializeBuffer::operator << (unsigned long uintValue)
{
	if (m_iBufferSize - m_iDataSize < sizeof(uintValue)) {
		throw CSerializeBufException(L"operator << (unsigned int) error\n");
	}

	*(unsigned long*)m_BackPtr = uintValue;
	m_BackPtr += sizeof(unsigned long);
	m_iDataSize += sizeof(unsigned long);
	return *this;
}

CSerializeBuffer& CSerializeBuffer::operator << (int iValue)
{
	if (m_iBufferSize - m_iDataSize < sizeof(iValue)) {
		throw CSerializeBufException(L"operator << (int) error\n");
	}

	*(int*)m_BackPtr = iValue;
	m_BackPtr += sizeof(int);
	m_iDataSize += sizeof(int);
	return *this;
}

CSerializeBuffer& CSerializeBuffer::operator << (float fValue)
{
	if (m_iBufferSize - m_iDataSize < sizeof(fValue)) {
		throw CSerializeBufException(L"operator << (float) error\n");
	}

	*(float*)m_BackPtr = fValue;
	m_BackPtr += sizeof(float);
	m_iDataSize += sizeof(float);
	return *this;
}

CSerializeBuffer& CSerializeBuffer::operator << (unsigned __int64 uint64Value)
{
	if (m_iBufferSize - m_iDataSize < sizeof(uint64Value)) {
		throw CSerializeBufException(L"operator << (unsigned __int64) error\n");
	}

	*(unsigned __int64*)m_BackPtr = uint64Value;
	m_BackPtr += sizeof(unsigned __int64);
	m_iDataSize += sizeof(unsigned __int64);
	return *this;
}

CSerializeBuffer& CSerializeBuffer::operator << (__int64 int64Value)
{
	if (m_iBufferSize - m_iDataSize < sizeof(int64Value)) {
		throw CSerializeBufException(L"operator << (__int64) error\n");
	}

	*(__int64*)m_BackPtr = int64Value;
	m_BackPtr += sizeof(__int64);
	m_iDataSize += sizeof(__int64);
	return *this;
}

CSerializeBuffer& CSerializeBuffer::operator << (double dValue)
{
	if (m_iBufferSize - m_iDataSize < sizeof(dValue)) {
		throw CSerializeBufException(L"operator << (double) error\n");
		return *this;
	}

	*(double*)m_BackPtr = dValue;
	m_BackPtr += sizeof(double);
	m_iDataSize += sizeof(double);
	return *this;
}

//////////////////////////////////////////////////////////////////////////
// 빼기.	각 변수 타입마다 모두 만듬.
//////////////////////////////////////////////////////////////////////////
CSerializeBuffer& CSerializeBuffer::operator >> (unsigned char& uchValue)
{
	if (m_iDataSize < sizeof(uchValue)) {
		throw CSerializeBufException(L"operator >> (unsigned char) error\n");
	}

	uchValue = *(unsigned char*)m_FrontPtr;
	m_FrontPtr += sizeof(unsigned char);
	m_iDataSize -= sizeof(unsigned char);
	return *this;
}

CSerializeBuffer& CSerializeBuffer::operator >> (char& chValue)
{
	if (m_iDataSize < sizeof(chValue)) {
		throw CSerializeBufException(L"operator >> (char) error\n");
	}

	chValue = *(char*)m_FrontPtr;
	m_FrontPtr += sizeof(char);
	m_iDataSize -= sizeof(char);
	return *this;
}

CSerializeBuffer& CSerializeBuffer::operator >> (unsigned short& ushValue)
{
	if (m_iDataSize < sizeof(ushValue)) {
		throw CSerializeBufException(L"operator >> (unsigned short) error\n");
	}

	ushValue = *(unsigned short*)m_FrontPtr;
	m_FrontPtr += sizeof(unsigned short);
	m_iDataSize -= sizeof(unsigned short);
	return *this;
}

CSerializeBuffer& CSerializeBuffer::operator >> (short& shValue)
{
	if (m_iDataSize < sizeof(shValue)) {
		throw CSerializeBufException(L"operator >> (short) error\n");
	}

	shValue = *(short*)m_FrontPtr;
	m_FrontPtr += sizeof(short);
	m_iDataSize -= sizeof(short);
	return *this;
}

CSerializeBuffer& CSerializeBuffer::operator >> (unsigned int& dwValue)
{
	if (m_iDataSize < sizeof(dwValue)) {
		throw CSerializeBufException(L"operator >> (unsigned int) error\n");
	}

	dwValue = *(unsigned int*)m_FrontPtr;
	m_FrontPtr += sizeof(unsigned int);
	m_iDataSize -= sizeof(unsigned int);
	return *this;
}

CSerializeBuffer& CSerializeBuffer::operator >> (unsigned long& dwValue)
{
	if (m_iDataSize < sizeof(dwValue)) {
		throw CSerializeBufException(L"operator >> (unsigned int) error\n");
	}

	dwValue = *(unsigned long*)m_FrontPtr;
	m_FrontPtr += sizeof(unsigned long);
	m_iDataSize -= sizeof(unsigned long);
	return *this;
}

CSerializeBuffer& CSerializeBuffer::operator >> (int& iValue)
{
	if (m_iDataSize < sizeof(iValue)) {
		throw CSerializeBufException(L"operator >> (int) error\n");
	}

	iValue = *(int*)m_FrontPtr;
	m_FrontPtr += sizeof(int);
	m_iDataSize -= sizeof(int);
	return *this;
}

CSerializeBuffer& CSerializeBuffer::operator >> (float& fValue)
{
	if (m_iDataSize < sizeof(fValue)) {
		throw CSerializeBufException(L"operator >> (float) error\n");
	}

	fValue = *(float*)m_FrontPtr;
	m_FrontPtr += sizeof(float);
	m_iDataSize -= sizeof(float);
	return *this;
}

CSerializeBuffer& CSerializeBuffer::operator >> (unsigned __int64& uint64Value)
{
	if (m_iDataSize < sizeof(uint64Value)) {
		throw CSerializeBufException(L"operator >> (unsigned __int64) error\n");
	}

	uint64Value = *(unsigned __int64*)m_FrontPtr;
	m_FrontPtr += sizeof(unsigned __int64);
	m_iDataSize -= sizeof(unsigned __int64);
	return *this;
}

CSerializeBuffer& CSerializeBuffer::operator >> (__int64& int64Value)
{
	if (m_iDataSize < sizeof(int64Value)) {
		throw CSerializeBufException(L"operator >> (__int64) error\n");
	}

	int64Value = *(__int64*)m_FrontPtr;
	m_FrontPtr += sizeof(__int64);
	m_iDataSize -= sizeof(__int64);
	return *this;
}

CSerializeBuffer& CSerializeBuffer::operator >> (double& dValue)
{
	if (m_iDataSize < sizeof(dValue)) {
		throw CSerializeBufException(L"operator >> (double) error\n");
	}

	dValue = *(double*)m_FrontPtr;
	m_FrontPtr += sizeof(double);
	m_iDataSize -= sizeof(double);
	return *this;
}

//////////////////////////////////////////////////////////////////////////
// 데이타 얻기.
//
// Parameters: (char *)Dest 포인터. (int)Size.
// Return: (int)복사한 사이즈.
//////////////////////////////////////////////////////////////////////////
int		CSerializeBuffer::GetData(char* chpDest, int iSize)
{
	if (iSize > m_iDataSize || iSize < 0) {
		throw CSerializeBufException(L"GetData error\n");
	}

	for (int i = 0; i < iSize; ++i) {
		chpDest[i] = m_FrontPtr[i];
	}

	m_FrontPtr += iSize;
	m_iDataSize -= iSize;
	return iSize;
}

//////////////////////////////////////////////////////////////////////////
// 데이타 삽입.
//
// Parameters: (char *)Src 포인터. (int)SrcSize.
// Return: (int)복사한 사이즈.
//////////////////////////////////////////////////////////////////////////
int		CSerializeBuffer::PutData(char* chpSrc, int iSrcSize)
{
	if (iSrcSize > m_iBufferSize - m_iDataSize || iSrcSize < 0) {
		throw CSerializeBufException(L"PutData error\n");
	}

	for (int i = 0; i < iSrcSize; ++i) {
		m_BackPtr[i] = chpSrc[i];
	}

	m_BackPtr += iSrcSize;
	m_iDataSize += iSrcSize;
	return iSrcSize;
}