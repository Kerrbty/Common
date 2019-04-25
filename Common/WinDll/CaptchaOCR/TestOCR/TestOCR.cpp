#include <windows.h>
#include <stdio.h>
#include "OCR.h"


int main()
{
    BYTE* out_buf = new BYTE[20];

    //////////////////////////////////////////////////////////////////////////
    // 打开验证码1
    //////////////////////////////////////////////////////////////////////////
    HANDLE handle = CreateFile(TEXT("865473026104094.gif"),
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    if (handle == INVALID_HANDLE_VALUE)
    {
        printf("open file error!\n");
        return 0;
    }
    DWORD dwSize = GetFileSize(handle, NULL);
    BYTE* buf = new BYTE[dwSize];
    
    DWORD dwBytes;
    ReadFile(handle, buf, dwSize, &dwBytes, NULL);
    CloseHandle(handle);

    // 识别
    int id = VcodeInit();
    ZeroMemory(out_buf, 20);
    if( GetVcode(id, buf, dwSize, out_buf) == TRUE)
    {
        for (int j=0; j<20 && out_buf[j] != '\0'; j++)
        {
            printf("%c", out_buf[j]);
        }
        printf("\n");
    }
    delete []buf;


    //////////////////////////////////////////////////////////////////////////
    // 打开验证码2
    //////////////////////////////////////////////////////////////////////////
    handle = CreateFile(TEXT("vercode.gif"),
                               GENERIC_READ,
                               FILE_SHARE_READ,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL);
    if (handle == INVALID_HANDLE_VALUE)
    {
        printf("open file error!\n");
        return 0;
    }
    dwSize = GetFileSize(handle, NULL);
    buf = new BYTE[dwSize];

    ReadFile(handle, buf, dwSize, &dwBytes, NULL);
    CloseHandle(handle);

    // 识别
    id = VcodeInit();
    ZeroMemory(out_buf, 20);
    if( GetVcode(id, buf, dwSize, out_buf) == TRUE)
    {
        for (int j=0; j<20 && out_buf[j] != '\0'; j++)
        {
            printf("%c", out_buf[j]);
        }
        printf("\n");
    }
    delete []buf;


    delete []out_buf;
    return 0;
}