#include <windows.h>
#include <tchar.h>

typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
LPFN_ISWOW64PROCESS fnIsWow64Process;
BOOL IsWow64()
{
    BOOL bIsWow64 = FALSE;

    fnIsWow64Process = (LPFN_ISWOW64PROCESS) 
		GetProcAddress( GetModuleHandle(TEXT("kernel32")), "IsWow64Process" );
	
    if(NULL != fnIsWow64Process)
    {
        if (!fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
        {
            OutputDebugString(TEXT("handle error"));
			return TRUE;
        }
    }
    return bIsWow64;
}

typedef BOOLEAN (WINAPI* pWow64EnableWow64FsRedirection)( BOOLEAN Wow64FsEnableRedirection );
typedef BOOL (WINAPI* pWow64DisableWow64FsRedirection)( PVOID *OldValue );
typedef BOOL (WINAPI* pWow64RevertWow64FsRedirection)( PVOID OldValue );
//////////////////////////////////////////////////////////////////////////
//
// 函数名：EnableWow64FsRedirection
// 参数：  bEnable : FALSE 禁用重定位
//                   TRUE  启动重定位 
//
//////////////////////////////////////////////////////////////////////////
BOOL EnableWow64FsRedirection(BOOL bEnable) 
{
	if ( IsWow64() )
	{
		// The process is running under WOW64.
		pWow64EnableWow64FsRedirection Wow64EnableFs = (pWow64EnableWow64FsRedirection) 
			GetProcAddress( GetModuleHandle(TEXT("kernel32")), "Wow64EnableWow64FsRedirection" );
		if (Wow64EnableFs != NULL)
		{
			// service 2003 sp1,2008 or later/xp x64,vista or later 
			return Wow64EnableFs(bEnable);
		}
		else
		{
			// Vista/Server 2003
			static PVOID OldValue = NULL;
			if (!bEnable)
			{
				// 禁用
				pWow64DisableWow64FsRedirection Wow64Disable = (pWow64DisableWow64FsRedirection)
					GetProcAddress( GetModuleHandle(TEXT("kernel32")), "Wow64DisableWow64FsRedirection" );
				return Wow64Disable(&OldValue);
			}
			else
			{
				// 启用
				pWow64RevertWow64FsRedirection Wow64Enable = (pWow64RevertWow64FsRedirection)
					GetProcAddress( GetModuleHandle(TEXT("kernel32")), "Wow64RevertWow64FsRedirection" );
				return Wow64Enable(OldValue);
			}
		}
	}
	return TRUE;
}

