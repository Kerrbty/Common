#include "stdafx.h"
#include "Unhook.h"
#include "LoadModel.h"
#include "pestreat.h"
#if _MSC_VER < 1400 
#include "mykernel32.h"
#endif

#define AllocMemory(_a)  HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, _a)
#define FreeMemory(_a)   { if (_a) { HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, _a); _a=NULL; } }

DWORD print_defferent(PBYTE dst, PBYTE src, DWORD dwsize)
{
    DWORD i = 0;
	for (i=0; i<dwsize; i++)
	{
		if (dst[i] == src[i])
		{
			break;
		}
	}

    HMODULE gethmod = NULL;
    LPTSTR bufinfo = (LPTSTR)AllocMemory(1024*sizeof(TCHAR));

    // ��ӡ��ַ
    wsprintf(bufinfo, TEXT("Inline Hook Address is: 0x%08X, size = %d\n"), dst, i);
    OutputDebugString(bufinfo);
	FreeMemory(bufinfo);

    DWORD dwBytes = 0;
    WriteProcessMemory(GetCurrentProcess(), dst, src, i, &dwBytes);

	return i;
}

DWORD compare_memory(PBYTE pdst, PBYTE psrc, DWORD size)
{
	DWORD number = 0;
	for (DWORD i=0; i<size; i++)
	{
		if (pdst[i] != psrc[i])
		{
			number++;
			i += print_defferent(&pdst[i], &psrc[i], size-i);
		}
	}
	
	return number;
}

DWORD CheckInlineHook(PBYTE hdst, PBYTE hsrc)
{
	PIMAGE_DOS_HEADER Header = (PIMAGE_DOS_HEADER)hsrc; // �����Լ����ص�dll
	PIMAGE_NT_HEADERS peheader = 
		(PIMAGE_NT_HEADERS)((DWORD)Header + Header->e_lfanew);
	ULONG check_number = 0 ;
	
	PIMAGE_SECTION_HEADER SectionHeader = (PIMAGE_SECTION_HEADER)( (DWORD)peheader + 
		sizeof(peheader->FileHeader) + 
		sizeof(peheader->Signature) +
		peheader->FileHeader.SizeOfOptionalHeader ); // �ڱ���Ŀ�ʼ
	
	DWORD SectionNum = peheader->FileHeader.NumberOfSections; // ����Ŀ
	
    // ������ַ
    DWORD IATAddress = (DWORD)hdst + peheader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress;
    DWORD IATSize = peheader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].Size;
    if (IATSize & 0xFFF)
    {
        IATSize = (IATSize & 0xFFFFF000) + 0x1000;
    }
 
#ifdef _HOOK_DEBUG
    LPTSTR bufinfo = new TCHAR[1024];
    wsprintf(bufinfo, TEXT("IAT From: 0x%08X, To: 0x%08X\n"), IATAddress, IATAddress+IATSize);
    OutputDebugString(bufinfo);

    wsprintf(bufinfo, TEXT("section number:%d\n"), SectionNum);
    OutputDebugString(bufinfo);
#endif
	
	
	for (DWORD i=0; i<SectionNum; i++) // ����һ�����Ƚ��Ƿ��޸�
	{
		if (   ( (SectionHeader[i].Characteristics&0x20000000) && // ��Ϊ��ִ�д���
			   (SectionHeader[i].Characteristics&0x20) &&  // ������ִ�д���
			   !(SectionHeader[i].Characteristics&0x80000000)  // ����д
              )
			)
		{
			DWORD dwsize = SectionHeader[i].SizeOfRawData;
			if ( dwsize > SectionHeader[i].Misc.VirtualSize )
			{
				dwsize = SectionHeader[i].Misc.VirtualSize;
			}
            DWORD dstaddr = (DWORD)hdst + SectionHeader[i].VirtualAddress;
            DWORD srcaddr = (DWORD)hsrc + SectionHeader[i].VirtualAddress;

#ifdef _HOOK_DEBUG
            wsprintf(bufinfo, TEXT("Code From: 0x%08X, size: 0x%08X\n"), dstaddr, dwsize);
            OutputDebugString(bufinfo);
#endif
            // ������ڴ�����м�
            if ( dstaddr <= IATAddress &&
                 dstaddr+dwsize >= IATAddress+IATSize )
            {
#ifdef _HOOK_DEBUG
                OutputDebugString(TEXT("�غ�"));
#endif
                if (dstaddr != IATAddress)
                {
#ifdef _HOOK_DEBUG
                    wsprintf(bufinfo, TEXT("First Line: 0x%08X, size: 0x%08X, dwsize = 0x%08X\n"), dstaddr, dstaddr-IATAddress, dwsize);
                    OutputDebugString(bufinfo);
#endif
                    check_number += compare_memory( (PBYTE)dstaddr, 
                                                    (PBYTE)srcaddr, 
											        dstaddr-IATAddress );
                   
                }

                if ( IATAddress+IATSize < dstaddr+dwsize )
                {
                    DWORD offset  = IATAddress+IATSize - dstaddr;
#ifdef _HOOK_DEBUG
                    wsprintf(bufinfo, TEXT("First Line: 0x%08X, size: 0x%08X, dwsize = 0x%08X\n"), 
                        (dstaddr+offset), (dstaddr+dwsize) - (IATAddress+IATSize), dwsize);
                    OutputDebugString(bufinfo);
#endif
                    check_number += compare_memory( (PBYTE)(dstaddr+offset), 
                                                    (PBYTE)(srcaddr+offset), 
											        (dstaddr+dwsize) - (IATAddress+IATSize) );
                    
                }
            }
            // ������غ�
            else
            {
#ifdef _HOOK_DEBUG
                OutputDebugString(TEXT("���غ�"));
#endif
			    check_number += compare_memory( (PBYTE)dstaddr, 
											    (PBYTE)srcaddr, 
										    	dwsize );
            }
		}
	}
