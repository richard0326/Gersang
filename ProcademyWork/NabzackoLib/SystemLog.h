#pragma once

enum class en_LOG_LEVEL
{
	LEVEL_DEBUG, // Debug 로그
	LEVEL_ERROR, // Error 로그
	LEVEL_SYSTEM, // System 관련 로그
};

enum class en_LOG_MODE
{
	MODE_NONE, // 아무것도 출력하지 않는 상황
	MODE_FILE, // 파일로만 로그를 저장하는 상황
	MODE_CONSOLE, // 콘솔 출력만 로그를 출력하는 상황
	MODE_ALL, // 파일, 콘솔 출력 모두 진행되는 상황
};

// 로그 관련 초기 설정
void SetLogMode(en_LOG_MODE logMode); // 로그 모드 설정
void SetLogLevel(en_LOG_LEVEL logLevel); // 로그 레벨 설정
void SetDirectory(const wchar_t* str); // 로그 경로 설정

// 로그 찍는 함수
void Log(const wchar_t* type, en_LOG_LEVEL logLevel, const wchar_t* str, ...);
// 16진수 로그 찍는 함수
void LogHex(const wchar_t* type, en_LOG_LEVEL logLevel, const wchar_t* str, void* pByte, int iByteLen);

// 로그를 저장 할 폴더 지정
#define SYSLOG_DIRECTORY(str)							SetDirectory(str)
// 로그 레벨 지정
#define SYSLOG_LEVEL(logLevel)							SetLogLevel(logLevel)
// 로그 모드 지정
#define SYSLOG_MODE(logMode)							SetLogMode(logMode)
// 로그 찍기
#define LOG(type, logLevel, str, ...)					Log(type, logLevel, str, __VA_ARGS__)
// 바이트를 포함한 로그 찍기
#define LOG_HEX(type, logLevel, str, pByte, iByteLen)	LogHex(type, logLevel, str, pByte, iByteLen)