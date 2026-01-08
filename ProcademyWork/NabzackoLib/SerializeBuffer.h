#pragma once

#include <exception>
#include "LockFreeTlsPoolA.h"

// 직렬화 버퍼 예외처리를 위한 객체
class CSerializeBufException : public std::exception
{
	wchar_t* message;
public:
	CSerializeBufException(const wchar_t* _m)
	{
		message = (wchar_t*)_m;
	}

	const char* what() const throw()
	{
		return (char*)message;
	}
};

class CSerializeBuffer
{
	// 직렬화 버퍼에 직접 접근이 가능한 객체들
	template <typename T> friend class CLockFreeTlsPoolA;
	friend class CLanServer;
	friend class CLanClient;
	friend class CNetServer;
	friend class CNetClient;
	friend class CMMOServer;

	CSerializeBuffer();
	~CSerializeBuffer();
public:

	// 직렬화 버퍼를 Parser를 통해서 외부에서 크기 설정이 가능하도록 Init을 전역화
	static bool Init();
	// Stop 이후 Start를 할때 사용할 Reset
	static void Reset();

	// 직렬화 버퍼를 Alloc하기 전에 크기가 충분한지 확인하기 위한 함수
	// 공격 패킷을 제거하기 위해서 한번 체크해주는 역할
	__forceinline static bool Check(int headerSize, int iBufferSize)
	{
		if (headerSize + iBufferSize > m_iMaxBufferSize)
			return false;

		return true;
	}

	// 직렬화 버퍼를 할당
	__forceinline static CSerializeBuffer* Alloc(int headerSize, int iBufferSize)
	{
		if (headerSize + iBufferSize > m_iMaxBufferSize)
			return nullptr;

		// tlsPool에서 직렬화 버퍼를 가져오는데 실패할 경우 crash를 방생시킵니다.
		CSerializeBuffer* pPacket = m_tlsPool->Alloc();
		if (pPacket == nullptr)
		{
			int* crash = nullptr;
			*crash = 0;
		}

		// alloc하기 전에 초기화하는 부분
		pPacket->m_iBufferSize = iBufferSize;

		pPacket->m_HeaderSize = headerSize;
		pPacket->m_DataSize = 0;
		pPacket->m_isCryption = false;
		pPacket->m_RandomKey = 0;

		pPacket->m_DataBuffer = pPacket->m_HeaderBuffer + headerSize;
		pPacket->m_FrontPtr = pPacket->m_BackPtr = pPacket->m_DataBuffer;

		pPacket->m_RefCount = 1;
#ifdef dfPACKET_KEY
		pPacket->m_Key = dfPACKET_KEY;
#endif

#ifdef MY_DEBUG
		for (int i = 0; i < 16; i++) {
			pPacket->m_logPacket[i] = 0;
			pPacket->m_sendID[i] = 0;
		}
		pPacket->m_logCount = 0;
#endif
		//

		return pPacket;
	}

	__forceinline static void* GetTlsPool()
	{
		return m_tlsPool;
	}

	//////////////////////////////////////////////////////////////////////////
	// 패킷 청소.
	//
	// Parameters: 없음.
	// Return: 없음.
	//////////////////////////////////////////////////////////////////////////
	__forceinline void	Clear(void)
	{
		m_DataSize = 0;
		m_FrontPtr = m_BackPtr = m_DataBuffer;
	}

	//////////////////////////////////////////////////////////////////////////
	// 버퍼 사이즈 얻기.
	//
	// Parameters: 없음.
	// Return: (int)패킷 버퍼 사이즈 얻기.
	//////////////////////////////////////////////////////////////////////////
	__forceinline int	GetBufferSize(void)
	{
		return m_iBufferSize;
	}

	//////////////////////////////////////////////////////////////////////////
	// 현재 사용중인 사이즈 얻기.
	//
	// Parameters: 없음.
	// Return: (int)사용중인 데이타 사이즈.
	//////////////////////////////////////////////////////////////////////////
	__forceinline int		GetDataSize(void)
	{
		return m_DataSize;
	}

	__forceinline int		GetHeaderDateSize(void)
	{
		return m_DataSize + m_HeaderSize;
	}

	//////////////////////////////////////////////////////////////////////////
	// 버퍼 포인터 얻기.
	//
	// Parameters: 없음.
	// Return: (char *)버퍼 포인터.
	//////////////////////////////////////////////////////////////////////////
	__forceinline char* GetHeaderPtr(void)
	{
		return m_HeaderBuffer;
	}

	__forceinline char* GetBufferPtr(void)
	{
		return m_DataBuffer;
	}

