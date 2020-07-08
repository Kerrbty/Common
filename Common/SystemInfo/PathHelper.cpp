#include "PathHelper.h"
#include <Tlhelp32.h>
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>

#pragma comment(lib, "kernel32")
#pragma comment(lib, "user32")

typedef LONG NTSTATUS, *PNTSTATUS;

#define _in
#define _out
#define _inout

#define IN
#define OUT
#define INOUT
#define SE_DEBUG_PRIVILEGE                  (20L)

LPTSTR DevicePathNameToDosPathName(const LPTSTR DevicePathName, LPTSTR DosPathName, DWORD dwLen)
{
	TCHAR tmp[4*26+1]; // 26个驱动设备，每个设备4个字符（包括'\0'）
	PTCHAR LogicalDrive = tmp;
	ZeroMemory(LogicalDrive, (4*26+1)*sizeof(TCHAR));
	GetLogicalDriveStrings(MAX_PATH, LogicalDrive);
	do
	{
		ZeroMemory(DosPathName, dwLen*sizeof(TCHAR));
		LogicalDrive[2] = L'\0';
		QueryDosDevice(LogicalDrive, DosPathName, dwLen);
		DWORD dwBytes = _tcslen(DosPathName)*sizeof(TCHAR);
		if ( memicmp(DevicePathName, 
			 DosPathName, 
			 dwBytes) == 0)
		{
			memcpy(DosPathName, LogicalDrive, sizeof(TCHAR)*2 );
			memcpy(&DosPathName[2], DevicePathName+dwBytes, _tcslen(DevicePathName)-dwBytes );
			break;
		}
		LogicalDrive += _tcslen(LogicalDrive) + 2;
	}while(*LogicalDrive != NULL);
	return DosPathName;
}


// 查找最后一个'\'符号位置
LPWSTR FindFileNameFromFullPathW(LPWSTR CommandLine, WCHAR FindWchar)
{
	int Len;
	for ( Len = wcslen(CommandLine) ; Len>0; Len-- )
	{
		if (CommandLine[Len] == FindWchar)
		{
			Len++;
			break;
		}
	}
	return &CommandLine[Len];
}

// 查找最后一个'\'符号位置
LPSTR FindFileNameFromFullPathA(LPSTR CommandLine, CHAR FindChar)
{
	int Len;
	for ( Len = strlen(CommandLine) ; Len>0; Len-- )
	{
		if (CommandLine[Len] == FindChar)
		{
			Len++;
			break;
		}
	}
	return &CommandLine[Len];
}