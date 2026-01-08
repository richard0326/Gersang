// 2DGame.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

/*************************************************************************
> File Name: 2DGame WindowsAPI
> Author: DeungHeok Chung (Nickname : Nabzacko)
> E-mail: richard0326@maver.com
> Created Time: 2020/09/01
> Personal Blog: https://github.com/richard0326
************************************************************************/

#include "stdafx.h"
#include "2DGame.h"
#include "GameMgr.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    if (SINGLETON(CGameMgr)->Init(hInstance, L"WinAPI 2DGame ( Made by Nabzacko )") == false)
        return 0;

    int ret = SINGLETON(CGameMgr)->runMessageLoop();

    SINGLETON(CGameMgr)->Release();

    return ret;
}
