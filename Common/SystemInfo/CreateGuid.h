#pragma once
#include <windows.h>
#include <tchar.h>


// ��ȡһ��ϵͳΨһ��ʶ��
char* WINAPI GetClientGuid(char* pbuf, DWORD dwSize);

// ���浱ǰguid
bool WINAPI WriteGuid(char* curver, char* szguid);

