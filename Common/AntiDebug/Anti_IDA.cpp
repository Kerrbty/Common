// Anti_IDA.cpp : 定义控制台应用程序的入口点。
//

#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <time.h>


BOOL BackFileW(LPCTSTR lpFileName)
{
    BOOL bret = FALSE;
    if (lpFileName == NULL)
    {
        return bret;
    }
    LPWSTR lpBackName = (LPWSTR)malloc((_tcslen(lpFileName)+6)*sizeof(TCHAR));
    if (lpBackName)
    {
        wsprintf(lpBackName, L"%s.bak", lpFileName);
        bret = CopyFile(lpFileName, lpBackName, TRUE);
        free(lpBackName);
    }
    return bret;
}

int ReplaceCode(unsigned char* buffer, unsigned int len)
{
    int ncount = 0;
    // __halt(), __nop(), __halt(), __nop(), __halt()  
    unsigned char code[5] = {0xF4, 0x90, 0xF4, 0x90, 0xF4};
    for (int i=0; i<len-sizeof(code); i++)
    {
        if (memicmp(buffer+i, code, sizeof(code)) == 0)
        {
            unsigned char j = rand()%0xF;
            unsigned char n = rand()%0x100;
            unsigned char repcode[5];
            // jx  03 
            repcode[0] = 0x70|j;
            repcode[1] = 0x03;
            // jnx 01 
            repcode[2] = 0x70|(j+1);
            repcode[3] = 0x01;
            // skip code 
            repcode[4] = n;
            memcpy(buffer+i, repcode, sizeof(repcode));
            ncount++;
        }
    }
    return ncount;
}

int Anti_IDA(LPCTSTR lpFile)
{
    int dwCount = 0;
    BackFileW(lpFile);
    
    HANDLE hFile = CreateFile(lpFile, 
        GENERIC_READ|GENERIC_WRITE, 
        FILE_SHARE_READ, 
        NULL, 
        OPEN_EXISTING, 
        FILE_ATTRIBUTE_NORMAL, 
        NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return dwCount;
    }
    DWORD dwSize = GetFileSize(hFile, NULL);
    unsigned char* buf = (unsigned char*)malloc(dwSize);
    do 
    {
        if (buf == NULL)
        {
            break;
        }

        DWORD dwBytes = 0;
        SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
        ReadFile(hFile, buf, dwSize, &dwBytes, NULL);

        // PE32 处理 
        PIMAGE_DOS_HEADER Header = (PIMAGE_DOS_HEADER)buf;
        PIMAGE_NT_HEADERS peheader = 
            (PIMAGE_NT_HEADERS)((LPBYTE)Header + Header->e_lfanew);
        PIMAGE_SECTION_HEADER SectionHeader = (PIMAGE_SECTION_HEADER)( (DWORD)peheader + 
            sizeof(peheader->FileHeader) + 
            sizeof(peheader->Signature) +
            peheader->FileHeader.SizeOfOptionalHeader ); // 节表项的开始

        WORD SectionNum = peheader->FileHeader.NumberOfSections; // 节数目
        for (WORD i=0; i<SectionNum; i++) // 将节一个个复制到内存中
        {
            DWORD ulsize = SectionHeader[i].Misc.VirtualSize;
            if ( ulsize > SectionHeader[i].SizeOfRawData )
            {
                ulsize = SectionHeader[i].SizeOfRawData;
            }
            dwCount += ReplaceCode((unsigned char*)Header + SectionHeader[i].PointerToRawData, ulsize); 
        } 
        CloseHandle(hFile);
    } while (FALSE);
    free(buf);
    return dwCount;
}

int _tmain(int argc, _TCHAR* argv[])
{
    srand(time(NULL));
    for (int i=1; i<argc; i++)
    {
        Anti_IDA(argv[i]);
    }
	return 0;
}

