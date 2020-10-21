#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include "StringFormat/StringFormat.h"
#include "disasm.h"
#include "AnalyzeSymbol.h"

void print_stack_call()
{
	HMODULE hmodule;
    t_disasm da;
    TCHAR Funcname[MAX_PATH];

	DWORD* dw_ebp;
    DWORD dw_stack_bottom;
	TCHAR* FileName = new TCHAR[MAX_PATH];
    TCHAR* printbuf = new TCHAR[MAX_PATH];
	__asm mov dw_ebp, ebp
    dw_stack_bottom = ((DWORD)dw_ebp&0xFFFF0000) + 0x10000;

	while(1)
	{
		dw_ebp++;
		while( (DWORD)dw_ebp < dw_stack_bottom )
        {
            BOOL findit = FALSE;
            DWORD len = 2;
            DWORD diasmaddr = *dw_ebp - 2;

            // 反汇编地址可能错误的
            __try{
                while( !IsBadReadPtr((PVOID)(*dw_ebp), 20) )
                {
                    DWORD dwlen = Disasm((char*)diasmaddr, 20, 0, &da, DISASM_CODE);
                    if (len == dwlen && memicmp(da.result, "call", 4) == NULL)
                    {
                        findit = TRUE;
                        break;
                    }
                    diasmaddr--;
                    len++;
                    if (len > 16)
                    {
                        break;
                    }
              }
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                ;
            }
            
            if (findit == TRUE)
            {
                break;
            }
            dw_ebp++;
        }
        if ( (DWORD)dw_ebp >= dw_stack_bottom )
        {
            break;
        }
		hmodule = NULL;
		GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (PCHAR)dw_ebp[0], &hmodule);
        ZeroMemory(printbuf, MAX_PATH*sizeof(TCHAR));
        if (hmodule != NULL)
        {
            ZeroMemory(FileName, MAX_PATH*sizeof(TCHAR));
            GetModuleFileName(hmodule, FileName, MAX_PATH);
            LPTSTR str = FindLasteSymbol(FileName, TEXT('.'));
            if ( str !=  FileName )
            {
                --str;
                *str = TEXT('\0');
            }
            _tcscat(printbuf, FindLasteSymbol(FileName, TEXT('\\')) );
        }

        // 获取符号表可能错误
        __try{
            DWORD offset = 0;
            GetFuncName(dw_ebp[0], Funcname, MAX_PATH, &offset);
            wsprintf(FileName, TEXT(" %08X\t%s!%s+0x%x\n"), dw_ebp[0], printbuf, Funcname, offset);
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            wsprintf(FileName, TEXT(" %08X\t%s\n"), dw_ebp[0], printbuf);
	    }
        printf("%s", FileName);
        OutputDebugString(FileName);
    }

    delete printbuf;
	delete FileName;
}
