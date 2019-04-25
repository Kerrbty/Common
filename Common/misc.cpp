#include "stdafx.h"
#include <tchar.h>
#include "misc.h"
#include <stdio.h>
#include <basetsd.h>
#include <DbgHelp.h>
#include <Tlhelp32.h>
#include "StringForm.h"
// #pragma comment(lib, "Tlhelp32")
#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "Comdlg32.lib")
#define BUFFER_LENGTH (1024)

// VS2008 去警告
#pragma warning (disable:4996)   // 字符串不安全函数警告
#pragma warning (disable:4018)   // 有符号与无符号比较警告

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  异常类函数
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// 写奔溃dump
BOOL WINAPI WriteMiniDump(PEXCEPTION_POINTERS ExceptionInfo)
{
	// EXCEPTION_CONTINUE_SEARCH means it continue to 
	// execute subsequent exception handlers.
	BOOL MemFormHeap = TRUE;

	///////////////////////////////////////////////////////////////////////////////////////////////////
	//  说明：用不同的方式申请内存,是为了防止堆内存被破坏情况下出现的异常问题,也可以正确的记录异常信息
	///////////////////////////////////////////////////////////////////////////////////////////////////
	LPTSTR szAppPath = (LPTSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_PATH*2*sizeof(TCHAR)); 
	if (szAppPath == NULL)
	{
		MemFormHeap = FALSE;
		szAppPath = new TCHAR[MAX_PATH+30];
	}
	int length = MAX_PATH;
	GetAppPath(szAppPath, &length);
	//创建dump文件夹
	_tcscat(szAppPath, _T("\\dump"));
	if (!IsFileExists(szAppPath))
		CreateDirectory(szAppPath, NULL);

	// 凑文件名
	TCHAR szId[64] = {0};
	SYSTEMTIME SysTimer = {0};
	GetLocalTime(&SysTimer);
	_stprintf(szId, 
		_T("\\%02d-%02d-%02d%02d%02d.dmp"), 
		SysTimer.wMonth, 
		SysTimer.wDay, 
		SysTimer.wHour,  
		SysTimer.wMinute,
		SysTimer.wSecond
		);
	_tcscat(szAppPath, szId);
	OutputDebugString(szAppPath);
	
	// 写dump
	HANDLE hDumpFile;
	hDumpFile = CreateFile(
		szAppPath, 
		GENERIC_WRITE, 
		FILE_SHARE_READ, 
		NULL, 
		CREATE_ALWAYS, 
		FILE_ATTRIBUTE_NORMAL, 
		NULL);
	if (MemFormHeap)
		HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, szAppPath);
	else 
		delete []szAppPath;

	if (hDumpFile != INVALID_HANDLE_VALUE)
	{
		MINIDUMP_EXCEPTION_INFORMATION ExInfo;

		ExInfo.ThreadId = GetCurrentThreadId();
		ExInfo.ExceptionPointers = ExceptionInfo;
		ExInfo.ClientPointers = TRUE;

		// Write the information into the dump
		if (MiniDumpWriteDump(
			GetCurrentProcess(), // Handle of process
			GetCurrentProcessId(), // Process Id
			hDumpFile,    // Handle of dump file
			MiniDumpNormal,   // Dump Level: Mini
			&ExInfo,    // Exception information
			NULL,     // User stream parameter
			NULL))     // Callback Parameter
		{
			CloseHandle(hDumpFile);
			return TRUE;
		}
		CloseHandle(hDumpFile);
	}
	return FALSE;
}

// 其他异常捕获都无法处理的情况下，最后的异常处理函数，调用写dump函数
LONG WINAPI BrushUnhandledExceptionFilter(PEXCEPTION_POINTERS pe)
{
	switch (pe->ExceptionRecord->ExceptionCode)
	{
	case EXCEPTION_ACCESS_VIOLATION:
//		FilePrint(_T("内存访问异常")); 
//		break; 
	default:
		FilePrint(_T("异常代码 = %d"), pe->ExceptionRecord->ExceptionCode);
		break;
	}
	// MiniDumpWriteDump
	FilePrint(_T("Program is error!, write dump file.\r\n"));

	__try
	{
		WriteMiniDump(pe);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	}

	TerminateProcess(GetCurrentProcess(), 0);
	return ExecuteHandler;
}


// 异常处理设置函数（HOOK函数）
LPTOP_LEVEL_EXCEPTION_FILTER mOldFilter = NULL;
F_SetUnhandledExceptionFilter pOldSetUnhandledExceptionFilter = SetUnhandledExceptionFilter;

