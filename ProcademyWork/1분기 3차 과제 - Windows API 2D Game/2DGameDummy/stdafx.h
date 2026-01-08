#pragma once

#include "framework.h"

// 네트워크 라이브러리
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>

// 일반 라이브러리
#include <Windows.h>
#include <iostream>
#include <stdio.h>
#include <unordered_map>
#include <string>
#include <memory>
using namespace std;

// 메모리 누수
#include "MemoryAllocMgr.h"
#include "Log.h"

// 전역 define
#include "define.h"
#include "PacketDefine.h"
#include "PacketStruct.h"
#include "PacketProc.h"
#include "MessagePacking.h"

// 싱글톤
#include "Singleton.h"

// 자료구조
#include "List.h"
#include "Queue.h"

#include "RingBuffer.h"

#include "Parser.h"
