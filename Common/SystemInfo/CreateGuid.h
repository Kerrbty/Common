#pragma once
#include <windows.h>
#include <tchar.h>


// ��ȡһ��ϵͳΨһ��ʶ��
char* GetClientGuid(char* pbuf, DWORD dwSize);

// ���浱ǰguid
bool WriteGuid(char* curver, char* szguid);

