#include "stdafx.h"

#include <Windows.h>
#include <ctime>
#include <strsafe.h>
#include "SystemLog.h"

wchar_t logWSTR[][20] = { L"DEBUG", L"ERROR", L"SYSTM" };

class CSystemLog
{
public:
	CSystemLog();
	~CSystemLog();

	bool CheckLevel(en_LOG_LEVEL logLevel);
	void SetLogMode(en_LOG_MODE logMode);
	void SetLogLevel(en_LOG_LEVEL logLevel);
	void SetDirectory(const wchar_t* str);
	void Log(const wchar_t* type, en_LOG_LEVEL logLevel, const wchar_t* str);
	void LogHex(const wchar_t* szType, en_LOG_LEVEL LogLevel, const wchar_t* szLog, BYTE* pByte, int iByteLen);

private:
	__declspec(align(64)) long long m_logCount;
	en_LOG_LEVEL m_LogLevel;
	en_LOG_MODE m_LogMode;
	wchar_t m_CurrentDirectory[1000];
	CRITICAL_SECTION m_LogCS;
};

CSystemLog g_SystemLog;

CSystemLog::CSystemLog()
{
	_wsetlocale(LC_ALL, L"korean");
	m_LogLevel = en_LOG_LEVEL::LEVEL_DEBUG;
	m_LogMode = en_LOG_MODE::MODE_NONE;

	InitializeCriticalSection(&m_LogCS);
	if (GetCurrentDirectory(1000, m_CurrentDirectory) == 0)
	{
		int* crash = nullptr;
		*crash = 0;
	}
}

CSystemLog::~CSystemLog()
{
	DeleteCriticalSection(&m_LogCS);
}

bool CSystemLog::CheckLevel(en_LOG_LEVEL logLevel)
{
	return logLevel >= m_LogLevel;
}

void CSystemLog::SetLogMode(en_LOG_MODE logMode)
{
	m_LogMode = logMode;
}

void CSystemLog::SetLogLevel(en_LOG_LEVEL logLevel)
{
	m_LogLevel = logLevel;
}

void CSystemLog::SetDirectory(const wchar_t* str)
{
	// 현재 다이렉토리 가져오기
	if (GetCurrentDirectory(1000, m_CurrentDirectory) == 0)
	{
		wprintf(L"SystemLog::SetDirectory Fail\n");
		return;
	}

	// stringSafe하게 문자열 합치기
	wchar_t fileName[1000];
	HRESULT hFNResult = StringCchPrintfW(fileName, 1000, L"%s\\%s", m_CurrentDirectory, str);
	if (hFNResult != S_OK)
	{
		// Crash
		int* crash = nullptr;
		*crash = 0;
	}

	// 파일 경로 생성하기
	CreateDirectory(fileName, NULL);
	wcscpy_s(m_CurrentDirectory, 1000, fileName);
}

void CSystemLog::Log(const wchar_t* type, en_LOG_LEVEL logLevel, const wchar_t* str)
{
	// 로그 파일로 저장할 경우
	if (m_LogMode == en_LOG_MODE::MODE_FILE || m_LogMode == en_LOG_MODE::MODE_ALL)
	{
		SYSTEMTIME		st;
		GetLocalTime(&st);

		long long logCount = InterlockedIncrement64(&m_logCount);

		// string Safe하게 파일 이름 합치기
		wchar_t fileName[1024];
		HRESULT hResult = StringCchPrintfW(fileName, 1024, L"%s\\%d%02d_%s.txt", m_CurrentDirectory, st.wYear, st.wMonth, type);
		if (hResult != S_OK)
		{
			// Crash
			int* crash = nullptr;
			*crash = 0;
		}

		// string Safe하게 저장할 로그 문자열 합치기
		wchar_t combineStr[2024];
		hResult = StringCchPrintfW(combineStr, 2024, L"[%s][%04d-%02d-%02d %02d:%02d:%02d / %s / %09d] %s\n",
			type,
			st.wYear, st.wMonth, st.wDay,
			st.wHour, st.wMinute, st.wSecond,
			logWSTR[(int)logLevel],
			logCount,
			str);

		if (hResult != S_OK)
		{
			// 혹시 크기가 부족한 경우...
			// 최대 크기인 STRSAFE_MAX_CCH 크기 만큼 새로 동적 할당하여 로그를 찍도록 합니다...
			LOG(L"SystemLogFail", en_LOG_LEVEL::LEVEL_ERROR, L"FailLog2");
			wchar_t* tempCombineStr = new wchar_t[STRSAFE_MAX_CCH];
			hResult = StringCchPrintfW(tempCombineStr, STRSAFE_MAX_CCH, L"[%s][%d-%d-%d %d:%d:%d / %s / %09d] %s\n",
				type,
				st.wYear, st.wMonth, st.wDay,
				st.wHour, st.wMinute, st.wSecond,
				logWSTR[(int)logLevel],
				logCount,
				str);

			// 이것 조차 실패한다면... 크래시를 내도록 했습니다...
			if (hResult != S_OK)
			{
				// Crash
				int* crash = nullptr;
				*crash = 0;
			}

			// 멀티스레드에서도 가능하도록
			// 파일 열기 / 저장 / 닫기
			EnterCriticalSection(&m_LogCS);

			FILE* fp = nullptr;
			for(;;)
			{
				if (_wfopen_s(&fp, fileName, L"at, ccs=UTF-16LE") == 0)
				{
					// 파일 쓰기
					fputws(tempCombineStr, fp);

					delete[] tempCombineStr;
					fclose(fp);
					break;
				}
			}

			LeaveCriticalSection(&m_LogCS);
		}
		else
		{
			// 멀티스레드에서도 가능하도록
			// 파일 열기 / 저장 / 닫기
			EnterCriticalSection(&m_LogCS);

			FILE* fp = nullptr;
			for (;;)
			{
				if (_wfopen_s(&fp, fileName, L"at, ccs=UTF-16LE") == 0)
				{
					// 파일 쓰기
					fputws(combineStr, fp);

					fclose(fp);
					break;
				}
			}

			LeaveCriticalSection(&m_LogCS);
		}
	}

	// 로그 콘솔 출력할 경우
	if (m_LogMode == en_LOG_MODE::MODE_CONSOLE || m_LogMode == en_LOG_MODE::MODE_ALL)
	{
		wprintf_s(L"%s", str);
	}
}

