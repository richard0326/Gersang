#include "stdafx.h"

int main() {

    setlocale(LC_ALL, "korean");

    CSelectServer selectServer;
    selectServer.Init(L"127.0.0.1", dfNETWORK_PORT);

    while (true)
    {
        selectServer.SelectLoop(AcceptSession, DeleteSession, RecvPacket);
    }

    selectServer.Release();
   
    return 0;
}