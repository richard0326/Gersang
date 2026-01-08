#pragma once

#include "framework.h"

// 네트워크 라이브러리
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>

// 일반 라이브러리
#include <Windows.h>
#include <memoryapi.h>
#include <iostream>
#include <stdio.h>
#include <unordered_map>
#include <string>
using namespace std;

// 메모리 누수
//#include "Lib/MemoryAllocMgr.h"
#include "Lib/log.h"

// 전역 define
#include "define.h"
#include "PacketDefine.h"
#include "PacketStruct.h"
#include "PacketProc.h"
#include "MessagePacking.h"

// 싱글톤
#include "Lib/Singleton.h"

// 자료구조
#include "Lib/List.h"
#include "Lib/Queue.h"

// 프로파일러
#include "Lib/Profiler.h"

// 파서
#include "Lib/Parser.h"