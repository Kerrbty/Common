#include "privilege.h"

BOOL EnableDebugPrivilege(BOOL bEnable) 
{ 

	BOOL fOK = FALSE;
	HANDLE hToken;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) //打开进程访问令牌
	{ 
		//试图修改“调试”特权
		TOKEN_PRIVILEGES tp;
		tp.PrivilegeCount = 1;
		LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid);
		tp.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;
		AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
		fOK = (GetLastError() == ERROR_SUCCESS);
		CloseHandle(hToken); 
	} 
	return fOK; 
}


void RaiseToDebug()
{
	HANDLE hToken;
	HANDLE hProcess = GetCurrentProcess();  // 获取当前进程句柄

	// 打开当前进程的Token，就是一个权限令牌，第二个参数可以用TOKEN_ALL_ACCESS
	if (OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		TOKEN_PRIVILEGES tkp;
		if (LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tkp.Privileges[0].Luid))
		{
			tkp.PrivilegeCount = 1;
			tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

			//通知系统修改进程权限
			BOOL bREt = AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, NULL, 0);
		}
		CloseHandle(hToken);
	}   
}