	//////////////////////////////////////////////////////////////////////////
	// 버퍼 Pos 이동. (음수이동은 안됨)
	// GetBufferPtr 함수를 이용하여 외부에서 강제로 버퍼 내용을 수정할 경우 사용. 
	//
	// Parameters: (int) 이동 사이즈.
	// Return: (int) 이동된 사이즈.
	//////////////////////////////////////////////////////////////////////////
	__forceinline int		MoveWritePos(int iSize)
	{
		if (iSize <= 0 || m_DataSize + iSize > m_iBufferSize) {
			return -1;
		}

		m_BackPtr += iSize;
		m_DataSize += iSize;

		return iSize;
	}

	// 패킷 읽는 포인터 size만큼 이동시키기
	__forceinline int		MoveReadPos(int iSize)
	{
		if (iSize <= 0 || iSize > m_DataSize) {
			return -1;
		}

		m_FrontPtr += iSize;
		m_DataSize -= iSize;

		return iSize;
	}

	/* ============================================================================= */
	// 연산자 오버로딩
	/* ============================================================================= */
	__forceinline CSerializeBuffer& operator = (CSerializeBuffer& clSrcPacket)
	{
		m_iBufferSize = clSrcPacket.m_iBufferSize;
		m_HeaderSize = clSrcPacket.m_HeaderSize;
		m_DataSize = clSrcPacket.m_DataSize;
		m_RefCount = clSrcPacket.m_RefCount;
		m_DataBuffer = clSrcPacket.m_DataBuffer;

		int size = m_DataSize + m_HeaderSize;
		for (int copyLoop = 0; copyLoop < size; ++copyLoop)
		{
			m_HeaderBuffer[copyLoop] = clSrcPacket.m_HeaderBuffer[copyLoop];
		}
		m_FrontPtr = clSrcPacket.m_FrontPtr;
		m_BackPtr = clSrcPacket.m_BackPtr;

		m_Key = clSrcPacket.m_Key;
		m_RandomKey = clSrcPacket.m_RandomKey;
		m_isCryption = clSrcPacket.m_isCryption;

		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	// 넣기.	각 변수 타입마다 모두 만듬.
	//////////////////////////////////////////////////////////////////////////
	__forceinline CSerializeBuffer& operator << (unsigned char uchValue)
	{
#ifdef MY_DEBUG
		if (m_iBufferSize - m_DataSize < sizeof(uchValue)) {
			throw CSerializeBufException(L"operator << (unsigned char) error\n");
		}
#endif

		* (unsigned char*)m_BackPtr = uchValue;
		m_BackPtr += sizeof(unsigned char);
		m_DataSize += sizeof(unsigned char);
		return *this;
	}

	__forceinline CSerializeBuffer& operator << (char chValue)
	{
#ifdef MY_DEBUG
		if (m_iBufferSize - m_DataSize < sizeof(chValue)) {
			throw CSerializeBufException(L"operator << (char) error\n");
		}
#endif

		* (char*)m_BackPtr = chValue;
		m_BackPtr += sizeof(char);
		m_DataSize += sizeof(char);
		return *this;
	}

	__forceinline CSerializeBuffer& operator << (unsigned short ushValue)
	{
#ifdef MY_DEBUG
		if (m_iBufferSize - m_DataSize < sizeof(ushValue)) {
			throw CSerializeBufException(L"operator << (unsigned short) error\n");
		}
#endif

		* (unsigned short*)m_BackPtr = ushValue;
		m_BackPtr += sizeof(unsigned short);
		m_DataSize += sizeof(unsigned short);
		return *this;
	}

	__forceinline CSerializeBuffer& operator << (short shValue)
	{
#ifdef MY_DEBUG
		if (m_iBufferSize - m_DataSize < sizeof(shValue)) {
			throw CSerializeBufException(L"operator << (short) error\n");
		}
#endif

		* (short*)m_BackPtr = shValue;
		m_BackPtr += sizeof(short);
		m_DataSize += sizeof(short);
		return *this;
	}

	__forceinline CSerializeBuffer& operator << (unsigned int uintValue)
	{
#ifdef MY_DEBUG
		if (m_iBufferSize - m_DataSize < sizeof(uintValue)) {
			throw CSerializeBufException(L"operator << (unsigned int) error\n");
		}
#endif

		* (unsigned int*)m_BackPtr = uintValue;
		m_BackPtr += sizeof(unsigned int);
		m_DataSize += sizeof(unsigned int);
		return *this;
	}

	__forceinline CSerializeBuffer& operator << (unsigned long uintValue)
	{
#ifdef MY_DEBUG
		if (m_iBufferSize - m_DataSize < sizeof(uintValue)) {
			throw CSerializeBufException(L"operator << (unsigned int) error\n");
		}
#endif

		* (unsigned long*)m_BackPtr = uintValue;
		m_BackPtr += sizeof(unsigned long);
		m_DataSize += sizeof(unsigned long);
		return *this;
	}

	__forceinline CSerializeBuffer& operator << (int iValue)
	{
#ifdef MY_DEBUG
		if (m_iBufferSize - m_DataSize < sizeof(iValue)) {
			throw CSerializeBufException(L"operator << (int) error\n");
		}
#endif

		* (int*)m_BackPtr = iValue;
		m_BackPtr += sizeof(int);
		m_DataSize += sizeof(int);
		return *this;
	}

	__forceinline CSerializeBuffer& operator << (float fValue)
	{
#ifdef MY_DEBUG
		if (m_iBufferSize - m_DataSize < sizeof(fValue)) {
			throw CSerializeBufException(L"operator << (float) error\n");
		}
#endif

		* (float*)m_BackPtr = fValue;
		m_BackPtr += sizeof(float);
		m_DataSize += sizeof(float);
		return *this;
	}

	__forceinline CSerializeBuffer& operator << (unsigned __int64 uint64Value)
	{
#ifdef MY_DEBUG
		if (m_iBufferSize - m_DataSize < sizeof(uint64Value)) {
			throw CSerializeBufException(L"operator << (unsigned __int64) error\n");
		}
#endif

		* (unsigned __int64*)m_BackPtr = uint64Value;
		m_BackPtr += sizeof(unsigned __int64);
		m_DataSize += sizeof(unsigned __int64);
		return *this;
	}

	__forceinline CSerializeBuffer& operator << (__int64 int64Value)
	{
#ifdef MY_DEBUG
		if (m_iBufferSize - m_DataSize < sizeof(int64Value)) {
			throw CSerializeBufException(L"operator << (__int64) error\n");
		}
#endif

		* (__int64*)m_BackPtr = int64Value;
		m_BackPtr += sizeof(__int64);
		m_DataSize += sizeof(__int64);
		return *this;
	}

	__forceinline CSerializeBuffer& operator << (double dValue)
	{
#ifdef MY_DEBUG
		if (m_iBufferSize - m_DataSize < sizeof(dValue)) {
			throw CSerializeBufException(L"operator << (double) error\n");
			return *this;
		}
#endif

		* (double*)m_BackPtr = dValue;
		m_BackPtr += sizeof(double);
		m_DataSize += sizeof(double);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	// 빼기.	각 변수 타입마다 모두 만듬.
	//////////////////////////////////////////////////////////////////////////
	__forceinline CSerializeBuffer& operator >> (unsigned char& uchValue)
	{
#ifdef MY_DEBUG
		if (m_DataSize < sizeof(uchValue)) {
			throw CSerializeBufException(L"operator >> (unsigned char) error\n");
		}
#endif

		uchValue = *(unsigned char*)m_FrontPtr;
		m_FrontPtr += sizeof(unsigned char);
		m_DataSize -= sizeof(unsigned char);
		return *this;
	}

	__forceinline CSerializeBuffer& operator >> (char& chValue)
	{
#ifdef MY_DEBUG
		if (m_DataSize < sizeof(chValue)) {
			throw CSerializeBufException(L"operator >> (char) error\n");
		}
#endif

		chValue = *(char*)m_FrontPtr;
		m_FrontPtr += sizeof(char);
		m_DataSize -= sizeof(char);
		return *this;
	}

	__forceinline CSerializeBuffer& operator >> (unsigned short& ushValue)
	{
#ifdef MY_DEBUG
		if (m_DataSize < sizeof(ushValue)) {
			throw CSerializeBufException(L"operator >> (unsigned short) error\n");
		}
#endif

		ushValue = *(unsigned short*)m_FrontPtr;
		m_FrontPtr += sizeof(unsigned short);
		m_DataSize -= sizeof(unsigned short);
		return *this;
	}

	__forceinline CSerializeBuffer& operator >> (short& shValue)
	{
#ifdef MY_DEBUG
		if (m_DataSize < sizeof(shValue)) {
			throw CSerializeBufException(L"operator >> (short) error\n");
		}
#endif

		shValue = *(short*)m_FrontPtr;
		m_FrontPtr += sizeof(short);
		m_DataSize -= sizeof(short);
		return *this;
	}

	__forceinline CSerializeBuffer& operator >> (unsigned int& dwValue)
	{
#ifdef MY_DEBUG
		if (m_DataSize < sizeof(dwValue)) {
			throw CSerializeBufException(L"operator >> (unsigned int) error\n");
		}
#endif

		dwValue = *(unsigned int*)m_FrontPtr;
		m_FrontPtr += sizeof(unsigned int);
		m_DataSize -= sizeof(unsigned int);
		return *this;
	}

	__forceinline CSerializeBuffer& operator >> (unsigned long& dwValue)
	{
#ifdef MY_DEBUG
		if (m_DataSize < sizeof(dwValue)) {
			throw CSerializeBufException(L"operator >> (unsigned int) error\n");
		}
#endif

		dwValue = *(unsigned long*)m_FrontPtr;
		m_FrontPtr += sizeof(unsigned long);
		m_DataSize -= sizeof(unsigned long);
		return *this;
	}

	__forceinline CSerializeBuffer& operator >> (int& iValue)
	{
#ifdef MY_DEBUG
		if (m_DataSize < sizeof(iValue)) {
			throw CSerializeBufException(L"operator >> (int) error\n");
		}
#endif

		iValue = *(int*)m_FrontPtr;
		m_FrontPtr += sizeof(int);
		m_DataSize -= sizeof(int);
		return *this;
	}

	__forceinline CSerializeBuffer& operator >> (float& fValue)
	{
#ifdef MY_DEBUG
		if (m_DataSize < sizeof(fValue)) {
			throw CSerializeBufException(L"operator >> (float) error\n");
		}
#endif

		fValue = *(float*)m_FrontPtr;
		m_FrontPtr += sizeof(float);
		m_DataSize -= sizeof(float);
		return *this;
	}

	__forceinline CSerializeBuffer& operator >> (unsigned __int64& uint64Value)
	{
#ifdef MY_DEBUG
		if (m_DataSize < sizeof(uint64Value)) {
			throw CSerializeBufException(L"operator >> (unsigned __int64) error\n");
		}
#endif

		uint64Value = *(unsigned __int64*)m_FrontPtr;
		m_FrontPtr += sizeof(unsigned __int64);
		m_DataSize -= sizeof(unsigned __int64);
		return *this;
	}

	__forceinline CSerializeBuffer& operator >> (__int64& int64Value)
	{
#ifdef MY_DEBUG
		if (m_DataSize < sizeof(int64Value)) {
			throw CSerializeBufException(L"operator >> (__int64) error\n");
		}
#endif

		int64Value = *(__int64*)m_FrontPtr;
		m_FrontPtr += sizeof(__int64);
		m_DataSize -= sizeof(__int64);
		return *this;
	}

	__forceinline CSerializeBuffer& operator >> (double& dValue)
	{
#ifdef MY_DEBUG
		if (m_DataSize < sizeof(dValue)) {
			throw CSerializeBufException(L"operator >> (double) error\n");
		}
#endif
		dValue = *(double*)m_FrontPtr;
		m_FrontPtr += sizeof(double);
		m_DataSize -= sizeof(double);
		return *this;
	}

	//////////////////////////////////////////////////////////////////////////
	// 데이타 얻기.
	//
	// Parameters: (char *)Dest 포인터. (int)Size.
	// Return: (int)복사한 사이즈.
	//////////////////////////////////////////////////////////////////////////
	__forceinline int		GetData(char* chpDest, int iSize)
	{
		if (iSize > m_DataSize || iSize < 0) {
			return -1;
		}

		for (int i = 0; i < iSize; ++i) {
			chpDest[i] = m_FrontPtr[i];
		}

		m_FrontPtr += iSize;
		m_DataSize -= iSize;
		return iSize;
	}

	//////////////////////////////////////////////////////////////////////////
	// 데이타 삽입.
	//
	// Parameters: (char *)Src 포인터. (int)SrcSize.
	// Return: (int)복사한 사이즈.
	//////////////////////////////////////////////////////////////////////////
	__forceinline int		PutData(char* chpSrc, int iSrcSize)
	{
		if (iSrcSize > m_iBufferSize - m_DataSize || iSrcSize < 0) {
			return -1;
		}

		for (int i = 0; i < iSrcSize; ++i) {
			m_BackPtr[i] = chpSrc[i];
		}

		m_BackPtr += iSrcSize;
		m_DataSize += iSrcSize;
		return iSrcSize;
	}

	// 직렬화 버퍼의 RefCount 증가/감소
	__forceinline void IncreaseRefCount()
	{
		InterlockedIncrement((LONG*)&m_RefCount);
	}

	void DecreaseRefCount();

	// NetServer용 헤더 세팅(Code 설정, 암호화, len, randKey)
	__forceinline void SetNetHeader()
	{
		if (m_isCryption)
			return;

#ifdef dfPACKET_CODE
		m_HeaderBuffer[0] = dfPACKET_CODE; // Code
#else
		m_HeaderBuffer[0] = 0;
#endif
		short* len = (short*)&m_HeaderBuffer[1];
		*len = m_DataSize;
		m_HeaderBuffer[3] = m_RandomKey = (char)rand();

		Encryption();
	}

	// LanServer용 헤더 세팅(len)
	__forceinline void SetLanHeader()
	{
		if (m_isCryption)
			return;

		short* len = (short*)&m_HeaderBuffer[0];
		*len = m_DataSize;

		m_isCryption = true;
	}

	__forceinline void Encryption()
	{
		m_isCryption = true;
		char* buf = m_DataBuffer - 1;
		char key = m_Key;
		char randomKey = m_RandomKey;
		int size = m_DataSize + 1;

		BYTE p = 0;
		BYTE e = 0;
		unsigned int checkSum = 0;
		for (int i = 1; i < size; ++i)
		{
			checkSum = (checkSum + buf[i]) % 256;
		}
		buf[0] = checkSum;

		for (int i = 0; i < size; ++i)
		{
			p = buf[i] ^ (p + randomKey + i + 1);
			e = p ^ (e + key + i + 1);
			buf[i] = e;
		}
	}

	// header가 CSerializeBuffer에 포함되지 않은 경우
	__forceinline bool Decryption(char* pHeader)
	{
#ifdef dfPACKET_CODE
		if (pHeader[0] != dfPACKET_CODE)
			return false;
#endif
		short* pLen = (short*)(pHeader + 1);
		if (*pLen != m_DataSize)
			return false;

		if (m_isCryption)
			return true;

		m_isCryption = true;

		char* buf = m_DataBuffer - 1;
		char key = m_Key;
		char randomKey = pHeader[3];
		BYTE p = 0;
		BYTE e = 0;
		BYTE p2 = 0;
		BYTE e2 = 0;

		unsigned int checkSum = 0;
		int size = *pLen + 1;
		buf[0] = (char)pHeader[4];
		for (int i = 0; i < size; i++)
		{
			e = buf[i];
			p = e ^ (e2 + key + i + 1);
			buf[i] = p ^ (p2 + randomKey + i + 1);
			p2 = p;
			e2 = e;

			if (i != 0)
				checkSum = (checkSum + buf[i]) % 256;
		}

		if ((unsigned char)checkSum != (unsigned char)buf[0])
			return false;

		return true;
	}

	// header가 이미 포함된 경우
	__forceinline bool Decryption()
	{
#ifdef dfPACKET_CODE
		if (m_HeaderBuffer[0] != dfPACKET_CODE)
			return false;
#endif
		short* pLen = (short*)(m_HeaderBuffer + 1);
		if (*pLen != m_DataSize)
			return false;

		if (m_isCryption)
			return true;

		m_isCryption = true;

		char* buf = m_DataBuffer - 1;
		char key = m_Key;
		char randomKey = m_HeaderBuffer[3];
		BYTE p = 0;
		BYTE e = 0;
		BYTE p2 = 0;
		BYTE e2 = 0;

		unsigned int checkSum = 0;
		int size = *pLen + 1;
		buf[0] = (char)m_HeaderBuffer[4];
		for (int i = 0; i < size; i++)
		{
			e = buf[i];
			p = e ^ (e2 + key + i + 1);
			buf[i] = p ^ (p2 + randomKey + i + 1);
			p2 = p;
			e2 = e;

			if (i != 0)
				checkSum = (checkSum + buf[i]) % 256;
		}

		if ((unsigned char)checkSum != (unsigned char)buf[0])
			return false;

		return true;
	}

#ifdef MY_DEBUG
	void SetLog(char pos, unsigned long long sendID);
#endif

protected:
	alignas(64) int m_RefCount;
	char* m_FrontPtr;
	char* m_BackPtr;
	int	m_DataSize;

	char* m_HeaderBuffer;
	char* m_DataBuffer;
	static CLockFreeTlsPoolA<CSerializeBuffer>* m_tlsPool;
	static int	m_iMaxBufferSize;
	int	m_iBufferSize;
	int	m_HeaderSize;
	char m_Key;
	char m_RandomKey;
	bool m_isCryption;

#ifdef MY_DEBUG
	char m_logPacket[16];
	unsigned long long m_sendID[16];
	short m_logCount;
#endif
};