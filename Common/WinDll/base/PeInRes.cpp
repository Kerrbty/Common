#include "PeInRes.h"
#include "Log.h"
#include <stdio.h>
#include <stdlib.h>

// 将资源保存为文件
BOOL SaveResFile(PTSTR szSaveFileName, PTSTR szResName, PTSTR ResId)
{
	HMODULE module = GetModuleHandle(NULL);
	HRSRC res = FindResource( module, ResId, szResName);
	HGLOBAL hglob = LoadResource(module, res);
	DWORD dwsize = SizeofResource(module, res);
	if (res == NULL)
	{
#ifdef _DEBUG
		DWORD error = GetLastError();
		_tprintf( TEXT("FindResource Error Msg: ") );
		PrintErrorMsg(error);
#endif
		return FALSE;
	}
	LPVOID pres = LockResource(hglob);

	HANDLE hpfile = CreateFile(szSaveFileName, 
								GENERIC_WRITE,
								0,
								NULL,
								CREATE_ALWAYS,
								FILE_ATTRIBUTE_NORMAL,
								NULL);
	if (hpfile == INVALID_HANDLE_VALUE)
	{
#ifdef _DEBUG
		DWORD error = GetLastError();
		_tprintf( TEXT("CreateFile Error: %d\n") );
		PrintErrorMsg(error);
#endif
		return FALSE;
	}
	DWORD dwBytes = 0;
	WriteFile(hpfile, pres, dwsize, &dwBytes, NULL);

	CloseHandle(hpfile);
	UnlockResource(hglob);
	return TRUE;
}
