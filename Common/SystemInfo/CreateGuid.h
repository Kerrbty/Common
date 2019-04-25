#pragma once
#include <windows.h>
#include <tchar.h>


// 获取一个系统唯一标识符
char* GetClientGuid(char* pbuf, DWORD dwSize);

// 保存当前guid
bool WriteGuid(char* curver, char* szguid);