#ifdef _HOOK_DEBUG
    delete []bufinfo;
#endif
	return check_number;
}


DWORD  DelInlineHook(HMODULE hmodule, PTSTR DllName)
{
    DWORD dwconut = 0;
	HMODULE FileModule = hmodule;
	if (FileModule == NULL)
	{
		FileModule = GetModuleHandle(DllName);
		if (FileModule == NULL)
		{
			return FALSE;
		}
	}

	// �Լ��Ĳ�����
	HMODULE gethmod ;
	GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPTSTR)DelInlineHook, &gethmod);
	if (gethmod == FileModule)
	{
		return FALSE;
	}
	LPTSTR  FileName = (LPTSTR)AllocMemory(MAX_PATH*sizeof(TCHAR));
	GetModuleFileName(FileModule, FileName, MAX_PATH);

#ifdef _HOOK_DEBUG
	OutputDebugString(FileName);
#endif

	HMODULE myhmod = LoadMyDlls(FileName, (LPBYTE)FileModule);
	FreeMemory(FileName);

    if (myhmod != NULL)
    {
        dwconut = CheckInlineHook((LPBYTE)FileModule, (LPBYTE)myhmod);
	    FreeMyDlls(myhmod, (LPBYTE)myhmod);
    }

	return dwconut;
}


DWORD  DelIATHook(HMODULE hModule)
{
	if (hModule == NULL)
	{
		return 0;
	}

    DWORD IATHookCount = 0;
	
	PIMAGE_DOS_HEADER Header = (PIMAGE_DOS_HEADER)hModule;
	PIMAGE_NT_HEADERS peheader = 
		(PIMAGE_NT_HEADERS)((DWORD)Header + Header->e_lfanew);

	// IAT��ַ
	PIMAGE_IMPORT_DESCRIPTOR pImpDescript = (PIMAGE_IMPORT_DESCRIPTOR)
		( (DWORD)hModule + peheader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

    if ((DWORD)pImpDescript == (DWORD)hModule)
    {
        return 0;
    }
    
    // ������2ָ����ַ���ָ���޸���1ָ����ڴ��(��1ָ��IMPORT table)
    while (pImpDescript->Name != NULL)
    {
        PDWORD ImportApiName = (PDWORD)((DWORD)hModule + pImpDescript->OriginalFirstThunk); // ���뺯����������
        // ��Ҫ�޸��ĵ�����ַ , �� IMAGE_DIRECTORY_ENTRY_IAT ����
        PDWORD FixTable = (PDWORD)((DWORD)hModule + pImpDescript->FirstThunk); 
        
        // �õ�����dll��ַ
        char* DLLName = (char*)((DWORD)hModule + pImpDescript->Name);
        
        HMODULE impDllModule = GetModuleHandleA( DLLName );
        if (impDllModule == NULL)
        {
            impDllModule = LoadLibraryA( DLLName );
        }
        
        if ( memicmp( DLLName, "API-MS-Win-Core", 15) == 0)
        {
            impDllModule = 0;
        }

        // �޸�
        if (impDllModule)
        {
            for (int i=0; ImportApiName[i]!=NULL ; i++)
            {
                PHintName imputhint = (PHintName)( ImportApiName[i] + (DWORD)hModule );
                __try
                {
                    if ( imputhint->Name != NULL )
                    {
                        DWORD addr = 0;
                        DWORD order = *((DWORD*)imputhint->Name);
						
                        addr = (DWORD)MyGetProcAddress(impDllModule, imputhint->Name);
                        if (addr == 0)
                        {
                            addr = (DWORD)MyGetProcAddress(impDllModule, (PSTR)(order&0xFFFF) );
                        }
						
                        if (addr != 0 && FixTable[i] != addr )
                        {
                            ::WriteProcessMemory(::GetCurrentProcess(), &FixTable[i], &addr, sizeof(DWORD), NULL);
//                             VirtualProtect(&FixTable[i], 4, PAGE_READWRITE, &OldProtect);  // QQ.exe �����в���ֱ�ӵ�������������ᱻ��⵽�˳�
//                             FixTable[i] = addr;
//                             VirtualProtect(&FixTable[i], 4, OldProtect, &OldProtect);
                            IATHookCount++;
                        }
                    }
                }
                __except(EXCEPTION_EXECUTE_HANDLER)
                {
                    continue;
                }
                
            }
        }
        pImpDescript++;
    }
    
	return IATHookCount;
}


