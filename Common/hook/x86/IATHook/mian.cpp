#include "IATHook.h"

VOID WINAPI MyExitProcess( UINT uExitCode )
{
    ExitProcess(uExitCode);
}
                 
BOOL WINAPI DllMain(HINSTANCE hModule, 
                    DWORD dwReason, 
                    LPVOID lpvReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        // 只hook一个模块
        CAPIHook::ReplaceIATEntryInOneMod("kernel32.dll", 
                                        (PROC)GetProcAddress(GetModuleHandle("kernel32.dll"), "ExitProcess"), 
                                        (PROC)MyExitProcess, 
                                        ::GetModuleHandle(NULL));
//         // 全局的hook，且不对自己hook
//         void CAPIHook::ReplaceIATEntryInAllMods( "kernel32.dll", 
//                                                 (PROC)GetProcAddress(GetModuleHandle("kernel32.dll"), "ExitProcess"), 
//                                                 (PROC)MyExitProcess,
//                                                 FALSE);

    }
    return TRUE;
}