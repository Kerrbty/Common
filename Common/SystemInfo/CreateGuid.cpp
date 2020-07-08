#include "CreateGuid.h"
#include <time.h>
#include "SystemInfo.h"
#include "../Http/HttpPost.h"

static char* WINAPI CreateNewClientGuid(char* pbuf, DWORD dwSize)
{

    if (pbuf == NULL || dwSize <= 39)
    {
        return NULL;
    }

    memset(pbuf, 0, dwSize);
    if( !GetMacAddress(pbuf, 20) )
    {
        time_t t = time(NULL);
        char cb = 'A'+((t&0xFF)%3);
        pbuf[0] = pbuf[1] = cb;

        cb = 'A'+((t&0xFF)%7);
        pbuf[2] = pbuf[3] = cb;

        cb = 'A'+(((t>>8)&0xFF)%10);
        pbuf[4] = pbuf[5] = cb;

        cb = 'A'+(((t>>16)&0xFF)%10);
        pbuf[6] = pbuf[7] = cb;

        cb = 'A'+(((t>>24)&0xFF)%10);
        pbuf[8] = pbuf[9] = cb;

        cb = 'A'+((t&0xFF)%13);
        pbuf[10] = pbuf[11] = cb;
    }
    pbuf[12] = '-';

    DWORD ip = 0;
    LPSTR lpPostResult = (LPSTR)GetHttpData("http://www.ip138.com/ips1388.asp", NULL);
    if (lpPostResult != NULL)
    {
        LPSTR ipaddr = strstr(lpPostResult, "您的IP地址");
        if (ipaddr)
        {
            ipaddr = strchr(ipaddr, '[');
            LPSTR ipend = strchr(ipaddr, ']');
            if (ipaddr && ipend)
            {
                *ipend = '\0';
                ip = inet_addr(ipaddr+1);
            }
        }
        FreeMemory(lpPostResult);
    }
    if (ip == 0)
    {
        LPSTR lpPostResult = (LPSTR)GetHttpData("http://only-997227-218-13-34-109.nstool.netease.com", NULL);
        if (lpPostResult != NULL)
        {
            LPSTR ipaddr = strstr(lpPostResult, "您的IP地址");
            if (ipaddr)
            {
                ipaddr = strchr(ipaddr, ' ');
                LPSTR ipend = strchr(ipaddr+1, ' ');
                if (ipaddr && ipend)
                {
                    *ipend = '\0';
                    ip = inet_addr(ipaddr+1);
                }
            }
            FreeMemory(lpPostResult);
        }
    }
    wsprintfA(pbuf+13, "%08X-", ip);

    // 系统内存大小，最大支持显示 1024-1 GB
    // +22 
    MEMORYSTATUSEX memstatus = {sizeof(MEMORYSTATUSEX)};
    LARGE_INTEGER la;
    GlobalMemoryStatusEx(&memstatus);
    la.QuadPart = memstatus.ullTotalPhys;
    wsprintfA(pbuf+22, "%02X%08X-", la.HighPart, la.LowPart);

    // 系统版本号
    DWORD MajorVer = 0;
    DWORD MinorVer = 0;
    DWORD bits = GetSystemBits(&MajorVer, &MinorVer);
    wsprintfA(pbuf+33, "%02u%02u%02u", bits, MajorVer, MinorVer);

    return pbuf;
}


char* WINAPI GetClientGuid(char* pbuf, DWORD dwSize)
{
    HKEY hKey;
    // 查询注册表
    if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\MSSec\\Plugins"), NULL, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        DWORD dwType;
        DWORD cbData = dwSize;
        if (RegQueryValueExA(hKey, "SmartInfoMd", NULL, &dwType, (LPBYTE)pbuf, &cbData) == ERROR_SUCCESS)
        {
            RegCloseKey(hKey);
            return pbuf;
        }
        RegCloseKey(hKey);
    }

    // 查询 win.ini
    if (GetProfileStringA("MSSecPlugins", "SmartInfoMd", "", pbuf, dwSize) != 0 )
    {
        return pbuf;
    }

    return CreateNewClientGuid(pbuf, dwSize);
}

bool WINAPI WriteGuid(char* curver, char* szguid)
{
    if (szguid != NULL)
    {
        // 放入注册表
        HKEY hKey;
        if ( RegCreateKey(HKEY_CURRENT_USER, TEXT("Software\\MSSec\\Plugins"), &hKey) == ERROR_SUCCESS )
        {
            RegSetValueExA(hKey, "SmartInfoMd", 0, REG_SZ, (LPBYTE)szguid, strlen(szguid)+1);
            RegSetValueExA(hKey, "version", 0, REG_SZ, (LPBYTE)curver, strlen(curver)+1);
            RegCloseKey(hKey);
        }

        // 写入win.ini
        WriteProfileStringA("MSSecPlugins", "SmartInfoMd", szguid);
        WriteProfileStringA("MSSecPlugins", "version", curver);
        return true;
    }
    return false;
}