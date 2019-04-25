#include "GetProcCommand.h"

BOOL EnableDebugPrivilege(BOOL bEnable) 
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

BOOL GetCommandLineW(DWORD pid, LPWSTR CmdLns, DWORD dwlenth)
{
	BOOL retval = FALSE;
	HMODULE hmodule = LoadLibrary(TEXT("kernelBase.dll"));
	if (hmodule == NULL)
	{
		hmodule = LoadLibrary(TEXT("kernel32.dll"));
	}
	PBYTE commandline = (PBYTE)GetProcAddress(hmodule, "GetCommandLineW");
	memset(CmdLns, 0, dwlenth*sizeof(WCHAR));
	if (commandline != NULL && commandline[0] == 0xA1) // mov eax,[0xXXXXXXX]
	{
		EnableDebugPrivilege(TRUE);
		HANDLE handle = OpenProcess(PROCESS_VM_READ, FALSE, pid);
		if (!handle)
		{
			return retval;
		}
		DWORD dwBytes = 0;
		DWORD newaddr = 0;
		if( ReadProcessMemory(handle, (LPVOID)(*(LPDWORD)(commandline+1)), &newaddr, 4, &dwBytes) && dwBytes > 0 )
		{
			retval = ReadProcessMemory(handle, (LPVOID)newaddr, CmdLns, dwlenth, &dwBytes);
		}
		CloseHandle(handle);
	}
	return retval;
}


BOOL GetCommandLineA(DWORD pid, LPSTR CmdLns, DWORD dwlenth)
{
	BOOL retval = FALSE;
	
	// ����1048���ַ��������о�����
	LPWSTR CMDLINES = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 1048*sizeof(WCHAR));
	retval = GetCommandLineW(pid, CMDLINES, 1048);
	if (retval)
	{
		WideCharToMultiByte( CP_ACP,
			0,
			CMDLINES,
			-1,
			CmdLns,
			dwlenth,
			NULL,
			NULL);
	}
	HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, CMDLINES);
	return retval;
}