LPTOP_LEVEL_EXCEPTION_FILTER WINAPI HookSetUnhandledExceptionFilter(
	LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter )
{
	if ( lpTopLevelExceptionFilter != BrushUnhandledExceptionFilter )
	{
		return BrushUnhandledExceptionFilter;
	}
	return pOldSetUnhandledExceptionFilter(BrushUnhandledExceptionFilter);
}


void WriteCurrentModule(HANDLE pFile)
{
	BOOL bFound = FALSE;     
	HANDLE hModuleSnap  = NULL;     
	MODULEENTRY32 me32 = {0};     
	hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());     
	if   (hModuleSnap == INVALID_HANDLE_VALUE)    
	{
		return ;
	}

	BOOL MemFormHeap = TRUE;
	me32.dwSize = sizeof(MODULEENTRY32);     
	PSTR szStr = (PSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, BUFFER_LENGTH*sizeof(CHAR));
	if (szStr == NULL)
	{
		MemFormHeap = FALSE;
		szStr = new char[BUFFER_LENGTH];
	}
	if   (Module32First(hModuleSnap, &me32))     
	{     
		do     
		{ 
#ifdef _UNICODE
			PSTR module = WideToMul(me32.szModule);
#else
			PSTR module = me32.szModule;
#endif // _UNICODE

			wsprintfA(szStr, "Start:0x%08X,  End:0x%08X, Module:%s\r\n", me32.modBaseAddr, me32.modBaseAddr+me32.modBaseSize, module);
            if (pFile != INVALID_HANDLE_VALUE)
            {
                DWORD dwBytes;
                WriteFile(pFile, szStr, strlen(szStr), &dwBytes, NULL);
            }

#ifdef _UNICODE
			HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, module);
#endif // _UNICODE
		} while (Module32Next(hModuleSnap, &me32));     
	}  

	if (MemFormHeap)
		HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, szStr);
	else
		delete []szStr;
	CloseHandle(hModuleSnap);     
}

// 异常处理，写错误日志报告
// __except 中从 GetExceptionInformation(); 得到 LPEXCEPTION_POINTERS 结构指针
// __except( ErrorReport(GetExceptionInformation()) )
#ifdef WIN64
int ErrorReport(LPEXCEPTION_POINTERS p_exinfo)
{
    return 0;
}
#else /* WIN64 */
int ErrorReport(LPEXCEPTION_POINTERS p_exinfo)
{
	// 堆被破坏的情况下，也要能正常工作才对
	BOOL MemFormHeap = TRUE;
	LPTSTR szAppPath = (LPTSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_PATH*2*sizeof(TCHAR)); 
	if (szAppPath == NULL)
	{
		MemFormHeap = FALSE;
		szAppPath = new TCHAR[MAX_PATH+38];
	}
	int length = MAX_PATH;
	GetAppPath(szAppPath, &length);
	//创建dump文件夹
	_tcscat(szAppPath, _T("\\ErrorReport"));
	if (!IsFileExists(szAppPath))
		CreateDirectory(szAppPath, NULL);

	// 凑文件名
	TCHAR szId[64] = {0};
	SYSTEMTIME SysTimer = {0};
	GetLocalTime(&SysTimer);
	_stprintf(szId, 
		_T("\\%02d-%02d-%02d%02d%02d.txt"), 
		SysTimer.wMonth, 
		SysTimer.wDay, 
		SysTimer.wHour,  
		SysTimer.wMinute,
		SysTimer.wSecond
		);
	_tcscat(szAppPath, szId);

	// 保存错误信息
    HANDLE pFile = CreateFile(szAppPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); // _tfopen(szAppPath, TEXT("ab"));
    SetFilePointer(pFile, 0, NULL, FILE_END);
	if (MemFormHeap)
		HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, szAppPath);
	else
		delete []szAppPath;
	WriteCurrentModule(pFile);

	char* errinfo = NULL;
	if (MemFormHeap)
		errinfo = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, BUFFER_LENGTH);
	else
		errinfo = new char[BUFFER_LENGTH];

	wsprintfA(errinfo, 
		"\r\n"
		"Except Info:\r\n"
		"Address:0x%08X, ErrorCode:0x%08X\r\n"
		"EAX:%08X, EBX:%08X, ECX:%08X, EDX:%08X, EBP:%08X\r\n"
		"ESP:%08X, EDI:%08X, ESI:%08X, EIP:%08X\r\n\r\n", 
		p_exinfo->ExceptionRecord->ExceptionAddress, 
		p_exinfo->ExceptionRecord->ExceptionCode,
		p_exinfo->ContextRecord->Eax,
		p_exinfo->ContextRecord->Ebx,
		p_exinfo->ContextRecord->Ecx,
		p_exinfo->ContextRecord->Edx,
		p_exinfo->ContextRecord->Ebp,
		p_exinfo->ContextRecord->Esp,
		p_exinfo->ContextRecord->Edi,
		p_exinfo->ContextRecord->Esi,
		p_exinfo->ContextRecord->Eip);

    DWORD dwBytes;
    WriteFile(pFile, errinfo, strlen(errinfo), &dwBytes, NULL);
    CloseHandle(pFile);

	if (MemFormHeap)
		HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, errinfo);
	else
		delete []errinfo;
	return EXCEPTION_EXECUTE_HANDLER;
}

