#include "FsWow64.h"

typedef BOOL (WINAPI *pDisibleFsWow64)( OUT PVOID *OldValue);
typedef BOOL (WINAPI *pEnableFsWow64)( IN PVOID OldValue);
typedef BOOL (WINAPI *pChangeFsWow64)(BOOLEAN Wow64FsEnableRedirection);


BOOL EnableFsWow64Set(BOOL ShutdownFsWow64)
{

    HMODULE hmodule = GetModuleHandle(TEXT("kernel32.dll"));
    pChangeFsWow64 ChangeToEnableFsWow64 = (pChangeFsWow64)GetProcAddress(hmodule, "Wow64EnableWow64FsRedirection");
    if (ChangeToEnableFsWow64 != NULL)
    {
        return ChangeToEnableFsWow64(ShutdownFsWow64);
    }

    static PVOID OldValue = 0;
    if (ShutdownFsWow64)
    {
        pEnableFsWow64 EnableFsWow64 = (pEnableFsWow64)GetProcAddress(hmodule, "Wow64RevertWow64FsRedirection");
        return EnableFsWow64(OldValue);
    }
    else
    {
        pDisibleFsWow64 DisibleFsWow64 = (pDisibleFsWow64)GetProcAddress(hmodule, "Wow64DisableWow64FsRedirection");
        return DisibleFsWow64(&OldValue);
    }
}