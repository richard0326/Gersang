#pragma once

/////////////////////////////////////////////////////////////////////////////
// 하나의 함수 Profiling 시작, 끝 함수.
//
// Parameters: (char *)Profiling이름.
// Return: 없음.
/////////////////////////////////////////////////////////////////////////////
void ProfileBegin(const wchar_t* szName);
void ProfileEnd(const wchar_t* szName);

/////////////////////////////////////////////////////////////////////////////
// Profiling 된 데이타를 Text 파일로 출력한다.
//
// Parameters: (char *)출력될 파일 이름.
// Return: 없음.
/////////////////////////////////////////////////////////////////////////////
void ProfileDataOutText(const wchar_t* szFileName);

/////////////////////////////////////////////////////////////////////////////
// 프로파일링 된 데이터를 모두 초기화 한다.
//
// Parameters: 없음.
// Return: 없음.
/////////////////////////////////////////////////////////////////////////////
void ProfileReset(void);

#define _PROFILE_ON

#ifdef	_PROFILE_ON

#define PRO_BEGIN(TagName)		ProfileBegin(TagName)
#define PRO_END(TagName)		ProfileEnd(TagName)
#define PRO_OUT(filename)		ProfileDataOutText(filename)
#define PRO_RESET()				ProfileReset()

#else

#define PRO_BEGIN(TagName)		;	
#define PRO_END(TagName)		;	
#define PRO_OUT(filename)		;	
#define PRO_RESET()				;	

#endif // _PROFILE_ON

class CAutoProfiling
{
public:
	CAutoProfiling(const wchar_t* szTagName)
	{
		wcscpy_s(m_tagname, 50, szTagName);
		PRO_BEGIN(m_tagname);
	}

	~CAutoProfiling()
	{
		PRO_END(m_tagname);
	}

private:
	wchar_t m_tagname[50];
};