#endif  /* WIN64 */


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  调试信息打印类
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef MYDEBUG

void DebugPrintA(LPCSTR lpszFormant, ...)
{
	LPSTR szBuffer = new CHAR[1024];
	va_list args;
	va_start(args, lpszFormant);
	vsprintf(szBuffer, lpszFormant, args);
	va_end(args);
//	strcat(szBuffer, "\n");
	OutputDebugStringA(szBuffer);
	delete []szBuffer;
}

void DebugPrintW(LPCWSTR lpszFormant, ...)
{
	LPWSTR szBuffer = new WCHAR[1024];
	va_list args;
	va_start(args, lpszFormant);
	vswprintf(szBuffer, lpszFormant, args);
	va_end(args);
//	wcscat(szBuffer, L"\n");
	OutputDebugStringW(szBuffer);
	delete []szBuffer;
}


void FilePrintA(LPCSTR lpszFormant, ...)
{
	if ( 
		!DebugInfoSet(_T("DebugString"), ODIN_SETTING) &&
		!DebugInfoSet(_T("DebugString"), MTK_SETTING)
		)
	{
		return;
	}

	__try
	{
		char* szBuffer = new char[BUFFER_LENGTH];
		va_list args;
		va_start(args, lpszFormant);
		vsprintf(szBuffer, lpszFormant, args);
		va_end(args);
		OutputDebugStringA(szBuffer);

		TCHAR* szAppPath = new TCHAR[MAX_PATH+12];
		int length = MAX_PATH;
		GetAppPath(szAppPath, &length);
		_tcscat(szAppPath, _T("\\debug.txt"));
		strcat(szBuffer, "\r\n");
	
#ifdef _USR_HOOK_FUNCTION_

		HANDLE hFile = CreateFile(szAppPath, 
			GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,  0, NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			SetFilePointer(hFile, 0, NULL, FILE_END);
			DWORD dwWritten = 0;
			WriteFile(hFile, szBuffer, strlen(szBuffer), &dwWritten, NULL);
			CloseHandle(hFile);
		}
		else
		{
			HANDLE pFile = CreateFile(szAppPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); 
            SetFilePointer(hFile, 0, NULL, FILE_END);
            DWORD dwBytes;
            WriteFile(pFile, szBuffer, strlen(szBuffer), &dwBytes, NULL);
            CloseHandle(pFile);
		}

#else
		HANDLE pFile = CreateFile(szAppPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); // FILE * pFile = _tfopen(szAppPath, _T("ab"));
        SetFilePointer(pFile, 0, NULL, FILE_END);
        DWORD dwBytes;
        WriteFile(pFile, szBuffer, strlen(szBuffer), &dwBytes, NULL);	
        CloseHandle(pFile);
#endif
		delete []szAppPath;
		delete []szBuffer;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	}
}

