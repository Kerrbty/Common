// CreateRemoteThread.cpp : �������̨Ӧ�ó������ڵ㡣
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

DWORD HookAllprocessByName(LPCTSTR lpszProcessName, TCHAR* FileName, pTo_CreateThread Function)//�õ�����ID
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 pe;
	pe.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hSnapshot, &pe)) 
	{
		printf("Process32First ����ʧ�ܣ�\n");
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
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) //�򿪽��̷�������
	{ 
		//��ͼ�޸ġ����ԡ���Ȩ
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
    HANDLE hProcess = GetCurrentProcess();  // ��ȡ��ǰ���̾��
	
    // �򿪵�ǰ���̵�Token������һ��Ȩ�����ƣ��ڶ�������������TOKEN_ALL_ACCESS
    if (OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {
        TOKEN_PRIVILEGES tkp;
        if (LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tkp.Privileges[0].Luid))
        {
            tkp.PrivilegeCount = 1;
            tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
			
            //֪ͨϵͳ�޸Ľ���Ȩ��
            BOOL bREt = AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, NULL, 0);
        }
        CloseHandle(hToken);
    }   
}

int To_CreateThread(TCHAR* FileName, DWORD PID)
{
	TCHAR buff[MAX_PATH];
	const DWORD dwThreadSize = MAX_PATH;//Զ���߳������С
	
	if(PID == 0)//����ID error
	{
		printf("process ID error\n");
		return -1;
	}
	
	buff[0] = TEXT('"');
	// �Ƿ�������·��
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
	
	//Զ���߳������ڴ�
	void* pRemoteThread = VirtualAllocEx(hProcess, 0, 
		dwThreadSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (pRemoteThread == NULL)
	{
		printf("����Զ���ڴ�ʧ��! %d\n", GetLastError());
		return -1;
	}
	//���߳���д������������
	if (!WriteProcessMemory(hProcess, \
		pRemoteThread, &buff, dwThreadSize, 0)) //ע��dll
	{
		printf("д��Զ���߳�����ʧ��! %d", GetLastError());
		return -1;
	}
	//�����������д����߳�
	PTHREAD_START_ROUTINE pfnStartAddr=(PTHREAD_START_ROUTINE)\
			GetProcAddress(GetModuleHandle(TEXT("Kernel32.dll")), LOADAPI);
	if(pfnStartAddr == NULL)
	{
		printf("�õ�LoadLibrary ��ַʧ�� %d\n", GetLastError());
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
		printf("����Զ���߳�ʧ��: %d\n", GetLastError());
		return -1;
	}
	
	return 0;
}


void usage()
{
	printf("CreateRemoteThread /F DllName [ /P ProcessName | /PID PID ]\n\n");
	printf("  /F   ��Ҫע���DLL�ļ���\n");
	printf("  /P   ��ע��Ľ�����\n");
	printf("  /PID ע��Ľ���ID\n\n");
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
	RaiseToDebug(); //��Ȩ

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

