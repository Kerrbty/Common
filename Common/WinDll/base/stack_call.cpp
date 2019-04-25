#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include "mykernel32.h"
#include "StringForm.h"

void print_stack_call()
{
	HMODULE hmodule;
	DWORD* dw_ebp, *dw_esp;
	TCHAR* FileName = new TCHAR[MAX_PATH];
	__asm mov dw_ebp,ebp
	__try
	{
		while(1)
		{
			dw_esp = dw_ebp;
			dw_ebp = (DWORD*)(*dw_esp);
			if (dw_esp[1] == 0)
			{
				break;
			}
			hmodule = NULL;
			GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (PCHAR)dw_esp[1], &hmodule);
			ZeroMemory(FileName, MAX_PATH*sizeof(TCHAR));
			GetModuleFileName(hmodule, FileName, MAX_PATH);
			LPTSTR str = FindLasteSymbol(FileName, TEXT('.'));
			if ( str !=  FileName )
			{
				*str = TEXT('\0');
			}
			_tprintf("%s.0x%08X\n", FindLasteSymbol(FileName, '\\')+1, dw_esp[1]);
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		;
	}
	delete FileName;
}
