#include "LoadModel.h"
#include "pestreat.h"


BOOL RelocAddr(HMODULE hModule, DWORD BaseAddress); // 修改重定位表
HMODULE MapThePeFile(PTSTR szFileName); // 检测并映射文件
BOOL FixIMPORT(HMODULE hModule); // 填充导入表
BOOL isPEFile(LPVOID pFileMap); // 判断文件是否是PE文件


typedef BOOL (APIENTRY* pDllMain)(  HINSTANCE hModule, 
								  DWORD  ul_reason_for_call, 
									LPVOID lpReserved);



// 修改重定位表 hmodule 加载的地址
// BaseAddress 将要重定位的地址，用来检查HOOK或者恢复hook时候用
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
		// 加载到理想基址,不需要重定位
		return TRUE;
	}

	// PE 头 offset 0x98
	pRELOADTABLE  reloadaddr = (pRELOADTABLE)
		( (DWORD)hModule + peheader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress) ;

	if ( (DWORD)reloadaddr <= (DWORD)hModule || 
		 (DWORD)reloadaddr >= (DWORD)hModule+dwsizeOfImage )
	{
		// 不落在这个文件内,或者没有重定位表,重定位表错误
		return false;
	}

	// 遍历重定位表
	while ( reloadaddr->StartVirtualAddress != NULL && reloadaddr->size != NULL )
	{
		for (DWORD i=0; i<(reloadaddr->size-8)/2 ; i++)
		{
			__try
			{
				if ( reloadaddr->Table[i].flags == 3 )
				{
					PDWORD OffsetAddress = (PDWORD)(reloadaddr->Table[i].addr + (DWORD)hModule + reloadaddr->StartVirtualAddress);
					// 计算新的偏移量 = 原始值 - 原始加载地址 + 新的加载地址
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


// 填充导入表
BOOL FixIMPORT(HMODULE hModule) 
{
	if (hModule == NULL)
	{
		return false;
	}
	
	PIMAGE_DOS_HEADER Header = (PIMAGE_DOS_HEADER)hModule;
	PIMAGE_NT_HEADERS peheader = 
		(PIMAGE_NT_HEADERS)((DWORD)Header + Header->e_lfanew);

	// IAT地址
	PIMAGE_IMPORT_DESCRIPTOR pImpDescript = (PIMAGE_IMPORT_DESCRIPTOR) 
		( (DWORD)hModule + peheader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

    if ((DWORD)pImpDescript == (DWORD)hModule)
    {
        return FALSE;
    }

	// 根据桥2指向的字符串指针修复桥1指向的内存块(桥1指向IMPORT table)
	while (pImpDescript->Name != NULL)
	{
		PDWORD ImportApiName = (PDWORD)((DWORD)hModule + pImpDescript->OriginalFirstThunk); // 导入函数名称数组
		// 需要修复的导入表地址 , 在 IMAGE_DIRECTORY_ENTRY_IAT 里面
		PDWORD FixTable = (PDWORD)((DWORD)hModule + pImpDescript->FirstThunk); 
		
		// 得到导入dll基址
		HMODULE impDllModule = GetModuleHandleA( (char*)((DWORD)hModule + pImpDescript->Name) );
		if (impDllModule == NULL)
		{
			impDllModule = LoadLibraryA( (char*)((DWORD)hModule + pImpDescript->Name) );
		}

		// 修复
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
// 检测并映射文件
// 这个映射过程可由 ntdll 导出的 
// ZwOpenFile(ZwCreateFile)\ ZwCreateSection \ ZwMapViewOfSection 
// 取代
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

	// 映射文件
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
	
	// 根据文件记录的大小申请内存
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
											peheader->FileHeader.SizeOfOptionalHeader ); // 节表项的开始

    //	DWORD dwFileAlignment = peheader->OptionalHeader.FileAlignment; // 文件对齐粒度
    memcpy(DllAddress, pFile, SectionHeader[0].SizeOfRawData); // 将PE头复制过去

	DWORD SectionNum = peheader->FileHeader.NumberOfSections; // 节数目
	for (DWORD i=0; i<SectionNum; i++) // 将节一个个复制到内存中
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

// 判断PE文件是否合法
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
//		peheader->OptionalHeader.SectionAlignment != PAGE_SIZE) // 内存中对齐粒度
	{
		return FALSE;
	}

	return TRUE;
}

// 加载DLL函数
HMODULE LoadMyDlls(PTSTR szFileName, DWORD BaseAddress)
{
	HMODULE hModule = MapThePeFile(szFileName);
	if (hModule == NULL)
	{
		return NULL;
	}
	
	// 刷写重定位表
	if ( !RelocAddr(hModule, BaseAddress) )
	{
		HeapFree(GetProcessHeap(), 0, hModule);
		return NULL;
	}

	// 刷新导入表
	FixIMPORT(hModule);

	return hModule;
}



////////////////////////////////////////////////////////
// 解析导出函数地址
////////////////////////////////////////////////////////
LPVOID MyGetProcAddress(HMODULE hmodule, PSTR szFuncName) 
{
	DWORD i = 0;
	// szFuncName == NULL 的时候不知道是传的函数名,还是传的函数hint值
	if (hmodule == NULL || szFuncName == NULL)
	{
		return NULL;
	}
	
	PIMAGE_DOS_HEADER Header = (PIMAGE_DOS_HEADER)hmodule;
	PIMAGE_NT_HEADERS peheader = 
		(PIMAGE_NT_HEADERS)((DWORD)Header + Header->e_lfanew);
	
	// 导出表地址
	PIMAGE_EXPORT_DIRECTORY pExportDir = (PIMAGE_EXPORT_DIRECTORY) // (pRELOADTABLE)
		( (DWORD)hmodule + peheader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

    // IAT地址
    DWORD pExportAddr = (DWORD)hmodule + peheader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
    DWORD dwExportSize = peheader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;


	PSTR* NameRVA = (PSTR*)((DWORD)hmodule + pExportDir->AddressOfNames );
	PDWORD FuncAddr = (PDWORD)((DWORD)hmodule + pExportDir->AddressOfFunctions);
	PWORD Ordinal = (PWORD)((DWORD)hmodule + pExportDir->AddressOfNameOrdinals);

	// 遍历以名称导出的函数
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
		// 遍历以序号导出的函数
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
	
    // 如果是函数转发则，继续找
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
