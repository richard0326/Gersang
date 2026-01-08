#pragma once
#include <exception>

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
public:

	/*---------------------------------------------------------------
	Packet Enum.

	----------------------------------------------------------------*/
	enum en_SERIALIZEBUFFER
	{
		eBUFFER_DEFAULT = 1400		// 패킷의 기본 버퍼 사이즈.
	};

	//////////////////////////////////////////////////////////////////////////
	// 생성자, 파괴자.
	//
	// Return:
	//////////////////////////////////////////////////////////////////////////
	CSerializeBuffer();
	CSerializeBuffer(int iBufferSize);

	~CSerializeBuffer();

	//////////////////////////////////////////////////////////////////////////
	// 패킷 청소.
	//
	// Parameters: 없음.
	// Return: 없음.
	//////////////////////////////////////////////////////////////////////////
	void	Clear(void);


	//////////////////////////////////////////////////////////////////////////
	// 버퍼 사이즈 얻기.
	//
	// Parameters: 없음.
	// Return: (int)패킷 버퍼 사이즈 얻기.
	//////////////////////////////////////////////////////////////////////////
	int		GetBufferSize(void);
	//////////////////////////////////////////////////////////////////////////
	// 현재 사용중인 사이즈 얻기.
	//
	// Parameters: 없음.
	// Return: (int)사용중인 데이타 사이즈.
	//////////////////////////////////////////////////////////////////////////
	int		GetDataSize(void);



	//////////////////////////////////////////////////////////////////////////
	// 버퍼 포인터 얻기.
	//
	// Parameters: 없음.
	// Return: (char *)버퍼 포인터.
	//////////////////////////////////////////////////////////////////////////
	char* GetBufferPtr(void);

	//////////////////////////////////////////////////////////////////////////
	// 버퍼 Pos 이동. (음수이동은 안됨)
	// GetBufferPtr 함수를 이용하여 외부에서 강제로 버퍼 내용을 수정할 경우 사용. 
	//
	// Parameters: (int) 이동 사이즈.
	// Return: (int) 이동된 사이즈.
	//////////////////////////////////////////////////////////////////////////
	int		MoveWritePos(int iSize);
	int		MoveReadPos(int iSize);

	/* ============================================================================= */
	// 연산자 오버로딩
	/* ============================================================================= */
	CSerializeBuffer& operator = (CSerializeBuffer& clSrcPacket);

	//////////////////////////////////////////////////////////////////////////
	// 넣기.	각 변수 타입마다 모두 만듬.
	//////////////////////////////////////////////////////////////////////////
	CSerializeBuffer& operator << (unsigned char byValue);
	CSerializeBuffer& operator << (char chValue);

	CSerializeBuffer& operator << (unsigned short wValue);
	CSerializeBuffer& operator << (short shValue);

	CSerializeBuffer& operator << (unsigned int dwValue);
	CSerializeBuffer& operator << (unsigned long dwValue);
	CSerializeBuffer& operator << (int iValue);
	CSerializeBuffer& operator << (float fValue);

	CSerializeBuffer& operator << (unsigned __int64 iValue);
	CSerializeBuffer& operator << (__int64 iValue);
	CSerializeBuffer& operator << (double dValue);


	//////////////////////////////////////////////////////////////////////////
	// 빼기.	각 변수 타입마다 모두 만듬.
	//////////////////////////////////////////////////////////////////////////
	CSerializeBuffer& operator >> (unsigned char& byValue);
	CSerializeBuffer& operator >> (char& chValue);

	CSerializeBuffer& operator >> (unsigned short& wValue);
	CSerializeBuffer& operator >> (short& shValue);

	CSerializeBuffer& operator >> (unsigned int& dwValue);
	CSerializeBuffer& operator >> (unsigned long& dwValue);
	CSerializeBuffer& operator >> (int& iValue);
	CSerializeBuffer& operator >> (float& fValue);

	CSerializeBuffer& operator >> (unsigned __int64& iValue);
	CSerializeBuffer& operator >> (__int64& iValue);
	CSerializeBuffer& operator >> (double& dValue);

	//////////////////////////////////////////////////////////////////////////
	// 데이타 얻기.
	//
	// Parameters: (char *)Dest 포인터. (int)Size.
	// Return: (int)복사한 사이즈.
	//////////////////////////////////////////////////////////////////////////
	int		GetData(char* chpDest, int iSize);

	//////////////////////////////////////////////////////////////////////////
	// 데이타 삽입.
	//
	// Parameters: (char *)Src 포인터. (int)SrcSize.
	// Return: (int)복사한 사이즈.
	//////////////////////////////////////////////////////////////////////////
	int		PutData(char* chpSrc, int iSrcSize);

protected:

	int	m_iBufferSize;

	//------------------------------------------------------------
	// 현재 버퍼에 사용중인 사이즈.
	//------------------------------------------------------------
	int	m_iDataSize;

	char* m_chpBuffer;
	char* m_FrontPtr;
	char* m_BackPtr;
};