void FilePrintW(LPCWSTR lpszFormant, ...)
{
	if ( 
		!DebugInfoSet(_T("DebugString"), ODIN_SETTING) &&
		!DebugInfoSet(_T("DebugString"), MTK_SETTING)
		)
	{
		return;
	}

	__try
	{
		WCHAR* szBuffer = new WCHAR[BUFFER_LENGTH];
		va_list args;
		va_start(args, lpszFormant);
		vswprintf(szBuffer, lpszFormant, args);
		va_end(args);
		OutputDebugStringW(szBuffer);

		TCHAR* szAppPath = new TCHAR[MAX_PATH+12];
		int length = MAX_PATH;
		GetAppPath(szAppPath, &length);
		_tcscat(szAppPath, TEXT("\\debug.txt"));
		char* szDes = new char[BUFFER_LENGTH];
		WideCharToMultiByte(CP_ACP, 0, szBuffer, -1, szDes, BUFFER_LENGTH, NULL, NULL);
		strcat(szDes, "\r\n");
		delete []szBuffer;
	
#ifdef _USR_HOOK_FUNCTION_
		HANDLE hFile = CreateFile(szAppPath, 
			GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,  0, NULL);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			SetFilePointer(hFile, 0, NULL, FILE_END);
			DWORD dwWritten = 0;
			WriteFile(hFile, szDes, strlen(szDes), &dwWritten, NULL);
			CloseHandle(hFile);
		}
		else
		{
			HANDLE pFile = CreateFile(szAppPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); // FILE * pFile = _tfopen(szAppPath, _T("ab"));
            SetFilePointer(pFile, 0, NULL, FILE_END);
            DWORD dwBytes;
            WriteFile(pFile, szDes, strlen(szDes), &dwBytes, NULL);
            CloseHandle(pFile);
		}
#else
		HANDLE pFile = CreateFile(szAppPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); // FILE * pFile = _tfopen(szAppPath, _T("ab"));
        SetFilePointer(pFile, 0, NULL, FILE_END);
        DWORD dwBytes;
        WriteFile(pFile, szDes, strlen(szDes), &dwBytes, NULL);
        CloseHandle(pFile);
		
#endif
		delete []szAppPath;
		delete []szDes;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	}
}

#endif

void FilePrint2(int id, LPCTSTR lpszFormant, LPCTSTR lpszFileName)
{
	__try
	{
		LPTSTR szAppPath = new TCHAR[MAX_PATH+18];
		int length = MAX_PATH;
		GetAppPath(szAppPath, &length);
		//创建msgs文件夹
		_tcscat(szAppPath, _T("\\msgs"));
		if (!IsFileExists(szAppPath))
			CreateDirectory(szAppPath, NULL);
		TCHAR szId[24];
		_stprintf(szId, _T("\\%d.log"), id);
		_tcscat(szAppPath, szId);
// 		_tcscat(szAppPath, _T("\\debug.txt"));

#ifdef _UNICODE
		int len = WideCharToMultiByte(CP_ACP, 0, lpszFormant, -1, NULL, 0, NULL, NULL);
		char * pszDes = new char[len + 1];
		WideCharToMultiByte(CP_ACP, 0, lpszFormant, -1, pszDes, len , NULL, NULL);
#else
		int len = strlen(lpszFormant);
		char * pszDes = new char[len+1];
		strcpy(pszDes, lpszFormant);
#endif
	//	strcat(szDes, "\r\n");
#ifdef _USR_HOOK_FUNCTION_
		HANDLE hFile = CreateFile(szAppPath, 
            GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,  0, NULL);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			DWORD dwWritten = 0;
			for(int it = 0;it != len;it++){
				int ch = pszDes[it];
				pszDes[it] = ch ^ 100;
			}
			WriteFile(hFile, pszDes, len, &dwWritten, NULL);
			CloseHandle(hFile);
		}
		else
		{
			HANDLE pFile = CreateFile(szAppPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); // FILE * pFile = _tfopen(szAppPath, _T("wb"));
			for(int it = 0;it != len;it++){
				int ch = pszDes[it];
				pszDes[it] = ch ^ 100;
			}
            DWORD dwBytes;
            WriteFile(pFile, pszDes, strlen(pszDes), &dwBytes, NULL);
            CloseHandle(pFile);		
		}
#else
		HANDLE pFile = CreateFile(szAppPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); // FILE * pFile = _tfopen(szAppPath, _T("wb"));
// 		for(int it = 0;it != len;it++){
// 			int ch = pszDes[it];
// 			pszDes[it] = ch ^ 100;
// 		}
        DWORD dwBytes;
        WriteFile(pFile, pszDes, strlen(pszDes), &dwBytes, NULL);
        CloseHandle(pFile);
#endif

		delete [] pszDes;
		delete [] szAppPath;
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	}
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 路径字符串处理
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// 匹配str字符串是否与match字符串前strlen(str)个字符相同
BOOL StringMatch(LPCTSTR str, LPCTSTR match)
{
	if (str == NULL || match == NULL)
		return FALSE;

	int length = _tcslen(str);
	for (int i = 0; i < length; i++)
	{
		if (str[i] != match[i])
			return FALSE;
	}
	return TRUE;
}

