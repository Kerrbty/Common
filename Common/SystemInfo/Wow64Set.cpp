#include "Wow64Set.h"


// 禁/启用系统重定向
typedef BOOLEAN (WINAPI* LPFN_Wow64EnableWow64FsRedirection)( BOOLEAN Wow64FsEnableRedirection );
typedef BOOLEAN (WINAPI* LPFN_Wow64DisableWow64FsRedirection)(PVOID *OldValue);
typedef BOOLEAN (WINAPI* LPFN_Wow64RevertWow64FsRedirection)(PVOID OldValue);
BOOL WINAPI EnableWow64FsRedirection(BOOL enable)  // 启动禁用文件重定向
{
    LPFN_Wow64EnableWow64FsRedirection Wow64EnableWow64FsRedirection = (LPFN_Wow64EnableWow64FsRedirection)
        GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "Wow64EnableWow64FsRedirection");
    if (Wow64EnableWow64FsRedirection != NULL)
    {
        if ( Wow64EnableWow64FsRedirection(enable) )
        {
            return TRUE;
        }
    }
    else
    {
        // 换方法
        static PVOID oldvalue = 0;
        LPFN_Wow64DisableWow64FsRedirection Wow64DisableWow64FsRedirection = (LPFN_Wow64DisableWow64FsRedirection)
            GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "Wow64DisableWow64FsRedirection");
        LPFN_Wow64RevertWow64FsRedirection Wow64RevertWow64FsRedirection = (LPFN_Wow64RevertWow64FsRedirection)
            GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "Wow64RevertWow64FsRedirection");
        if (Wow64RevertWow64FsRedirection != NULL && Wow64DisableWow64FsRedirection != NULL)
        {
            if (enable)
            {
                return Wow64RevertWow64FsRedirection(oldvalue);
            }
            else
            {
                return Wow64DisableWow64FsRedirection(&oldvalue);
            }
        }
    }
    return FALSE;
}


BOOL WINAPI IsWow64()
{
    static BOOL bIsWow64 = FALSE;
    static BOOL initilize = FALSE;
    if ( !initilize )
    {
        typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS)(HANDLE,  PBOOL);
        LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(
            GetModuleHandle(TEXT("kernel32.dll")), "IsWow64Process");
        if (NULL != fnIsWow64Process)
        {
            if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
            {
                bIsWow64 = FALSE;
            }
        }
        initilize = TRUE;
    }
    return bIsWow64;
}