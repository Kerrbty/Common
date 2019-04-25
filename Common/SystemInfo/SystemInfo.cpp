#include <WinSock2.h>
#include "SystemInfo.h"
#include "Wow64Set.h"
#include <iphlpapi.h>  
#include "Common.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib") 
#pragma comment(lib, "Version.lib")

BOOL GetMacAddress(char* buf, int len)  
{  
    BOOL bResult = FALSE;
    PIP_ADAPTER_INFO pAdapterInfo;  
    DWORD AdapterInfoSize;  
    TCHAR szMac[32]   =   {0};  
    AdapterInfoSize   =   0;  

    // buffer 不够
    if (len<=6*2)
    {
        return FALSE;
    }
    if( GetAdaptersInfo(NULL, &AdapterInfoSize) ==   ERROR_BUFFER_OVERFLOW )
    { 
        pAdapterInfo = (PIP_ADAPTER_INFO)AllocMemory(AdapterInfoSize);  
        if(pAdapterInfo   !=   NULL)
        { 
            if(GetAdaptersInfo(pAdapterInfo, &AdapterInfoSize) == 0)
            {  
                PIP_ADAPTER_INFO tmpAdaptInfo = pAdapterInfo;
                while(tmpAdaptInfo)
                {
                    BOOL bActivityNetCard = FALSE;
                    PIP_ADDR_STRING GateWayAddr = &tmpAdaptInfo->GatewayList;
                    while(GateWayAddr)
                    {
                        if( strcmp(GateWayAddr->IpAddress.String, "0.0.0.0") != NULL )
                        {
                            bActivityNetCard = TRUE;
                        }
                        GateWayAddr = GateWayAddr->Next;
                    }
                    if (/*MIB_IF_TYPE_ETHERNET == tmpAdaptInfo->Type && */ bActivityNetCard)
                    {
                        wsprintfA(buf, "%02X%02X%02X%02X%02X%02X",    
                            tmpAdaptInfo->Address[0],  
                            tmpAdaptInfo->Address[1],  
                            tmpAdaptInfo->Address[2],  
                            tmpAdaptInfo->Address[3],  
                            tmpAdaptInfo->Address[4],  
                            tmpAdaptInfo->Address[5]);
                        break;
                    }
                    tmpAdaptInfo = tmpAdaptInfo->Next;
                }
                bResult = TRUE;
            }     
            FreeMemory(pAdapterInfo);  

        }       
    }  
    return bResult;
} 


BOOL GetPEFileVerW(LPWSTR szPath, DWORD* dwWinMajor, DWORD* dwWinMinor)
{
    BOOL GetVerSuccess = FALSE;
    DWORD dwHandle;
    DWORD dwSize = GetFileVersionInfoSizeW(szPath, &dwHandle);
    PWSTR pVersionData = (PWSTR)AllocMemory(dwSize*sizeof(WCHAR));
    if(GetFileVersionInfoW(szPath, dwHandle, dwSize,(void**)pVersionData))  
    {
        UINT dwQuerySize = 0;  
        VS_FIXEDFILEINFO *pTransTable ;
        if (VerQueryValueW(pVersionData, L"\\",(LPVOID*)&pTransTable, &dwQuerySize))  
        {  
            WORD WinMajor = HIWORD(pTransTable->dwProductVersionMS);//major version number    
            WORD WinMinor = LOWORD(pTransTable->dwProductVersionMS); //minor version number

            if (dwWinMajor != NULL)
            {
                *dwWinMajor = WinMajor;
            }
            if (dwWinMinor != NULL)
            {
                *dwWinMinor = WinMinor;
            }
            GetVerSuccess = TRUE;
        }  
    }
    FreeMemory(pVersionData);
    return GetVerSuccess;
}


BOOL GetPEFileVerA(LPSTR szPath, DWORD* dwWinMajor, DWORD* dwWinMinor)
{
    PWSTR  pElementText;
    int    iTextLen;
    BOOL   bSuccess;

    iTextLen = MultiByteToWideChar( CP_ACP,
        0,
        szPath,
        -1,
        NULL,
        0 );

    pElementText = 
        (PWSTR)AllocMemory((iTextLen+1)*sizeof(WCHAR));

    MultiByteToWideChar( CP_ACP,
        0,
        szPath,
        -1,
        pElementText,
        iTextLen );
    bSuccess = GetPEFileVerW(pElementText, dwWinMajor, dwWinMinor);

    FreeMemory(pElementText);
    return bSuccess;
}


