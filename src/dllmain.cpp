#include "MenuUI.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
       MenuUi::InjectPatches();
    }
    return TRUE;
}