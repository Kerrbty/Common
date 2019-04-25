// CreateRemoteThread.cpp : 定义控制台应用程序的入口点。
//
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <tchar.h>
#include <TlHelp32.h>
#pragma warning(disable:4996)
typedef INT (pTo_CreateThread)(TCHAR* FileName, DWORD PID);
#ifdef _UNICODE
#define LOADAPI  "LoadLibraryW"
#else
#define LOADAPI  "LoadLibraryA"
#endif

DWORD HookAllprocessByName(LPCTSTR lpszProcessName, TCHAR* FileName, pTo_CreateThread Function)//得到进程ID
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 pe;
	pe.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hSnapshot, &pe)) 
	{
		printf("Process32First 调用失败！\n");
		return 0;
	}

	while (Process32Next(hSnapshot, &pe)) {
		if (!_tcsicmp(lpszProcessName, pe.szExeFile)) {
			Function(FileName, pe.th32ProcessID);
		}
	}

	return 0;
}

BOOL EnableDebugPrivilege(BOOL bEnable = TRUE) 
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

int To_CreateThread(TCHAR* FileName, DWORD PID)
{
	TCHAR buff[MAX_PATH];
	const DWORD dwThreadSize = MAX_PATH;//远程线程申请大小
	
	if(PID == 0)//进程ID error
	{
		printf("process ID error\n");
		return -1;
	}
	
	buff[0] = TEXT('"');
	// 是否是完整路径
	if (FileName[1] != TEXT(':'))
	{
//		GetCurrentDirectory(MAX_PATH, &buff[1]);
		GetCurrentDirectory(MAX_PATH, buff);
		_tcscat(buff, TEXT("\\"));
		_tcscat(buff, FileName);
	}
	else
	{
		_tcscpy(buff, FileName);
	}
//	strcat(buff, "\"");
	_tprintf(TEXT("%s\n"), buff);

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
	
	//远程线程申请内存
	void* pRemoteThread = VirtualAllocEx(hProcess, 0, 
		dwThreadSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (pRemoteThread == NULL)
	{
		printf("申请远程内存失败! %d\n", GetLastError());
		return -1;
	}
	//把线程体写入宿主进程中
	if (!WriteProcessMemory(hProcess, \
		pRemoteThread, &buff, dwThreadSize, 0)) //注入dll
	{
		printf("写入远程线程数据失败! %d", GetLastError());
		return -1;
	}
	//在宿主进程中创建线程
	PTHREAD_START_ROUTINE pfnStartAddr=(PTHREAD_START_ROUTINE)\
			GetProcAddress(GetModuleHandle(TEXT("Kernel32.dll")), LOADAPI);
	if(pfnStartAddr == NULL)
	{
		printf("得到LoadLibrary 地址失败 %d\n", GetLastError());
		return -1;
	}
	
	if( CreateRemoteThread(hProcess, 
						   NULL,
						   0, 
						   pfnStartAddr, 
						   pRemoteThread, 
						   0, 
						   NULL) == NULL)
	{
		printf("创建远程线程失败: %d\n", GetLastError());
		return -1;
	}
	
	return 0;
}


void usage()
{
	printf("CreateRemoteThread /F DllName [ /P ProcessName | /PID PID ]\n\n");
	printf("  /F   需要注入的DLL文件名\n");
	printf("  /P   被注入的进程名\n");
	printf("  /PID 注入的进程ID\n\n");
	printf("CreateRemoteThread /F Insert.dll /P explorer.exe\n");
	printf("CreateRemoteThread /F Insert.dll /PID 504\n");
}

// process  /f filename [ /p process | /pid processID ]
INT _tmain(INT argc, TCHAR* argv[])
{
	TCHAR FileName[MAX_PATH] = {0};
	TCHAR ProcessName[MAX_PATH] = {0};
	DWORD PID = 0; 

	if (argc < 5)
	{
		usage();
		return 0;
	}
	for (int i=1; i<argc; i++)
	{
		if (_tcsicmp(argv[i], TEXT("/p")) == 0 || 
            _tcsicmp(argv[i], TEXT("-p")) == 0)
		{
			_tcscpy( ProcessName, argv[++i] );
		}
		else if ( _tcsicmp(argv[i], TEXT("/pid")) == 0 ||
            _tcsicmp(argv[i], TEXT("-pid")) == 0 )
		{
			PID = _ttoi(argv[++i]);
		}
		else if ( _tcsicmp(argv[i], TEXT("/f")) == 0  ||
            _tcsicmp(argv[i], TEXT("-f")) == 0 )
		{
			_tcscpy(FileName, argv[++i]);
		}
	}
	EnableDebugPrivilege(TRUE);
	RaiseToDebug(); //提权

	if ( PID == 0 )
	{
		PID = HookAllprocessByName(ProcessName, FileName, To_CreateThread);
	}
	else
	{
		To_CreateThread(FileName, PID);
	}
	
	
	return 0;
}