// 得到系统位数，并得到系统版本号
ULONG GetSystemBits(DWORD* dwWinMajor, DWORD* dwWinMinor)
{
    static ULONG retbit = -1;
    static DWORD dwMajor, dwMinor;

    // 已经查过了，那就直接返回之前记录的 
    if (retbit == -1)
    {
        // 没查过，分32位 64位
        PWSTR szPath = (PWSTR)AllocMemory(MAX_PATH*sizeof(WCHAR));

#ifdef _WIN64

        // 64 位程序能运行，必须64位系统

        DWORD dwHandle;
        ExpandEnvironmentStringsW(L"%SystemRoot%\\system32\\ntdll.dll", szPath, MAX_PATH);

        // 获取系统版本信息
        GetPEFileVer(szPath, &dwMajor, &dwMinor);
        retbit = 64;

#else  /* _WIN64 */ 

        // 获取%systemdir%/system32/ntdll.dll 
        DWORD dwBytes = 0;

        PBYTE pbuf = (PBYTE)AllocMemory(0x1024);

        do 
        {
            // 禁用系统重定位，查询ntdll.dll的位数
            EnableWow64FsRedirection(FALSE);
            ExpandEnvironmentStringsW(L"%SystemRoot%\\system32\\ntdll.dll", szPath, MAX_PATH);

            // 获取系统版本信息
            GetPEFileVerW(szPath, &dwMajor, &dwMinor);

            HANDLE hNtdll = CreateFileW(szPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hNtdll == INVALID_HANDLE_VALUE)
            {
                break;
            }
            SetFilePointer(hNtdll, 0, NULL, FILE_BEGIN);
            ReadFile(hNtdll, pbuf, 0x1000, &dwBytes, NULL);
            CloseHandle(hNtdll);
            if (dwBytes == 0)
            {
                break;
            }
            // 解析是不是64位的
            PIMAGE_DOS_HEADER Header = (PIMAGE_DOS_HEADER)pbuf;
            if (Header->e_magic != IMAGE_DOS_SIGNATURE)
            {
                // 不是有效的PE文件
                break;
            }
            PIMAGE_NT_HEADERS peheader = 
                (PIMAGE_NT_HEADERS)((LPBYTE)Header + Header->e_lfanew);
            if ((PBYTE)peheader < pbuf || (PBYTE)peheader > (pbuf+0x1000) )
            {
                break;
            }
            if (peheader->Signature != IMAGE_NT_SIGNATURE)
            {
                break;
            }

            if ( peheader->FileHeader.Machine == IMAGE_FILE_MACHINE_I386 /*i386*/ )
            {
                retbit = 32;
            }
            else if (
                peheader->FileHeader.Machine == IMAGE_FILE_MACHINE_IA64 /* IA64 */ ||
                peheader->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64 /* AMD64 */
                )
            {
                retbit = 64;
            }

        } while (false);

        EnableWow64FsRedirection(TRUE);
        FreeMemory(pbuf);    
#endif
        FreeMemory(szPath);
    }

    if (dwWinMajor != NULL)
    {
        *dwWinMajor = dwMajor;
    }
    if (dwWinMinor != NULL)
    {
        *dwWinMinor = dwMinor;
    }
    return retbit;
}


// 检测是否以管理员方式运行（过UAC） 
BOOL IsRunAsAdmin()
{
    DWORD dwWinMajor, dwWinMinor;
    DWORD dwBites = GetSystemBits(&dwWinMajor, &dwWinMinor);
    if (dwWinMajor < 6)
    {
        // xp 的话直接是管理员无疑
        return TRUE;
    }

    // vista 以后系统继续查询 
    BOOL bElevated = FALSE;  
    HANDLE hToken = NULL;  

    // Get current process token
    if ( !OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &hToken ) )
        return FALSE;

    TOKEN_ELEVATION tokenEle;
    DWORD dwRetLen = 0;  

    // Retrieve token elevation information
    if ( GetTokenInformation( hToken, TokenElevation, &tokenEle, sizeof(tokenEle), &dwRetLen ) ) 
    {  
        if ( dwRetLen == sizeof(tokenEle) ) 
        {
            bElevated = tokenEle.TokenIsElevated;  
        }
    }  

    CloseHandle( hToken );  
    return bElevated;  
}

// 以管理员方式运行 
BOOL RunAsAdmin()
{
    BOOL retvalue = FALSE;
    SHELLEXECUTEINFO ExecInfo = {sizeof(SHELLEXECUTEINFO)};

    ExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    ExecInfo.lpVerb = TEXT("runas");

    // Exe 模块 
    LPTSTR szExeName = (LPTSTR)AllocMemory(MAX_PATH*sizeof(TCHAR));
    GetModuleFileName(GetModuleHandle(NULL), szExeName, MAX_PATH);
    ExecInfo.lpFile = szExeName; // exe 

    // 参数 
    LPTSTR lpCommandLine = GetCommandLine();
    if (*lpCommandLine == TEXT('"') ) // 第一个字符是引号,路径被引号包围
    {
        do 
        {
            lpCommandLine++;
        } while ( *lpCommandLine != TEXT('\0') && *lpCommandLine != TEXT('"'));

        if (*lpCommandLine != TEXT('\0'))
        {
            do 
            {
                lpCommandLine++;
            } while ( *lpCommandLine != TEXT(' ') ||  *lpCommandLine == TEXT('\t'));
        }
    }
    else
    {
        do 
        {
            lpCommandLine++;
        } while ( *lpCommandLine != TEXT('\0') && *lpCommandLine != TEXT(' ') && *lpCommandLine != TEXT('\t') );

        if (*lpCommandLine != TEXT('\0'))
        {
            do 
            {
                lpCommandLine++;
            } while ( *lpCommandLine != TEXT(' ') ||  *lpCommandLine == TEXT('\t'));
        }
    }
    ExecInfo.lpParameters = lpCommandLine; 

    retvalue = ShellExecuteEx(&ExecInfo);
    FreeMemory(szExeName);
    return retvalue;
}