#include "stdafx.h"
#include "CrashDump.h"
#include "SystemLog.h"

// 자동 생성
CCrashDump g_dump;

long CCrashDump::m_DumpCount = 0;

CCrashDump::CCrashDump()
{
	m_DumpCount = 0;
	_invalid_parameter_handler oldHandler, newHandler;
	newHandler = myInvalidParameterHandler;

	// crt함수에 null포인터
	oldHandler = _set_invalid_parameter_handler(newHandler);
	_CrtSetReportMode(_CRT_WARN, 0);
	_CrtSetReportMode(_CRT_ASSERT, 0);
	_CrtSetReportMode(_CRT_ERROR, 0);

	_CrtSetReportHook(_custom_Report_hook);

	//----------------------------------------------
	// pure virtual function called 에러 핸들러를 사용자 정의 함수로 우회시킨다.
	//----------------------------------------------
	_set_purecall_handler(myPurecallHandler);

	SetHandlerDump();
}

void CCrashDump::Crash(void)
{
	int* p = nullptr;
	*p = 0;
}

LONG WINAPI CCrashDump::MyExceptionFilter(__in PEXCEPTION_POINTERS pExceptionPointer)
{
	int iWorkingMemory = 0;
	SYSTEMTIME stNowTime;

	long DumpCount = InterlockedIncrement(&m_DumpCount);

	//----------------------------------------------
	// 현재 프로세스의 메모리 사용량을 얻어온다. 
	// (덤프 파일의 용량이 곧 메모리 사용량을 의미한다... 비슷하다)
	//----------------------------------------------
	HANDLE hProcess = 0;
	PROCESS_MEMORY_COUNTERS pmc;
	
	hProcess = GetCurrentProcess();

	if (NULL == hProcess)
		return 0;

	if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
	{
		iWorkingMemory = (int)(pmc.WorkingSetSize / 1024 / 1024);
	}
	CloseHandle(hProcess);

	//----------------------------------------------
	// 현재 날짜와 시간을 알아온다.
	//----------------------------------------------
	WCHAR filename[255];
	GetLocalTime(&stNowTime);
	wsprintf(filename, L"Dump_%d%02d%02d_%02d.%02d.%02d_%d_%dMB.dmp",
		stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay, stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond, DumpCount, iWorkingMemory);
	
	wprintf(L"\n\n\n덤프 중!!!! 끄지마세요!!!!\n");
	wprintf(L"!!! Crash Error!!! %d.%d.%d / %d:%d:%d \n",
		stNowTime.wYear, stNowTime.wMonth, stNowTime.wDay, stNowTime.wHour, stNowTime.wMinute, stNowTime.wSecond);
	
	wprintf(L"Now Save dump file... \n");

	HANDLE hDumpFile = CreateFile(filename,
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);

	if (hDumpFile != INVALID_HANDLE_VALUE)
	{
		_MINIDUMP_EXCEPTION_INFORMATION MinidumpExceptionInformation = { 0, };

		MinidumpExceptionInformation.ThreadId = GetCurrentThreadId();
		MinidumpExceptionInformation.ExceptionPointers = pExceptionPointer;
		MinidumpExceptionInformation.ClientPointers = TRUE;

		MiniDumpWriteDump(GetCurrentProcess(),
			GetCurrentProcessId(),
			hDumpFile,
			MiniDumpWithFullMemory,
			&MinidumpExceptionInformation,
			NULL,
			NULL);

		CloseHandle(hDumpFile);

		wprintf(L"CrashDump Save Finish !\n");

		return EXCEPTION_EXECUTE_HANDLER;
	}
	else
	{
		int errCode = GetLastError();
		LOG(L"Dump", en_LOG_LEVEL::LEVEL_SYSTEM, L"CrashDump Fail!!! ErrCode : %d\n", errCode);
		wprintf(L"CrashDump Fail !\n");
	}

	return EXCEPTION_EXECUTE_HANDLER;
}

void CCrashDump::SetHandlerDump()
{
	SetUnhandledExceptionFilter(MyExceptionFilter);
}

// Invalid Parameter handler
void CCrashDump::myInvalidParameterHandler(const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved)
{
	Crash();
}

int CCrashDump::_custom_Report_hook(int ireposttype, char* message, int* returnvalue)
{
	Crash();
	return true;
}

void CCrashDump::myPurecallHandler(void)
{
	Crash();
}
