// Log4Odious.cpp : Defines the entry point for the console application.
//
#include <windows.h>


extern CRITICAL_SECTION  CriticalSection;

BOOL WINAPI DllMain(
                    HINSTANCE hModule,
                    DWORD dwReason,
                    LPVOID lpvReserved
                    )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        InitializeCriticalSection(&CriticalSection);
        DisableThreadLibraryCalls(hModule);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        DeleteCriticalSection(&CriticalSection);
    }
    return TRUE;
}
