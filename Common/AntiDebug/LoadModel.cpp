#include "stdafx.h"
#include "LoadModel.h"

#pragma pack(push, 1)
BOOL copy_memory(DWORD DstAddr, DWORD SrcAddr, size_t dwsize);

// �ض�λ��ṹ
typedef struct _OffTable{
	USHORT addr:12;
	USHORT flags:4;
}OffTable, *pOffTable;

typedef struct _RELOADTABLE{
	DWORD StartVirtualAddress;
	DWORD size;
	OffTable Table[1];
}RELOADTABLE, *pRELOADTABLE;



// �����ṹ
typedef struct _HintName
{
	WORD Hint;
	CHAR Name[1];
}HintName, *PHintName;

typedef struct _DllMainCall 
{
	HMODULE hmodule;
	DWORD reasons;
}DllMainCall, *pDllMainCall;

#pragma pack(pop)



BOOL RelocAddr(HMODULE hModule, LPBYTE BaseAddress); // �޸��ض�λ��
HMODULE MapThePeFileW(PWSTR szFileName); // ��Ⲣӳ���ļ�
DWORD WINAPI DllInit(LPVOID hModule); // ��ʼ��(����)DllMain
BOOL FixIMPORT(HMODULE hModule); // ��䵼���
BOOL isPEFile(LPVOID pFileMap); // �ж��ļ��Ƿ���PE�ļ�
BOOL FixHookModule(HMODULE SrcHmod, HMODULE DstHmod); // �޸�����Σ���Dst�Ĵ���θ���Src�Ĵ����


typedef BOOL (APIENTRY* pDllMain)(  HINSTANCE hModule, 
								  DWORD  ul_reason_for_call, 
									LPVOID lpReserved);



// �޸��ض�λ�� hmodule ���صĵ�ַ
// BaseAddress ��Ҫ�ض�λ�ĵ�ַ���������HOOK���߻ָ�hookʱ����
BOOL RelocAddr( HMODULE hModule, LPBYTE BaseAddress )
{
	if (hModule == NULL)
	{
		return false;
	}

	if ( BaseAddress == NULL )
	{
		BaseAddress = (LPBYTE)hModule;
	}

	PIMAGE_DOS_HEADER Header = (PIMAGE_DOS_HEADER)hModule;
	PIMAGE_NT_HEADERS peheader = 
		(PIMAGE_NT_HEADERS)((LPBYTE)Header + Header->e_lfanew);

	DWORD dwsizeOfImage = peheader->OptionalHeader.SizeOfImage;
	LPBYTE dwAddress = (LPBYTE)peheader->OptionalHeader.ImageBase;

	if ( dwAddress == (LPBYTE)hModule )
	{
		// ���ص������ַ,����Ҫ�ض�λ
		return TRUE;
	}

	// PE ͷ offset 0x98
	pRELOADTABLE  reloadaddr = (pRELOADTABLE)
		( (LPBYTE)hModule + peheader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress) ;

	if ( (LPBYTE)reloadaddr < (LPBYTE)hModule || 
		 (LPBYTE)reloadaddr > (LPBYTE)hModule+dwsizeOfImage )
	{
		// ����������ļ���,�ض�λ�����
		return false;
	}

	// �����ض�λ��
	while ( reloadaddr->StartVirtualAddress != NULL && reloadaddr->size != NULL )
	{
		for (DWORD i=0; i<(reloadaddr->size-8)/2 ; i++)
		{
			__try
			{
				if ( reloadaddr->Table[i].flags == 3 )
				{
					PDWORD* OffsetAddress = (PDWORD*)(reloadaddr->Table[i].addr + (LPBYTE)hModule + reloadaddr->StartVirtualAddress);
					// �����µ�ƫ���� = ԭʼֵ - ԭʼ���ص�ַ + �µļ��ص�ַ
					*OffsetAddress = (LPDWORD)((LPBYTE)(*OffsetAddress) - dwAddress + BaseAddress);
				}
			}
			__except(EXCEPTION_EXECUTE_HANDLER)
			{
				continue;
			}
		}
		reloadaddr = (pRELOADTABLE) ((DWORD)reloadaddr  + reloadaddr->size );
	}


	return true; 
}


