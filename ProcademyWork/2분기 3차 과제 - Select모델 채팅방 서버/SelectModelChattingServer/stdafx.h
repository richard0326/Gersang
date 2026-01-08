#pragma once

#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream> 
using namespace std;
#include <list>

#include <NabzackoLib/Singleton.h>
#include <NabzackoLib/RingBuffer.h>
#include <NabzackoLib/SerializeBuffer.h>

#include <NabzackoLib/SelectModel.h>
#include "ContentContext.h"
#include "Protocol.h"
#include "SerializeBufferPacking.h"
#include "SerializeBufferProc.h"