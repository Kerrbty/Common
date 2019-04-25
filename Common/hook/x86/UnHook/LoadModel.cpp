#include "LoadModel.h"
#include "pestreat.h"


BOOL RelocAddr(HMODULE hModule, DWORD BaseAddress); // �޸��ض�λ��
HMODULE MapThePeFile(PTSTR szFileName); // ��Ⲣӳ���ļ�
BOOL FixIMPORT(HMODULE hModule); // ��䵼���
BOOL isPEFile(LPVOID pFileMap); // �ж��ļ��Ƿ���PE�ļ�


typedef BOOL (APIENTRY* pDllMain)(  HINSTANCE hModule, 
								  DWORD  ul_reason_for_call, 
									LPVOID lpReserved);



// �޸��ض�λ�� hmodule ���صĵ�ַ
// BaseAddress ��Ҫ�ض�λ�ĵ�ַ���������HOOK���߻ָ�hookʱ����
BOOL RelocAddr( HMODULE hModule, DWORD BaseAddress )
{
	if (hModule == NULL)
	{
		return false;
	}

	if ( BaseAddress == NULL )
	{
		BaseAddress = (DWORD)hModule;
	}

	PIMAGE_DOS_HEADER Header = (PIMAGE_DOS_HEADER)hModule;
	PIMAGE_NT_HEADERS peheader = 
		(PIMAGE_NT_HEADERS)((DWORD)Header + Header->e_lfanew);

	DWORD dwsizeOfImage = peheader->OptionalHeader.SizeOfImage;
	DWORD dwAddress = peheader->OptionalHeader.ImageBase;

	if ( dwAddress == (DWORD)hModule )
	{
		// ���ص������ַ,����Ҫ�ض�λ
		return TRUE;
	}

	// PE ͷ offset 0x98
	pRELOADTABLE  reloadaddr = (pRELOADTABLE)
		( (DWORD)hModule + peheader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress) ;

	if ( (DWORD)reloadaddr <= (DWORD)hModule || 
		 (DWORD)reloadaddr >= (DWORD)hModule+dwsizeOfImage )
	{
		// ����������ļ���,����û���ض�λ��,�ض�λ�����
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
					PDWORD OffsetAddress = (PDWORD)(reloadaddr->Table[i].addr + (DWORD)hModule + reloadaddr->StartVirtualAddress);
					// �����µ�ƫ���� = ԭʼֵ - ԭʼ���ص�ַ + �µļ��ص�ַ
					*OffsetAddress = *OffsetAddress - (DWORD)dwAddress + BaseAddress;
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
		(PIMAGE_NT_HEADERS)((DWORD)Header + Header->e_lfanew);

	// IAT��ַ
	PIMAGE_IMPORT_DESCRIPTOR pImpDescript = (PIMAGE_IMPORT_DESCRIPTOR) 
		( (DWORD)hModule + peheader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

    if ((DWORD)pImpDescript == (DWORD)hModule)
    {
        return FALSE;
    }

	// ������2ָ����ַ���ָ���޸���1ָ����ڴ��(��1ָ��IMPORT table)
	while (pImpDescript->Name != NULL)
	{
		PDWORD ImportApiName = (PDWORD)((DWORD)hModule + pImpDescript->OriginalFirstThunk); // ���뺯����������
		// ��Ҫ�޸��ĵ�����ַ , �� IMAGE_DIRECTORY_ENTRY_IAT ����
		PDWORD FixTable = (PDWORD)((DWORD)hModule + pImpDescript->FirstThunk); 
		
		// �õ�����dll��ַ
		HMODULE impDllModule = GetModuleHandleA( (char*)((DWORD)hModule + pImpDescript->Name) );
		if (impDllModule == NULL)
		{
			impDllModule = LoadLibraryA( (char*)((DWORD)hModule + pImpDescript->Name) );
		}

		// �޸�
		if (impDllModule)
		{
			for (int i=0; ImportApiName[i]!=NULL ; i++)
			{
				PHintName imputhint = (PHintName)( ImportApiName[i] + (DWORD)hModule );
				__try
				{
					if (imputhint->Name[0] != '\0')
					{
						FixTable[i] = (DWORD)GetProcAddress(impDllModule, imputhint->Name);
					}
					else
					{
						FixTable[i] = (DWORD)GetProcAddress(impDllModule, (PSTR)imputhint->Hint);
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
HMODULE MapThePeFile(PTSTR szFileName)
{
	HANDLE handle = CreateFile(szFileName, 
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
		(PIMAGE_NT_HEADERS)((DWORD)Header + Header->e_lfanew);
	
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

	PIMAGE_SECTION_HEADER SectionHeader = (PIMAGE_SECTION_HEADER)( (DWORD)peheader + 
											sizeof(peheader->FileHeader) + 
											sizeof(peheader->Signature) +
											peheader->FileHeader.SizeOfOptionalHeader ); // �ڱ���Ŀ�ʼ

    //	DWORD dwFileAlignment = peheader->OptionalHeader.FileAlignment; // �ļ���������
    memcpy(DllAddress, pFile, SectionHeader[0].SizeOfRawData); // ��PEͷ���ƹ�ȥ

	DWORD SectionNum = peheader->FileHeader.NumberOfSections; // ����Ŀ
	for (DWORD i=0; i<SectionNum; i++) // ����һ�������Ƶ��ڴ���
	{
		DWORD ulsize = SectionHeader[i].SizeOfRawData;
		if ( ulsize > SectionHeader[i].Misc.VirtualSize )
		{
			ulsize = SectionHeader[i].Misc.VirtualSize;
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
		(PIMAGE_NT_HEADERS)((DWORD)Header + Header->e_lfanew);

	if (peheader->Signature != 'EP' || 
		peheader->FileHeader.Machine != 0x014C )
//		peheader->OptionalHeader.SectionAlignment != PAGE_SIZE) // �ڴ��ж�������
	{
		return FALSE;
	}

	return TRUE;
}

// ����DLL����
HMODULE LoadMyDlls(PTSTR szFileName, DWORD BaseAddress)
{
	HMODULE hModule = MapThePeFile(szFileName);
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
	FixIMPORT(hModule);

	return hModule;
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
		( (DWORD)hmodule + peheader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

    // IAT��ַ
    DWORD pExportAddr = (DWORD)hmodule + peheader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    DWORD dwExportSize = peheader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;


	PSTR* NameRVA = (PSTR*)((DWORD)hmodule + pExportDir->AddressOfNames );
	PDWORD FuncAddr = (PDWORD)((DWORD)hmodule + pExportDir->AddressOfFunctions);
	PWORD Ordinal = (PWORD)((DWORD)hmodule + pExportDir->AddressOfNameOrdinals);

	// ���������Ƶ����ĺ���
    LPSTR funaddr = 0;
	if ( HIWORD(szFuncName) > 0 )
	{
		__try
		{
			for (i=0; i<pExportDir->NumberOfNames; i++)
			{
                if (NameRVA[i] == 0)
                {
                    continue;
                }
				if( strcmp( (PSTR)((DWORD)NameRVA[i]+(DWORD)hmodule), szFuncName) == 0)
				{
					WORD Hint = Ordinal[i];
					funaddr = (PSTR)hmodule+FuncAddr[Hint];
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
		DWORD firstOrd = pExportDir->Base;
		PDWORD FuncAddr = (PDWORD)((DWORD)hmodule + pExportDir->AddressOfFunctions); 
		for (i=0; i<pExportDir->NumberOfFunctions; i++)
		{
			if ( LOWORD(szFuncName) == Ordinal[i] + firstOrd )
			{
                funaddr = (PSTR)hmodule+FuncAddr[i];
                break;
			}
		}
	}
	
    // ����Ǻ���ת���򣬼�����
	PCHAR filename = (PCHAR)funaddr;
    if (funaddr != NULL && pExportAddr <= (DWORD)funaddr && pExportAddr+dwExportSize >= (DWORD)funaddr &&
		(filename[0]|0x20) >= 'a' && (filename[0]|0x20) <= 'z' &&
		(filename[1]|0x20) >= 'a' && (filename[1]|0x20) <= 'z' 
		)
    {
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
        HMODULE dllhmodule = GetModuleHandle(DllName);
        return MyGetProcAddress(dllhmodule, funaddr);
    }

	return funaddr;
}


BOOL FreeMyDlls(HMODULE hmodule)
{
	if (hmodule == NULL)
	{
		return FALSE;
	}
	return HeapFree(GetProcessHeap(), 0, hmodule);
}
