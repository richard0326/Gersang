#pragma once

#pragma comment(lib, "DbgHelp.Lib")
#include <dbghelp.h>
#include <psapi.h>
#include <crtdbg.h> 

class CCrashDump
{
public:
	CCrashDump();

	// 에러 상황
	static void Crash(void);

	// 덤프 핸들러 - 덤프 파일 저장하는 함수
	static LONG WINAPI MyExceptionFilter(__in PEXCEPTION_POINTERS pExceptionPointer);
	
	// 덤프 핸들러 등록하는 함수
	static void SetHandlerDump();

	// Invalid Parameter handler에 대한 에러 처리
	static void myInvalidParameterHandler(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved);

	static int _custom_Report_hook(int ireposttype, char* message, int* returnvalue);

	// 인터페이스 클래스의 생성자에서 가상 함수 호출시 발생
	static void myPurecallHandler(void);

	// 파일 중복 생성을 막기 위한 변수
	static long m_DumpCount;
};