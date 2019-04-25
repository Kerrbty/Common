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
        // ֻhookһ��ģ��
        CAPIHook::ReplaceIATEntryInOneMod("kernel32.dll", 
                                        (PROC)GetProcAddress(GetModuleHandle("kernel32.dll"), "ExitProcess"), 
                                        (PROC)MyExitProcess, 
                                        ::GetModuleHandle(NULL));
//         // ȫ�ֵ�hook���Ҳ����Լ�hook
//         void CAPIHook::ReplaceIATEntryInAllMods( "kernel32.dll", 
//                                                 (PROC)GetProcAddress(GetModuleHandle("kernel32.dll"), "ExitProcess"), 
//                                                 (PROC)MyExitProcess,
//                                                 FALSE);

    }
    return TRUE;
}