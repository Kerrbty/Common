#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <tchar.h>

#pragma comment(lib, "kernel32")
#pragma comment(lib, "user32")


#ifdef _UNICODE
#define _tfscanf fwscanf
#define _tsnprintf _snwprintf
#define _tfprintf fwprintf
#else
#define _tfscanf fscanf
#define _tsnprintf _snprintf
#define _tfprintf fprintf
#endif

BOOL Stdcall = TRUE;
BOOL delasm = TRUE;

const TCHAR AsmHeaderStdStr[] = TEXT("\t\t.386\n"
                                  "\t\t.model\tflat,stdcall \n"
                                  "\toption\tcasemap:none\t\n"
                                  "\t\t.code\n"
                                  ";DLL入口\n"
                                  "DllEntry\tproc\t_hInstance,_dwReason,_dwReserved\n"
                                  "\t\tmov\t\teax,1\n"
                                  "\t\tret\n"
                                  "DllEntry\tEndp\n"
                                  ";导出函数\n");

const TCHAR AsmHeaderCStr[] = TEXT("\t\t.386\n"
                                  "\t\t.model\tflat,C \n"
                                  "\toption\tcasemap:none\t\n"
                                  "\t\t.code\n"
                                  ";DLL入口\n"
                                  "DllEntry\tproc\t_hInstance,_dwReason,_dwReserved\n"
                                  "\t\tmov\t\teax,1\n"
                                  "\t\tret\n"
                                  "DllEntry\tEndp\n"
                                  ";导出函数\n");

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

VOID usage()
{
	TCHAR FileName[MAX_PATH];
	GetModuleFileName(NULL, FileName, MAX_PATH);
	_tprintf( TEXT("Windows API Maker v0.0.1\n") );
	_tprintf( TEXT("Use the File to Make the *.lib files\n\n") );
	_tprintf( TEXT("%s -c -d [InputFile]\n"), FindLasteSymbol(FileName, TEXT('\\')) );
    _tprintf( TEXT("  -c    用cdecl的调用方式创建lib [默认stdcall]\n") );
    _tprintf( TEXT("  -d    不删除文ASM件\n\n") );
	_tprintf( TEXT("[-] 程序会创建与输入文件同名lib文件, 用于静态链接导出函数\n") );
	_tprintf( TEXT("[-] 输入的文件内部数据格式为 (函数名  参数个数)\n") );
	_tprintf( TEXT("[-] 如: GetModuleHandleExA  3 \n") );
    
}

BOOL Make2File(PSTR filename)
{
	FILE *rfile, *wafile, *wdfile;
	TCHAR FuncName[MAX_PATH] = {0};
	TCHAR OutFileName[MAX_PATH] = {0};
	DWORD FuncValue = 0;

	if( (rfile = _tfopen(filename, "r+" )) == NULL )
	{
		_tprintf( TEXT("open failed on %s file"), filename);
		return FALSE;
	}

	_tcsnccpy( OutFileName, FindLasteSymbol(filename, TEXT('\\')), MAX_PATH );
	_tcsnccpy( FindLasteSymbol(OutFileName, TEXT('.')), TEXT("asm"), MAX_PATH - _tcslen(OutFileName));
	if( (wafile = _tfopen(OutFileName, "w" )) == NULL )
	{
		_tprintf( TEXT("open failed on %s file"), OutFileName);
		fclose(rfile);
		return FALSE;
	}

	_tcsnccpy(FindLasteSymbol(OutFileName, TEXT('.')), TEXT("def"), MAX_PATH - _tcslen(OutFileName));
	if( (wdfile = _tfopen(OutFileName, "w" )) == NULL )
	{
		_tprintf( TEXT("open failed on %s file"), OutFileName);
		fclose(wafile);
		fclose(rfile);
		return FALSE;
	}

    if (Stdcall)
    {
        _tfprintf(wafile, TEXT("%s"), AsmHeaderStdStr);
    }
    else
    {
        _tfprintf(wafile, TEXT("%s"), AsmHeaderCStr);
    }
	
	_tfprintf(wdfile, TEXT("EXPORTS\n"));
	while( _tfscanf(rfile, TEXT("%s %d"), FuncName, &FuncValue) != -1 )
	{
		_tprintf( TEXT("Find function \"%s\" in %s\n"), FuncName, filename);
		_tfprintf(wafile, TEXT("%s\tproc\t"), FuncName);
		for (DWORD i=0; i<FuncValue; i++)
		{
			_tsnprintf(OutFileName, MAX_PATH, TEXT(",argument_%d "), i);
			_tfprintf(wafile, OutFileName);
		}
		_tfprintf(wafile, TEXT("\n\t\txor\t\teax,eax\n\t\tret\n"));
		_tfprintf(wafile, TEXT("%s\tendp\n\n"), FuncName);

		_tfprintf(wdfile, TEXT("\t\t%s\n"), FuncName);
	}
	_tfprintf(wafile, TEXT("end DllEntry"));

	fclose(wdfile);
	fclose(wafile);
	fclose(rfile);
	return TRUE;
}


// 将资源保存为文件
BOOL SaveResFile(PTSTR szSaveFileName, PTSTR ResId, PTSTR szResName)
{
	HMODULE module = GetModuleHandle(NULL);
	HRSRC res = FindResource( module, ResId, szResName);
	HGLOBAL hglob = LoadResource(module, res);
	DWORD dwsize = SizeofResource(module, res);
	if (res == NULL)
	{
#ifdef _DEBUG
		printf("FindResource error: %d\n", GetLastError());
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
		printf("CreateFile Error: %d\n", GetLastError());
#endif
		return FALSE;
	}
	DWORD dwBytes = 0;
	WriteFile(hpfile, pres, dwsize, &dwBytes, NULL);

	CloseHandle(hpfile);
	UnlockResource(hglob);
//	Sleep(100);
	return TRUE;
}

