#include "Unhook.h"
#include <Tlhelp32.h>

///////////////自定义函数实现/////////////////
BOOL PromotePrivilege(BOOL bEnable)
{
	HANDLE hToken;

	if(OpenProcessToken(::GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) 
	{
		LUID uID;
		if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &uID))
		{
			return FALSE;
		}
		TOKEN_PRIVILEGES tp;
		tp.PrivilegeCount = 1;
		tp.Privileges[0].Luid = uID;
		tp.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;
		if(!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL,NULL))
		{
			return FALSE;
		}
		CloseHandle(hToken);
		return TRUE;
	}
	return FALSE;
}

DWORD WINAPI MyUnHookAll(LPVOID lparam)
{
	PromotePrivilege(TRUE);
    DWORD InlineHookCount = 0;
    DWORD IATCount = 0;

	HANDLE hSnapshot = NULL;
	MODULEENTRY32 me32 ={0};
	me32.dwSize = sizeof (me32);

	hSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE, GetCurrentProcessId() );
	if (hSnapshot == INVALID_HANDLE_VALUE)  
	{
		return -1;  
 	}  
	if(Module32First(hSnapshot, &me32))
	{
		do 
		{
			InlineHookCount += DelInlineHook(me32.hModule, me32.szModule);
            IATCount += DelIATHook(me32.hModule);
		} while ( Module32Next(hSnapshot, &me32) );
	}
    
    LPTSTR  buf = new TCHAR[40];
    wsprintf(buf, TEXT("Inline Hook Count:%d\n"), InlineHookCount);
    OutputDebugString(buf);
    
    wsprintf(buf, TEXT("IAT Hook Count:%d\n"), IATCount);
    OutputDebugString(buf);

    delete []buf;
	
	return 0;
}

BOOL APIENTRY DllMain( HANDLE hModule, 
					  DWORD  ul_reason_for_call, 
					  LPVOID lpReserved
					  )
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			DisableThreadLibraryCalls((HINSTANCE)hModule);
			CloseHandle( CreateThread(NULL, 0, MyUnHookAll, NULL, 0, NULL) );
			break;
    }
    return TRUE;
}