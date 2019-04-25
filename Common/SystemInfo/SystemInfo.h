#ifndef _SYSTEM_INFO_HEADER_HH_H_
#define _SYSTEM_INFO_HEADER_HH_H_
#include <windows.h>
#include <tchar.h>


#ifdef _UNICODE
#define GetPEFileVer  GetPEFileVerW
#else
#define GetPEFileVer  GetPEFileVerA
#endif

// ��ȡPE�ļ��汾��Ϣ
BOOL GetPEFileVerA( LPSTR szPath, DWORD* dwWinMajor, DWORD* dwWinMinor);
BOOL GetPEFileVerW(LPWSTR szPath, DWORD* dwWinMajor, DWORD* dwWinMinor);

// ��ȡϵͳλ��������ȡϵͳ�汾
ULONG GetSystemBits(DWORD* dwWinMajor, DWORD* dwWinMinor);

// ��ȡMAC��ַ(��д��ĸ�ַ���)
BOOL GetMacAddress(char* buf, int len);

// ����Ƿ��Թ���Ա��ʽ���У���UAC�� 
BOOL IsRunAsAdmin();

// �Թ���Ա��ʽ���� 
BOOL RunAsAdmin();


#endif // _SYSTEM_INFO_HEADER_HH_H_
