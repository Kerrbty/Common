#include "stdafx.h"

#include "cfile.h"
#include "ntdll.h"
#include <Psapi.h>
#include <tchar.h>
#include <stdio.h>
#pragma comment(lib, "Psapi.lib")

BOOL GetFileNameByHandle(HANDLE hFile, LPTSTR buff, DWORD size)
{
	BOOL retval = FALSE;  
    if (buff == NULL || size == 0)
    {
        return 0;
    }
    HANDLE hfilemap = CreateFileMapping(hFile, NULL, PAGE_READONLY, NULL, NULL, NULL);
    if(INVALID_HANDLE_VALUE == hfilemap)
    {
        return retval;
    }
	
	LPVOID lpmap = MapViewOfFile(hfilemap, FILE_MAP_READ, NULL, NULL, 0);
	do 
	{
		
		if( NULL == lpmap )
		{
			break;
		}
		
		DWORD length = GetMappedFileName(GetCurrentProcess(), lpmap, buff, size);
		if(0 == length)
		{
			break;			
		}

        retval = TRUE;
		TCHAR* DosPath = new TCHAR[MAX_PATH];
		TCHAR* DriverString = new TCHAR[MAX_PATH];
		
        ZeroMemory(DriverString, MAX_PATH*sizeof(TCHAR));
		GetLogicalDriveStrings(MAX_PATH, DriverString);
		TCHAR * p = (TCHAR*)DriverString;  //p用来指向盘符
		do
		{
            p[2] = TEXT('\0');
			QueryDosDevice((LPCTSTR)p, DosPath, MAX_PATH);
			if(memicmp(DosPath, buff, _tcslen(DosPath)*sizeof(TCHAR)) == 0)
			{
                _tcscpy(buff, p);
				_tcscpy(buff+_tcslen(p), buff+_tcslen(DosPath));  //连接路径
				break;
			}
			p += _tcslen(p)+2;
		}while(*p!=0);
		delete []DosPath;
        delete []DriverString;
		
	} while (0);

	if (lpmap)
	{
		UnmapViewOfFile(lpmap);
	}
	CloseHandle(hfilemap);
	return retval;
}


BOOL GetProcessKernelObjectName(HANDLE hHandleObjKernel, PTSTR namebuffer, DWORD size)
{
    namebuffer[0] = 0;
    ULONG nNumberHandle = 0;
    NTSTATUS ntStatus = 0;
    ULONG ulSize;
    BOOL ret = FALSE;
    
    PTSTR namebuf = new TCHAR[MAX_PATH+18];
    do
    {
        
        if (namebuf == NULL)
        {
            break;
        }
        ntStatus = ZwQueryObject(hHandleObjKernel, ObjectNameInformation, namebuf, MAX_PATH, &ulSize);
        if(!NT_SUCCESS(ntStatus))
        {
            break;
        }
        POBJECT_NAME_INFORMATION pNameInfo = (POBJECT_NAME_INFORMATION)namebuf;
        UNICODE_STRING str1 = pNameInfo->Name;

        LPTSTR szFileName = new TCHAR[MAX_PATH+10];
        ZeroMemory(szFileName, MAX_PATH*sizeof(TCHAR));

#ifndef _UNICODE
        ANSI_STRING  AnsiStr = {0};
        AnsiStr.Buffer = szFileName;
        AnsiStr.MaximumLength = MAX_PATH;
        RtlUnicodeStringToAnsiString(&AnsiStr, &str1, FALSE);
#else
        wcsncpy(szFileName, str1.Buffer, str1.Length/2);
#endif	
        _tcscpy(namebuf, szFileName);
        delete []szFileName;

        ret = TRUE;
        TCHAR* DosPath = new TCHAR[MAX_PATH];
        TCHAR* DriverString = new TCHAR[MAX_PATH];

        ZeroMemory(DriverString, MAX_PATH*sizeof(TCHAR));
        GetLogicalDriveStrings(MAX_PATH, DriverString);
        TCHAR * p = (TCHAR*)DriverString;  //p用来指向盘符
        do
        {
            p[2] = TEXT('\0');
            QueryDosDevice((LPCTSTR)p, DosPath, MAX_PATH);
            if(memicmp(DosPath, namebuf, _tcslen(DosPath)*sizeof(TCHAR)) == 0)
            {
                _tcscpy(namebuf, p);
                _tcscpy(namebuf+_tcslen(p), namebuf+_tcslen(DosPath));  //连接路径
                break;
            }
            p += _tcslen(p)+2;
        }while(*p!=0);
        delete []DosPath;
        delete []DriverString;

    } while (0);

    if (ret)
    {
        _tcscpy(namebuffer, namebuf);
    }
    delete []namebuf;
	return ret;
}
