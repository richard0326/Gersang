#pragma once

#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <list>
using namespace std;

#include <NabzackoLib/Singleton.h>
#include <NabzackoLib/RingBuffer.h>
#include <NabzackoLib/SerializeBuffer.h>

#include "Session.h"
#include "SerializeBufferProc.h"
#include "SerializeBufferPacking.h"

#include "Protocol.h"