// ��䵼���
BOOL FixIMPORT(HMODULE hModule) 
{
	if (hModule == NULL)
	{
		return false;
	}
	
	PIMAGE_DOS_HEADER Header = (PIMAGE_DOS_HEADER)hModule;
	PIMAGE_NT_HEADERS peheader = 
		(PIMAGE_NT_HEADERS)((LPBYTE)Header + Header->e_lfanew);

	// IAT��ַ
	PIMAGE_IMPORT_DESCRIPTOR pImpDescript = (PIMAGE_IMPORT_DESCRIPTOR) (pRELOADTABLE)
		( (LPBYTE)hModule + peheader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

	// ������2ָ����ַ���ָ���޸���1ָ����ڴ��(��1ָ��IMPORT table)
	while (pImpDescript->Name != NULL)
	{
		PDWORD ImportApiName = (PDWORD)((LPBYTE)hModule + pImpDescript->OriginalFirstThunk); // ���뺯����������
		// ��Ҫ�޸��ĵ�����ַ , �� IMAGE_DIRECTORY_ENTRY_IAT ����
		LPDWORD* FixTable = (PDWORD*)((LPBYTE)hModule + pImpDescript->FirstThunk); 
		
		// �õ�����dll��ַ
		HMODULE impDllModule = GetModuleHandleA( (char*)((LPBYTE)hModule + pImpDescript->Name) );
		if (impDllModule == NULL)
		{
			impDllModule = LoadLibraryA( (char*)((LPBYTE)hModule + pImpDescript->Name) );
		}

		// �޸�
		if (impDllModule)
		{
			for (int i=0; ImportApiName[i]!=NULL ; i++)
			{
				PHintName imputhint = (PHintName)( ImportApiName[i] + (LPBYTE)hModule );
				__try
				{
					if (imputhint->Name[0] != '\0')
					{
						FixTable[i] = (LPDWORD)GetProcAddress(impDllModule, imputhint->Name);
					}
					else
					{
						FixTable[i] = (LPDWORD)GetProcAddress(impDllModule, (PSTR)imputhint->Hint);
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

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// ��Ⲣӳ���ļ�
// ���ӳ����̿��� ntdll ������ 
// ZwOpenFile(ZwCreateFile)\ ZwCreateSection \ ZwMapViewOfSection 
// ȡ��
/////////////////////////////////////////////////////////////////////////////////////////////////
HMODULE MapThePeFileW(PWSTR szFileName)
{
	HANDLE handle = CreateFileW(szFileName, 
		GENERIC_READ, 
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (handle == INVALID_HANDLE_VALUE)
	{
		return NULL;
	}

	// ӳ���ļ�
    HANDLE pWrite = CreateFileMapping(handle,
        NULL,  
        PAGE_READONLY,  
        0,  
        0,  
        NULL);  
	CloseHandle(handle);
	if (pWrite == NULL)
	{
		return NULL;
	}

    LPVOID pFile = MapViewOfFile(pWrite, 
        FILE_MAP_READ,  
        0,  
        0,  
        0);  
	if (pFile == NULL || !isPEFile(pFile))
	{
		CloseHandle(pWrite);
		return NULL;
	}

	PIMAGE_DOS_HEADER Header = (PIMAGE_DOS_HEADER)pFile;
	PIMAGE_NT_HEADERS peheader = 
		(PIMAGE_NT_HEADERS)((LPBYTE)Header + Header->e_lfanew);
	
	// �����ļ���¼�Ĵ�С�����ڴ�
	DWORD dwsizeOfImage = peheader->OptionalHeader.SizeOfImage;
	LPVOID DllAddress = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwsizeOfImage);
	if (DllAddress == NULL)
	{
		UnmapViewOfFile(pFile);
		CloseHandle(pWrite);
		return NULL;
	}
	DWORD oldprotect;
	VirtualProtect(DllAddress, dwsizeOfImage, PAGE_EXECUTE_READWRITE, &oldprotect);

	DWORD dwFileAlignment = peheader->OptionalHeader.FileAlignment; // �ļ���������
	memcpy(DllAddress, pFile, dwFileAlignment); // ��PEͷ���ƹ�ȥ

	PIMAGE_SECTION_HEADER SectionHeader = (PIMAGE_SECTION_HEADER)( (DWORD)peheader + 
											sizeof(peheader->FileHeader) + 
											sizeof(peheader->Signature) +
											peheader->FileHeader.SizeOfOptionalHeader ); // �ڱ���Ŀ�ʼ

	WORD SectionNum = peheader->FileHeader.NumberOfSections; // ����Ŀ
	for (WORD i=0; i<SectionNum; i++) // ����һ�������Ƶ��ڴ���
	{
		DWORD ulsize = SectionHeader[i].Misc.VirtualSize;
		if ( ulsize > SectionHeader[i].SizeOfRawData )
		{
			ulsize = SectionHeader[i].SizeOfRawData;
		}
		memcpy(	(LPVOID)((DWORD)DllAddress + SectionHeader[i].VirtualAddress), 
				(LPVOID)((DWORD)pFile + SectionHeader[i].PointerToRawData), 
				ulsize ); 
	} 

	UnmapViewOfFile(pFile);
	CloseHandle(pWrite);

	return (HMODULE)DllAddress;
}

// �ж�PE�ļ��Ƿ�Ϸ�
BOOL isPEFile(LPVOID pFileMap)
{
	PIMAGE_DOS_HEADER Header = (PIMAGE_DOS_HEADER)pFileMap;

	if( Header->e_magic != 'ZM' )
	{
		return FALSE;
	}

	if ( Header->e_lfanew > 0x1000 || Header->e_lfanew < 0 )
	{
		return FALSE;
	}

	PIMAGE_NT_HEADERS peheader = 
		(PIMAGE_NT_HEADERS)((LPBYTE)Header + Header->e_lfanew);

	if (peheader->Signature != 'EP' || 
		peheader->FileHeader.Machine != 0x014C &&  // x86
		peheader->FileHeader.Machine != 0x8664 )   // x64
//		peheader->OptionalHeader.SectionAlignment != PAGE_SIZE) // �ڴ��ж�������
	{
		return FALSE;
	}

	return TRUE;
}


// ����DLLMain
DWORD WINAPI DllInit(LPVOID lpwparam)
{
	pDllMainCall dllcall = (pDllMainCall)lpwparam;
	if (dllcall == NULL || dllcall->hmodule == NULL)
	{
		return -1;
	}

	PIMAGE_DOS_HEADER Header = (PIMAGE_DOS_HEADER)dllcall->hmodule;
	PIMAGE_NT_HEADERS peheader = 
		(PIMAGE_NT_HEADERS)((LPBYTE)Header + Header->e_lfanew);
	
	DWORD dwAddress = peheader->OptionalHeader.AddressOfEntryPoint;

	// ���ô�˵�е�DllMain
	pDllMain dllini = (pDllMain)( (LPBYTE)dllcall->hmodule + dwAddress) ;
	
	__try{
		if ( !dllini((HINSTANCE)dllcall->hmodule, dllcall->reasons, NULL) )
		{
			return -1;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return -1;
	}
	
	return 0;
}


// ����ϵͳ��ȫ�����ݵ�
BOOL CopyModuleSection(HMODULE SrcHmod, HMODULE DstHmod)
{
	PIMAGE_DOS_HEADER Header = (PIMAGE_DOS_HEADER)SrcHmod;
	PIMAGE_NT_HEADERS peheader = 
		(PIMAGE_NT_HEADERS)((DWORD)Header + Header->e_lfanew);
	ULONG check_number = 0 ;

	PIMAGE_SECTION_HEADER SectionHeader = (PIMAGE_SECTION_HEADER)( (DWORD)peheader + 
		sizeof(peheader->FileHeader) + 
		sizeof(peheader->Signature) +
		peheader->FileHeader.SizeOfOptionalHeader ); // �ڱ���Ŀ�ʼ

	DWORD SectionNum = peheader->FileHeader.NumberOfSections; // ����Ŀ

	for (DWORD i=0; i<SectionNum; i++) // ����һ�������Ƶ��ڴ���
	{
		if ( !(SectionHeader[i].Characteristics&0x20000000) && // ����Ϊ��ִ�д���
			!(SectionHeader[i].Characteristics&0x20)  // ��������ִ�д���
			)
		{
			DWORD ulsize = SectionHeader[i].Misc.VirtualSize ;
			if ( ulsize > SectionHeader[i].SizeOfRawData )
			{
				ulsize = SectionHeader[i].SizeOfRawData;
			}
			copy_memory( (DWORD)DstHmod + SectionHeader[i].VirtualAddress,
				(DWORD)SrcHmod + SectionHeader[i].VirtualAddress,
				ulsize);
		}
	}
	return true;
}

// ����DLL����
HMODULE LoadMyDllsW(PWSTR szFileName, LPBYTE BaseAddress)
{
	HMODULE hModule = MapThePeFileW(szFileName);
	if (hModule == NULL)
	{
		return NULL;
	}
	
	// ˢд�ض�λ��
	if ( !RelocAddr(hModule, BaseAddress) )
 	{
 		HeapFree(GetProcessHeap(), 0, hModule);
 		return NULL;
 	}

	// ˢ�µ����
	// �������ĸ��ʱȽϸ�,ץȡ���쳣
	FixIMPORT(hModule);

	// ˢ����ʱ����� , �ȿ��Ű�, �Ժ�ʵ��
// 	if ( !DelayImport)
// 	{
// 		HeapFree(GetProcessHeap(), 0, hModule);
// 		return NULL;
// 	}

	// ����һ���߳�, ����DLLMain (ͬ��)
	if (BaseAddress == NULL)
	{
		DllMainCall dllcall;
		dllcall.hmodule = hModule;
		dllcall.reasons = DLL_PROCESS_ATTACH;
		HANDLE handle = CreateThread(NULL, 0, DllInit, (LPVOID)&dllcall, 0, NULL);
		if (handle != NULL)
		{
			WaitForSingleObject(handle, INFINITE);
			CloseHandle(handle);
		}
	}
	else
	{
#ifdef _WIN64
		CopyModuleSection((HMODULE)BaseAddress, hModule);
#endif
	}
	
	return hModule;
}


HMODULE LoadMyDllsA(PSTR szFileName, LPBYTE BaseAddress)
{
	PWSTR  pElementText;
	int    iTextLen;

	iTextLen = MultiByteToWideChar( CP_ACP,
		0,
		(PCHAR)szFileName,
		-1,
		NULL,
		0 );
	
	pElementText = 
		(PWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (iTextLen+1)*sizeof(WCHAR));

	MultiByteToWideChar( CP_ACP,
		0,
		(PCHAR)szFileName,
		-1,
		pElementText,
		iTextLen );
	
	HMODULE hmodule = LoadMyDllsW(pElementText, BaseAddress);
	HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, pElementText);
	return hmodule;
}


////////////////////////////////////////////////////////
// ��������������ַ
////////////////////////////////////////////////////////
LPVOID MyGetProcAddress(HMODULE hmodule, PSTR szFuncName) 
{
    DWORD i = 0;
    // szFuncName == NULL ��ʱ��֪���Ǵ��ĺ�����,���Ǵ��ĺ���hintֵ
    if (hmodule == NULL || szFuncName == NULL)
    {
        return NULL;
    }
    
    PIMAGE_DOS_HEADER Header = (PIMAGE_DOS_HEADER)hmodule;
    PIMAGE_NT_HEADERS peheader = 
        (PIMAGE_NT_HEADERS)((DWORD)Header + Header->e_lfanew);
    
    // �������ַ
    PIMAGE_EXPORT_DIRECTORY pExportDir = (PIMAGE_EXPORT_DIRECTORY) // (pRELOADTABLE)
        ( (LPBYTE)hmodule + peheader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
    
    // IAT��ַ
    LPBYTE pExportAddr = (LPBYTE)hmodule + peheader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    DWORD dwExportSize = peheader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
    
    PDWORD NameRVA = (PDWORD)((LPBYTE)hmodule + pExportDir->AddressOfNames );
    PDWORD FuncAddr = (PDWORD)((LPBYTE)hmodule + pExportDir->AddressOfFunctions);
    PWORD Ordinal = (PWORD)((LPBYTE)hmodule + pExportDir->AddressOfNameOrdinals);
    
    // ���������Ƶ����ĺ���
    LPBYTE funaddr = 0;
    if ( HIWORD(szFuncName) > 0 )
    {
//         OutputDebugStringA(szFuncName);
// 		OutputDebugStringA("\r\n");
        __try
        {
            for (i=0; i<pExportDir->NumberOfNames; i++)
            {
// 				char buf[8];
// 				wsprintfA(buf, "%d:\t", i);
// 				OutputDebugStringA(buf);
// 				OutputDebugStringA((PSTR)((DWORD)NameRVA[i]+(LPBYTE)hmodule));
// 				OutputDebugStringA("\r\n");
				LPSTR tmpfunname = (PSTR)((DWORD)NameRVA[i]+(LPBYTE)hmodule);
                if( strcmp( tmpfunname, szFuncName) == 0)
                {
                    WORD Hint = Ordinal[i];
                    funaddr = (LPBYTE)hmodule+FuncAddr[Hint];
                    break;
                }
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            return NULL;
        }
        
    }
    else
    {
        // ��������ŵ����ĺ���
//        OutputDebugStringA("orider");
        DWORD firstOrd = pExportDir->Base;
        PDWORD FuncAddr = (PDWORD)((LPBYTE)hmodule + pExportDir->AddressOfFunctions); 
        for (i=0; i<pExportDir->NumberOfFunctions; i++)
        {
            if ( LOWORD(szFuncName) == Ordinal[i] + firstOrd )
            {
                funaddr = (LPBYTE)hmodule+FuncAddr[i];
                break;
            }
        }
    }
    
    // ����Ǻ���ת���򣬼�����
    PCHAR filename = (PCHAR)funaddr;
    if (funaddr != NULL && pExportAddr <= (LPBYTE)funaddr && pExportAddr+dwExportSize >= (LPBYTE)funaddr &&
        (filename[0]|0x20) >= 'a' && (filename[0]|0x20) <= 'z' &&
        (filename[1]|0x20) >= 'a' && (filename[1]|0x20) <= 'z' 
        )
    {
//        OutputDebugStringA("Next Stepped!");
        DWORD szlen = 0;
        char DllName[MAX_PATH] = {0};
        while(*funaddr != '.')
        {
            DllName[szlen] = *funaddr;
            funaddr++;
            szlen++;
        }
        funaddr++;
        DllName[szlen] = '\0';
        strcat(DllName, ".dll");
        HMODULE dllhmodule = GetModuleHandleA(DllName);
        return MyGetProcAddress(dllhmodule, (LPSTR)funaddr);
    }
    
    return funaddr;
}

BOOL FreeMyDlls(HMODULE hmodule, LPBYTE BaseAddress)
{
	if (BaseAddress != NULL)
	{
		return HeapFree(GetProcessHeap(), 0, hmodule);;
	}

	DllMainCall dllcall;
	if (hmodule == NULL)
	{
		return FALSE;
	}
	dllcall.hmodule = hmodule;
	dllcall.reasons = DLL_PROCESS_DETACH;
	HANDLE handle = CreateThread(NULL, 0, DllInit, (LPVOID)&dllcall, 0, NULL);
	if (handle != NULL)
	{
		WaitForSingleObject(handle, INFINITE);
		CloseHandle(handle);
	}

	return HeapFree(GetProcessHeap(), 0, hmodule);
}

// �޸�DstAddr��ַ���ڴ�����
BOOL copy_memory(DWORD DstAddr, DWORD SrcAddr, size_t dwsize)
{
	// 
	__try
	{
		DWORD OldProtect;
		VirtualProtect((LPVOID)DstAddr, dwsize, PAGE_EXECUTE_READWRITE, &OldProtect);
		memcpy((LPVOID)DstAddr, (LPVOID)SrcAddr, dwsize);
		VirtualProtect((LPVOID)DstAddr, dwsize, OldProtect, &OldProtect);
	}
	__except( EXCEPTION_EXECUTE_HANDLER )
	{
		return false;
	}
	return true;
}

// �ָ�hook����
BOOL FixHookModule(HMODULE SrcHmod, HMODULE DstHmod)
{
	PIMAGE_DOS_HEADER Header = (PIMAGE_DOS_HEADER)SrcHmod;
	PIMAGE_NT_HEADERS peheader = 
		(PIMAGE_NT_HEADERS)((DWORD)Header + Header->e_lfanew);
	ULONG check_number = 0 ;
	
	PIMAGE_SECTION_HEADER SectionHeader = (PIMAGE_SECTION_HEADER)( (DWORD)peheader + 
		sizeof(peheader->FileHeader) + 
		sizeof(peheader->Signature) +
		peheader->FileHeader.SizeOfOptionalHeader ); // �ڱ���Ŀ�ʼ
	
	DWORD SectionNum = peheader->FileHeader.NumberOfSections; // ����Ŀ
	
	for (DWORD i=0; i<SectionNum; i++) // ����һ�������Ƶ��ڴ���
	{
		if ( (SectionHeader[i].Characteristics&0x20000000) && // ��Ϊ��ִ�д���
			(SectionHeader[i].Characteristics&0x20) &&  // ������ִ�д���
			!(SectionHeader[i].Characteristics&0x80000000)  // ����д
			)
		{
			DWORD ulsize = SectionHeader[i].Misc.VirtualSize ;
			if ( ulsize > SectionHeader[i].SizeOfRawData )
			{
				ulsize = SectionHeader[i].SizeOfRawData;
			}
			copy_memory( (DWORD)DstHmod + SectionHeader[i].VirtualAddress,
						(DWORD)SrcHmod + SectionHeader[i].VirtualAddress,
						ulsize);
		}
	}
	return true;
}

LPTSTR FindLasteSymbol(LPTSTR CommandLine, TCHAR FindWchar)
{
	int Len;
	for ( Len = _tcslen(CommandLine) ; Len>0; Len-- )
	{
		if (CommandLine[Len] == FindWchar)
		{
			Len++;
			break;
		}
	}
	return &CommandLine[Len];
}


BOOL FixDllHook(LPTSTR DllPathName)
{
	PTSTR lpDllName = FindLasteSymbol(DllPathName, TEXT('\\'));
	HMODULE BaseHmodule = GetModuleHandle(lpDllName);
	HMODULE hmodule = LoadMyDlls(DllPathName, (LPBYTE)BaseHmodule);
	if (hmodule != NULL)
	{
		FixHookModule(hmodule, BaseHmodule);
		FreeMyDlls(hmodule);
		return true;
	}
	return false;
}