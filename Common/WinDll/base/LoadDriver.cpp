#include "LoadDriver.h"
#include "ntdll.h"
#include "StringForm.h"
#include "log.h"
#include <stdio.h>
#include <tchar.h>
#include "privilege.h"


//  ����ɾ��ע����ֵ���Ӽ�
VOID DeleteKey(HKEY hkey)
{
	HKEY phkey;
	LPTSTR lpName = new TCHAR[MAX_PATH];
	FILETIME ftime;
	DWORD lenth = MAX_PATH;
	
	// ɾ���Ӽ�
	while (ERROR_SUCCESS == RegEnumKeyEx(hkey, 
										0, 
										lpName, 
										&lenth, 
										NULL, 
										NULL, 
										NULL, 
										&ftime))
	{
		if (ERROR_SUCCESS == RegOpenKeyEx(	hkey, 
											lpName, 
											NULL, 
											KEY_ALL_ACCESS, 
											&phkey))
		{
			DeleteKey(phkey);
			RegCloseKey(phkey);
			if( RegDeleteKey(hkey, lpName) !=  ERROR_SUCCESS)
			{
#ifdef _DEBUG
				DWORD ErrorCode = GetLastError();
				_tprintf( TEXT("Delete the Key:%s Error: "), lpName );
				PrintErrorMsg(ErrorCode);
#endif
			}
		}
		lenth = MAX_PATH;
	}

	// ɾ���˼�����ֵ
	lenth = MAX_PATH;
	while (ERROR_SUCCESS == RegEnumValue(	hkey, 
											0, 
											lpName, 
											&lenth, 
											NULL, 
											NULL, 
											NULL, 
											NULL))
	{
		if( RegDeleteValue(hkey, lpName) !=  ERROR_SUCCESS)
		{
#ifdef _DEBUG
			DWORD ErrorCode = GetLastError();
			_tprintf( TEXT("Delete the Key:%s Error: "), lpName );
			PrintErrorMsg(ErrorCode);
#endif
		}
		lenth = MAX_PATH;
	}

	delete lpName;
}

// ��������Ļ��¼
BOOL CleanDriverInfoW(PTSTR DriverSavePath, PWSTR ServiceName)
{
	HKEY hkey;
	WCHAR RegPathName[MAX_PATH];
//	DeleteFile(DriverSavePath);
	BOOL b;
	
	UNICODE_STRING uDriverReg = {0};
	wsprintfW(RegPathName, L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\%s", ServiceName);
	RtlInitUnicodeString(&uDriverReg, RegPathName);
	NtUnloadDriver(&uDriverReg);

	wsprintfW(RegPathName, L"SYSTEM\\CurrentControlSet\\Services\\%s", ServiceName);
	if( ERROR_SUCCESS == RegOpenKeyExW(HKEY_LOCAL_MACHINE,
										RegPathName,
										0,
										KEY_ALL_ACCESS,
										&hkey) )
	{
		// ����ɾ���Ӽ�
		DeleteKey(hkey);
		
		b = ( RegDeleteKeyW(HKEY_LOCAL_MACHINE, RegPathName) == ERROR_SUCCESS );
		RegCloseKey(hkey);
	}
	return b;
}


// ��������Ļ��¼
BOOL CleanDriverInfoA(PTSTR DriverSavePath, PSTR ServiceName)
{
	BOOL b;
	PWSTR NewSerName = MulToWide(ServiceName);
	b = CleanDriverInfoW(DriverSavePath, NewSerName);
	HeapFree(GetProcessHeap(), 0, NewSerName); // �ͷ�MulToWide������ڴ�
	return b;
}

// �������������ڴ�
BOOL LoadDriverW(PWSTR DriverSavePath, PWSTR ServiceName)
{
	WCHAR RegPathName[MAX_PATH];
	EnableLoadDriverPrivilege(TRUE);

	HKEY hkey;
	wsprintfW(RegPathName, L"SYSTEM\\CurrentControlSet\\Services\\%s", ServiceName);
	if( ERROR_SUCCESS != RegCreateKeyExW(	HKEY_LOCAL_MACHINE, 
											RegPathName, 
											0,
											NULL,
											REG_OPTION_NON_VOLATILE,
											KEY_ALL_ACCESS,
											NULL,
											&hkey,
											NULL))
	{
		
#ifdef _DEBUG
			DWORD ErrorCode = GetLastError();
			_tprintf( TEXT("error in RegCreateKeyEx: ") );
			PrintErrorMsg(ErrorCode);
#endif

	}

	DWORD dwType = REG_SZ;
	if( ERROR_SUCCESS != RegSetValueEx(hkey, 
										TEXT("Type"),
										0,
										REG_DWORD,
										(CONST BYTE *)&dwType,
										4) )
	{
#ifdef _DEBUG
			DWORD ErrorCode = GetLastError();
			_tprintf( TEXT("error in RegCreateKeyEx: ") );
			PrintErrorMsg(ErrorCode);
#endif
	}

	UNICODE_STRING uDriverReg = {0};
	RtlDosPathNameToNtPathName_U(DriverSavePath, &uDriverReg, NULL, NULL);

	if( ERROR_SUCCESS != RegSetValueExW(hkey, 
										L"ImagePath",
										0,
										REG_EXPAND_SZ,
										(CONST BYTE *)uDriverReg.Buffer,
										uDriverReg.Length) )
	{
#ifdef _DEBUG
			DWORD ErrorCode = GetLastError();
            RtlFreeUnicodeString(&uDriverReg);
			_tprintf( TEXT("error in RegCreateKeyEx: ") );
			PrintErrorMsg(ErrorCode);
#endif
	}

    RtlFreeUnicodeString(&uDriverReg);
	dwType = 3; // ������ʽ
	if( ERROR_SUCCESS != RegSetValueEx(hkey, 
										TEXT("Start"),
										0,
										REG_DWORD,
										(CONST BYTE *)&dwType,
										4) )
	{
#ifdef _DEBUG
			DWORD ErrorCode = GetLastError();
			_tprintf( TEXT("error in RegCreateKeyEx: ") );
			PrintErrorMsg(ErrorCode);
#endif
	}

	wsprintfW(RegPathName, L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\%s", ServiceName);

	RtlInitUnicodeString(&uDriverReg, RegPathName);

	if( NtLoadDriver(&uDriverReg) )
	{
#ifdef _DEBUG
		_tprintf( TEXT("Loader Driver Error\n") );
#endif
	}
	RegCloseKey(hkey);

	return TRUE;
}



// �������������ڴ�
BOOL LoadDriverA(PSTR DriverSavePath, PSTR ServiceName)
{
	BOOL b;
	
	// ת������
	PWSTR NewDriverSavePath = MulToWide(DriverSavePath);
	PWSTR NewServiceName = MulToWide(ServiceName);
	
	b = LoadDriverW(NewDriverSavePath, NewServiceName);

	// �ͷ�MulToWide������ڴ�
	HeapFree(GetProcessHeap(), 0, NewDriverSavePath); 
	HeapFree(GetProcessHeap(), 0, NewServiceName); 
	
	return b;
}