BOOL GetAppPathA(LPSTR lpszAppPath, int * length)
{
	if (NULL == lpszAppPath)
		return FALSE;

	ZeroMemory(lpszAppPath, *length*sizeof(CHAR));
	static char* szAppPath = NULL;
	do
	{
		if (szAppPath == NULL)
		{
			szAppPath = new char[MAX_PATH + 1];
			if (NULL == szAppPath)
			{
				break;
			}
			if (!GetModuleFileNameA(NULL, szAppPath, MAX_PATH + 1))
				break;

			LPSTR pcBackSlash = strrchr(szAppPath, '\\');
			if (pcBackSlash == NULL)
				break;

			pcBackSlash[0] = '\0';
			if ((*length - 1) < strlen(szAppPath))
			{
				*length = strlen(szAppPath) + 1;
				break;
			}
			
		}
		strncpy(lpszAppPath, szAppPath, *length);
		return TRUE;
	} while (0);

	return FALSE;
}

BOOL GetAppPathW(LPWSTR lpszAppPath,int * length)
{
	if (NULL == lpszAppPath)
		return FALSE;

	ZeroMemory(lpszAppPath, *length*sizeof(WCHAR));
	static WCHAR* szAppPath = NULL;
	do
	{
		if (szAppPath == NULL)
		{
			szAppPath = new WCHAR[MAX_PATH + 1];
			if (szAppPath == NULL)
			break;

			if (!GetModuleFileNameW(NULL, szAppPath, MAX_PATH + 1))
				break;

			LPWSTR pcBackSlash = wcsrchr(szAppPath, L'\\');
			if (pcBackSlash == NULL)
				break;

			pcBackSlash[0] = L'\0';
			if (*length < wcslen(szAppPath)+1)
			{
				*length = wcslen(szAppPath) + 1;
				break;
			}
		}
		wcsncpy(lpszAppPath, szAppPath, *length);
		return TRUE;

	} while (0);

	return FALSE;
}


LPCSTR GetPathExtA(LPCSTR szFilePath)
{
	LPCSTR p = strrchr(szFilePath, '.');
	if (p)
		return (p + 1);
	return NULL;
}

LPCWSTR GetPathExtW(LPCWSTR szFilePath)
{
    LPCWSTR p = wcsrchr(szFilePath, L'.');
    if (p)
        return (p + 1);
    return NULL;
}

void AddPathExt(LPCTSTR szFilePath1, LPCTSTR extName, LPTSTR szFilePath2)
{
	_stprintf(szFilePath2, _T("%s.%s"), szFilePath1, extName);
}

BOOL GetPrevExt(LPCTSTR szFilePath, LPTSTR szExt)
{
	LPCTSTR p = GetPathExt(szFilePath);
	if (p)
	{
		_tcscpy(szExt, p);
		return TRUE;
	}
	return FALSE;
}

BOOL DelExtA(LPSTR szFilePath)
{
	LPSTR p = strrchr(szFilePath, '.');
	if (p)
	{
		p[0] = 0;
		return TRUE;
	}
	return FALSE;
}

BOOL DelExtW(LPWSTR szFilePath)
{
    LPWSTR p = wcsrchr(szFilePath, L'.');
    if (p)
    {
        p[0] = 0;
        return TRUE;
    }
    return FALSE;
}

LPCSTR GetFileNameA(LPCSTR lpszFilePath)
{
	LPCSTR p = strrchr(lpszFilePath, '\\');
	if (p)
		return (p + 1);
	return NULL;
}

LPCWSTR GetFileNameW(LPCWSTR lpszFilePath)
{
	LPCWSTR p = wcsrchr(lpszFilePath, L'\\');
	if (p)
		return (p + 1);
	return NULL;
}

DWORD GetMyFileSize(LPTSTR szFilePath)
{
	HANDLE hFile;
	DWORD dwFileSize;

	hFile = CreateFile(szFilePath, 0, 0, NULL, 
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); 
	if (hFile == INVALID_HANDLE_VALUE) 
		return -1; 

	dwFileSize = GetFileSize(hFile, NULL); 
	CloseHandle(hFile);
	return dwFileSize;
}

char * TrimSpace(char * ps)
{
	if (ps == NULL)
		return NULL;
	//从前往后扫描
	char * psTemp = ps;
	while (*psTemp != 0)
	{
		//遇到中文字符
		if (*psTemp < 0 
			|| !isspace(*psTemp))
			break;
		else
			psTemp++;
	}
	//从后往前扫描
	char * psTemp2 = ps + strlen(ps) - 1;
	while (psTemp2 > psTemp)
	{
		if ((psTemp2 - 1 > psTemp 
			&& *(psTemp2 - 1) < 0) //中文字符
			|| !isspace(*psTemp2))
			break;
		else
			psTemp2--;
	}
	psTemp2[1] = '\0';
	return psTemp;
}

