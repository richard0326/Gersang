#pragma once

// 네트워크 라이브러리
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>

#include <Windows.h>
#include <iostream>
#include <conio.h>
#include <ctime>
using namespace std;

#include <list>
#include <map>
#include <unordered_set>

#include <NabzackoLib/Singleton.h>
#include <NabzackoLib/RingBuffer.h>
#include <NabzackoLib/SerializeBuffer.h>
#include <NabzackoLib/FPSTimer.h>
#include "Protocol.h"

#include "MakePacket.h"
#include "defines.h"
