#pragma once

#include <windows.h>
#include <tchar.h>


LPTSTR GetFuncNameByDllName(  LPSTR DllName, 
                            DWORD Address, 
                            LPTSTR FuncName, 
                            DWORD lenth, 
                            DWORD* offset, 
                            HANDLE hprocess = GetCurrentProcess());


LPTSTR GetFuncName(  DWORD Address, 
                   LPTSTR FuncName, 
                   DWORD lenth, 
                   DWORD* offset);

DWORD GetFuncAddress(LPTSTR DllName, LPTSTR FuncName, HANDLE hprocess = GetCurrentProcess() );