void CSystemLog::LogHex(const wchar_t* type, en_LOG_LEVEL logLevel, const wchar_t* str, BYTE* pByte, int iByteLen)
{
	if (m_LogMode == en_LOG_MODE::MODE_FILE || m_LogMode == en_LOG_MODE::MODE_ALL)
	{
		SYSTEMTIME		st;
		GetLocalTime(&st);

		long long logCount = InterlockedIncrement64(&m_logCount);

		// string Safe하게 파일 이름 합치기
		wchar_t fileName[1024];
		HRESULT hResult = StringCchPrintfW(fileName, 1024, L"%s\\%d%02d_%s.txt", m_CurrentDirectory, st.wYear, st.wMonth, type);
		if (hResult != S_OK)
		{
			// Crash
			int* crash = nullptr;
			*crash = 0;
		}

		// string Safe하게 저장할 로그 문자열 합치기
		wchar_t combineStr[2024];
		hResult = StringCchPrintfW(combineStr, 2024, L"[%s][%04d-%02d-%02d %02d:%02d:%02d / %s / %09d] %s\n",
			type,
			st.wYear, st.wMonth, st.wDay,
			st.wHour, st.wMinute, st.wSecond,
			logWSTR[(int)logLevel],
			logCount,
			str);

		if (hResult != S_OK)
		{
			// 크기가 부족한 경우가 있을 수 있지만 거의 희박하다고 보고, 크래시를 내도록 하겠습니다.
			// Crash
			int* crash = nullptr;
			*crash = 0;
		}
		else
		{
			// 멀티스레드에서 가능하도록
			// 파일 열기 / 저장 / 닫기
			EnterCriticalSection(&m_LogCS);

			FILE* fp = nullptr;
			for (;;)
			{
				if (_wfopen_s(&fp, fileName, L"at, ccs=UTF-16LE") == 0)
				{
					// 파일 쓰기
					fputws(combineStr, fp);

					for (int i = 0; i < iByteLen; i++)
					{
						wchar_t wStr[100];
						hResult = StringCchPrintfW(wStr, 100, L"0x%d\n", pByte[i]);
						fputws(wStr, fp);
					}
					fclose(fp);
					break;
				}
			}

			LeaveCriticalSection(&m_LogCS);
		}
	}

	if (m_LogMode == en_LOG_MODE::MODE_CONSOLE || m_LogMode == en_LOG_MODE::MODE_ALL)
	{
		wprintf_s(L"%s", str);

		for (int i = 0; i < iByteLen; i++)
		{
			wprintf_s(L"0x%x\n", pByte[i]);
		}
	}
}

void SetLogMode(en_LOG_MODE logMode)
{
	g_SystemLog.SetLogMode(logMode);
}

void SetLogLevel(en_LOG_LEVEL logLevel)
{
	g_SystemLog.SetLogLevel(logLevel);
}

void SetDirectory(const wchar_t* str)
{
	g_SystemLog.SetDirectory(str);
}

void Log(const wchar_t* type, en_LOG_LEVEL logLevel, const wchar_t* str, ...)
{
	// 로그 레벨이 가능한 경우...
	if (g_SystemLog.CheckLevel(logLevel) == false)
		return;

	wchar_t combineStr[2024];

	va_list		ap;
	va_start(ap, str);

	// string safe하게 문자열 합치기
	HRESULT hResult = StringCchVPrintf(combineStr, 2024, str, ap);
	if (hResult != S_OK)
	{
		// 문자열이 너무 큰 경우...
		// 최대 크기 만큼 할당하여 문자열 합치기
		LOG(L"SystemLogFail", en_LOG_LEVEL::LEVEL_ERROR, L"FailLog1");
		wchar_t* tempCombineStr = new wchar_t[STRSAFE_MAX_CCH];
		hResult = StringCchVPrintf(tempCombineStr, STRSAFE_MAX_CCH, str, ap);
		if (hResult != S_OK)
		{
			// 최대 크기도 안되는 경우... 크래시
			int* crash = nullptr;
			*crash = 0;
		}
		va_end(ap);
		g_SystemLog.Log(type, logLevel, combineStr);

		delete [] tempCombineStr;
	}
	else
	{
		va_end(ap);
		g_SystemLog.Log(type, logLevel, combineStr);
	}	
}

void LogHex(const wchar_t* type, en_LOG_LEVEL logLevel, const wchar_t* str, void* pByte, int iByteLen)
{
	// 로그 레벨이 가능한지 체크한다...
	if (g_SystemLog.CheckLevel(logLevel) == false)
		return;

	g_SystemLog.LogHex(type, logLevel, str, (BYTE*)pByte, iByteLen);
}