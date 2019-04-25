#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

enum {PEFILE, SIGFILE};

LPTSTR FindLastSymbalInStr(LPTSTR CommandLine, TCHAR FindWchar)
{
	int Len;
	for ( Len = _tcslen(CommandLine) ; Len>0; Len-- )
	{
		if (CommandLine[Len] == FindWchar)
		{
			break;
		}
	}
	return &CommandLine[Len];
}

// ����ǩ��ƫ�Ƽ���С
BOOL SetSigOffsetInFile( HANDLE FileHanlde, DWORD SigOffset, DWORD SigSize )
{
    HANDLE pWrite = CreateFileMapping(FileHanlde,
        NULL,  
        PAGE_READWRITE,  
        0,  
        0,  
        NULL);  
    LPVOID pFile = MapViewOfFile(pWrite,  
        FILE_MAP_WRITE,  
        0,  
        0,  
        0);  
    if ( pWrite == NULL || pFile == NULL)  
    {  
        return FALSE;  
    }  

	PIMAGE_DOS_HEADER Header = (PIMAGE_DOS_HEADER)pFile;
	PIMAGE_NT_HEADERS peheader = 
		(PIMAGE_NT_HEADERS)((DWORD)Header + Header->e_lfanew);
	// PE ͷ offset 0x98
	peheader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY].VirtualAddress = SigOffset;
	peheader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY].Size = SigSize;
	CloseHandle(pWrite);

	return TRUE;
}

// ��ȡָ����ǩ��д��pefile�ļ�
BOOL ReadWriteSigFile( HANDLE pefile, PTSTR FileName, DWORD filetyep, DWORD* signalsize )
{
	LPBYTE szbuf = NULL;
	DWORD dwBytes = 0;

	HANDLE pfhand = CreateFile(FileName,   
		GENERIC_READ|GENERIC_WRITE,  
		FILE_SHARE_READ|FILE_SHARE_WRITE,  
		NULL,  
		OPEN_EXISTING,  
		0,  
		NULL);
	if ( pfhand == INVALID_HANDLE_VALUE )  
	{  
		return NULL;  
	} 

	HANDLE pWrite = CreateFileMapping(pfhand,
		NULL,  
		PAGE_READWRITE,  
		0,  
		0,  
		NULL);  
	LPVOID pFile = MapViewOfFile(pWrite,  
		FILE_MAP_WRITE,  
		0,  
		0,  
		0); 
	if ( pWrite == NULL || pFile == NULL)  
	{  
		return NULL;  
	} 

	if (filetyep == PEFILE)
	{
		PIMAGE_DOS_HEADER Header = (PIMAGE_DOS_HEADER)pFile;
		PIMAGE_NT_HEADERS32 peheader = 
			(PIMAGE_NT_HEADERS32)((DWORD)Header + Header->e_lfanew);
		// PE ͷ offset 0x98
		szbuf = (LPBYTE)( (DWORD)Header + peheader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY].VirtualAddress);
		*signalsize = peheader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY].Size ;
	}
	else if (filetyep == SIGFILE)
	{
		szbuf = (LPBYTE)pFile;
		*signalsize = GetFileSize(pfhand, NULL);
	}
	else
	{
		CloseHandle(pWrite);
		CloseHandle(pfhand);
		return FALSE;
	}

	SetFilePointer(pefile, 0, NULL, FILE_END);
	WriteFile(pefile, szbuf, *signalsize, &dwBytes, NULL);

	CloseHandle(pWrite);
	CloseHandle(pfhand);
	return TRUE;
}

// �����ļ�
BOOL CreateBackFile(PTSTR szFile)
{
	PTSTR BackFileName = (PTSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_PATH*sizeof(TCHAR));

	_tcsncpy(BackFileName, szFile, MAX_PATH);
	_tcsncpy(FindLastSymbalInStr(BackFileName, TEXT('.')), TEXT(".bak"), MAX_PATH-_tcslen(BackFileName));
	CopyFile(szFile, BackFileName, FALSE);

	HeapFree(GetProcessHeap(), NULL, BackFileName);
	return TRUE;
}

// ������ǩ���ĳ��򲢱���ԭ����
BOOL MakeSigFile(PTSTR szFile, PTSTR szSig, DWORD dwtype)
{
	if (dwtype >= 2)
	{
		return FALSE;
	}
	// ����ԭ�ļ� Ϊ .bak
	CreateBackFile(szFile);

	// ���ļ�д��ǩ��
	HANDLE hfile = CreateFile(	szFile, 
								GENERIC_READ|GENERIC_WRITE,
								FILE_SHARE_READ,
								NULL,
								OPEN_EXISTING,
								FILE_ATTRIBUTE_NORMAL,
								NULL);
	if (hfile == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	DWORD szFileSize = GetFileSize(hfile, NULL);

	// д��ǩ��
	DWORD sigsize;
	if (!ReadWriteSigFile(hfile, szSig, dwtype, &sigsize))
	{
		return FALSE;
	}

	// ����PEͷ����ǩ��ƫ�Ƽ���С���
	SetSigOffsetInFile(hfile, szFileSize, sigsize);

	CloseHandle(hfile);
	return TRUE;
}


void usage()
{
	TCHAR FileName[MAX_PATH];
	GetModuleFileName(NULL, FileName, MAX_PATH*sizeof(TCHAR));
	PTCHAR name = FindLastSymbalInStr(FileName, TEXT('\\'))+1;
	printf("%s /F FileName [ /SI SigFile | /SF SigInPeFile ]\n\n", name);
	printf("˵��: �� PE �ļ��м������������ǩ��\n");
	printf("  /F   ��Ҫ�����������ǩ����PE�ļ�\n");
	printf("  /SI  ������ȡ����������ǩ���ļ�\n");
	printf("  /SF  ��������ǩ����PE�ļ�\n\n");
	printf("%s /F HackPe.exe /SI signal.bin\n", name);
	printf("%s /F HackPe.exe /SF  vmplayer.exe\n", name);
}


// process  /f FileName [ /si SigFile | /sf SigInPeFile ]
int main(int argc, char* argv[])
{
	char szFileName[MAX_PATH] = {0};
	char szSigFile[MAX_PATH] = {0};
	DWORD dwsignal = -1; 

	if (argc < 5)
	{
		usage();
		return 0;
	}
	for (int i=1; i<argc; i++)
	{
		if (stricmp(argv[i], "/si") == 0)
		{
			dwsignal = SIGFILE;
			strcpy( szSigFile, argv[++i] );
		}
		else if ( stricmp(argv[i], "/sf") == 0 )
		{
			dwsignal = PEFILE;
			strcpy( szSigFile, argv[++i] );
		}
		else if ( stricmp(argv[i], "/f") == 0 )
		{
			strcpy(szFileName, argv[++i]);
		}
	}
	MakeSigFile(szFileName, szSigFile, dwsignal);

//	system("pause");
	return 0;
}