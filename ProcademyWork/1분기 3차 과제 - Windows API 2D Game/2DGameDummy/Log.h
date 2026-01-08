#pragma once

// 로그 출력 타입
#define LOG_NONE			0x00000000
#define LOG_CONSOLE			0x00000001
#define LOG_FILE			0x00000002
#define LOG_WINDOW			0x00000004
#define LOG_NO_CONSOLE		( LOG_FILE | LOG_WINDOW )
#define LOG_NO_WINDOWS		( LOG_CONSOLE | LOG_FILE )
#define LOG_ALL				( LOG_CONSOLE | LOG_FILE | LOG_WINDOW )

void InitLog(unsigned int target, const wchar_t* fileName = nullptr);
int AddLog(const wchar_t* strmsg, ...);
void ReleaseLog();