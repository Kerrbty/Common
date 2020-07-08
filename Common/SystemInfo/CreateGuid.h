#pragma once
#include <windows.h>
#include <tchar.h>


// 获取一个系统唯一标识符
char* WINAPI GetClientGuid(char* pbuf, DWORD dwSize);

// 保存当前guid
bool WINAPI WriteGuid(char* curver, char* szguid);