BOOL IsFileExists(LPCTSTR   lpszFileName)   
{   
	WIN32_FIND_DATA   wfd;   
	BOOL   bRet;   
	HANDLE   hFind;   
	hFind   =   FindFirstFile(lpszFileName,   &wfd);   
	bRet   =   hFind  !=  INVALID_HANDLE_VALUE;   
	FindClose(hFind);   
	return   bRet;   
}  

UINT DebugInfoSet(LPCTSTR szinfo, LPCTSTR szFileName)
{
	TCHAR* szBuffer = new TCHAR[MAX_PATH+40];
	int length = MAX_PATH;
	GetAppPath(szBuffer, &length);
	_tcscat(szBuffer, _T("\\"));
	_tcscat(szBuffer, szFileName);
	UINT ret = GetPrivateProfileInt(_T("Debug"), szinfo, 0, szBuffer);
	delete []szBuffer;
	return ret;
}

//指定文件写内容
void log_printer(LPTSTR filename, LPCTSTR lpszFormant, ...)
{

	__try
	{

	TCHAR* szBuffer = new TCHAR[BUFFER_LENGTH];
	TCHAR* szTimeAndBuf = new TCHAR[BUFFER_LENGTH];
	SYSTEMTIME* szTime = new SYSTEMTIME[BUFFER_LENGTH];
	va_list args;
	va_start(args, lpszFormant);
#ifdef UNICODE
	vswprintf(szBuffer, lpszFormant, args);
#else
	vsprintf(szBuffer, lpszFormant, args);
#endif
	
	va_end(args);
	//OutputDebugString(szBuffer);
	GetLocalTime(szTime);
#ifdef UNICODE
	wsprintf(szTimeAndBuf, L"[%d-%d-%d %d:%d:%d:%d] ", szTime->wYear, szTime->wMonth, szTime->wDay,
					szTime->wHour, szTime->wMinute, szTime->wSecond, szTime->wMilliseconds);
#else
	sprintf(szTimeAndBuf, "[%d-%d-%d %d:%d:%d:%d] ", szTime->wYear, szTime->wMonth, szTime->wDay,
		szTime->wHour, szTime->wMinute, szTime->wSecond, szTime->wMilliseconds);
#endif
	_tcscat(szTimeAndBuf, szBuffer);
	TCHAR* szDes = new TCHAR[BUFFER_LENGTH];
	memset(szDes, 0, (sizeof(TCHAR)));
#ifdef UNICODE
	WideCharToMultiByte(CP_ACP, 0, szTimeAndBuf, -1, (LPSTR)szDes, BUFFER_LENGTH, NULL, NULL);
#else
	_tcscat(szDes, szTimeAndBuf);
#endif	
	_tcscat(szDes, _T("\r\n"));
	delete []szBuffer;
	delete []szTimeAndBuf;
	delete []szTime;

#ifdef _USR_HOOK_FUNCTION_
	HANDLE hFile = CreateFile(filename, 
		GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,  0, NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		SetFilePointer(hFile, 0, NULL, FILE_END);
		DWORD dwWritten = 0;
		WriteFile(hFile, szDes, strlen(szDes), &dwWritten, NULL);
		CloseHandle(hFile);
	}
	else
	{
		HANDLE pFile = CreateFile(filename, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); // FILE * pFile = _tfopen(szAppPath, _T("ab"));
		SetFilePointer(pFile, 0, NULL, FILE_END);
		DWORD dwBytes;
		WriteFile(pFile, szDes, strlen(szDes), &dwBytes, NULL);
		CloseHandle(pFile);
	}
#else

	HANDLE pFile = CreateFile(filename, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL); // FILE * pFile = _tfopen(szAppPath, _T("ab"));
	SetFilePointer(pFile, 0, NULL, FILE_END);
	//DebugPrintA("CreateFile");
	DWORD dwBytes;
	WriteFile(pFile, szDes, _tcslen(szDes), &dwBytes, NULL);
	//DebugPrintA("WriteFile");
	CloseHandle(pFile);

#endif
	delete []szDes;

	}

	__except(EXCEPTION_EXECUTE_HANDLER)
	{
	}


}