BOOL CreateResFile()
{
	if ( !SaveResFile( TEXT("ml.exe"), TEXT("ML"), TEXT("BINARY") ) ||
		!SaveResFile( TEXT("link.exe"), TEXT("LINK"), TEXT("BINARY") ) ||
		!SaveResFile( TEXT("mspdb50.dll"), TEXT("MSPDB50"), TEXT("BINARY") ))
	{
		_tprintf( TEXT("Create the Link file error\n") );
		return FALSE;
	}
	return TRUE;
}

// 运行命令
BOOL RunCommand(PTSTR Runapp, PTSTR CommandLine)
{
	STARTUPINFO sInfo; 
	PROCESS_INFORMATION pInfo;
	
	ZeroMemory( &sInfo, sizeof(sInfo) ); 
	sInfo.cb = sizeof(sInfo); 
	sInfo.dwFillAttribute = STARTF_USEFILLATTRIBUTE;
	sInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES ; 
	sInfo.wShowWindow = SW_HIDE;
	ZeroMemory( &pInfo, sizeof(pInfo) ); 

	if( ::CreateProcess( 
		Runapp,    
		CommandLine,  
		NULL,  
		NULL,  
		TRUE,  
		CREATE_NEW_CONSOLE,  
		NULL,  
		NULL,  
		&sInfo,  
		&pInfo )             
		)  
	{
		WaitForSingleObject( pInfo.hProcess, INFINITE );
		CloseHandle( pInfo.hThread );
		CloseHandle( pInfo.hProcess );
		return TRUE;
	}
	return FALSE;
}

BOOL MakeLibFile(PSTR filename)
{
	TCHAR Parameters[MAX_PATH] = {0};
	TCHAR szFile[MAX_PATH] = {0};

	// 编译lib
	_tcsnccpy(szFile, FindLasteSymbol(filename, TEXT('\\')), MAX_PATH);
	FindLasteSymbol(szFile, TEXT('.'))[0] = TEXT('\0');

	_tsnprintf(Parameters, MAX_PATH, TEXT("/c /coff /Cp %sasm"), szFile );
	RunCommand(TEXT("ml.exe"), Parameters);

	// link  /section:.bss,S   /dll   /def:%%i /subsystem:windows *.obj 
	_tsnprintf(	Parameters, 
				MAX_PATH, 
				TEXT("/section:.bss,S /dll /def:%sdef /subsystem:windows %sobj "),
				szFile, szFile );
	RunCommand(TEXT("link.exe"), Parameters);

	Sleep(500);
	// 清除编译中间变量
    if (delasm)
    {
        _tcsnccat(szFile, TEXT("asm"), MAX_PATH-_tcslen(szFile));
	    DeleteFile(szFile);
    }

	_tcsnccpy(FindLasteSymbol(szFile, TEXT('.')), TEXT("def"), MAX_PATH-_tcslen(szFile));
    DeleteFile(szFile);

	_tcsnccpy(FindLasteSymbol(szFile, TEXT('.')), TEXT("obj"), MAX_PATH-_tcslen(szFile));
	DeleteFile(szFile);

	_tcsnccpy(FindLasteSymbol(szFile, TEXT('.')), TEXT("exp"), MAX_PATH-_tcslen(szFile));
	DeleteFile(szFile);

	_tcsnccpy(FindLasteSymbol(szFile, TEXT('.')), TEXT("dll"), MAX_PATH-_tcslen(szFile));
	DeleteFile(szFile);

 	_tcsnccpy(szFile, filename, MAX_PATH);
	_tcsnccpy(FindLasteSymbol(szFile, TEXT('.')), TEXT("lib"), MAX_PATH-_tcslen(filename));
	_tcsnccpy(Parameters, FindLasteSymbol(filename, TEXT('\\')), MAX_PATH);
	_tcsnccpy( FindLasteSymbol(Parameters, TEXT('.')) , TEXT("lib"), MAX_PATH-_tcslen(Parameters));
	
//	DeleteFile(szFile);
 	MoveFile(Parameters, szFile);

	return TRUE;
}

VOID CleanTheLinkFile()
{
	DeleteFile( TEXT("ml.exe") );
	DeleteFile( TEXT("link.exe") );
	DeleteFile( TEXT("mspdb50.dll") );
}

INT main(INT argc, TCHAR* argv[])
{
	if (argc < 2)
	{
		usage();
		return -1;
	}

    for (int i=1; i<argc; i++)
    {
        if (      memicmp(argv[i], TEXT("/c"), 3*sizeof(TCHAR) ) == 0 ||
            memicmp(argv[i], TEXT("-c"), 3*sizeof(TCHAR) ) == 0 )
        {
            Stdcall = FALSE;
        }
        else if (      memicmp(argv[i], TEXT("/d"), 3*sizeof(TCHAR) ) == 0 ||
            memicmp(argv[i], TEXT("-d"), 3*sizeof(TCHAR) ) == 0 )
        {
            delasm = FALSE;
        }
    }

	try
	{
		CreateResFile();
		for (INT i=1; i<argc; i++)
		{
            if (argv[i][0] != TEXT('/') && argv[i][0] != TEXT('-'))
            {
                if (FindLasteSymbol(argv[i], TEXT('.')) == argv[i])
                {
                    _tprintf( TEXT("so sorry, %s 不能找到文件后缀名\n"), argv[i] );
                }
                else
                {
                    Make2File(argv[i]);
                    MakeLibFile(argv[i]);
			    }
            }
			
		}
	}
	catch(...)
	{
		CleanTheLinkFile();
	}
	CleanTheLinkFile();
	return 0;
}