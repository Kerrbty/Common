#ifndef _SYSTEM_INFO_HEADER_HH_H_
#define _SYSTEM_INFO_HEADER_HH_H_
#include <windows.h>
#include <tchar.h>


#ifdef _UNICODE
#define GetPEFileVer  GetPEFileVerW
#else
#define GetPEFileVer  GetPEFileVerA
#endif

// 获取PE文件版本信息
BOOL GetPEFileVerA( LPSTR szPath, DWORD* dwWinMajor, DWORD* dwWinMinor);
BOOL GetPEFileVerW(LPWSTR szPath, DWORD* dwWinMajor, DWORD* dwWinMinor);

// 获取系统位数，并获取系统版本
ULONG GetSystemBits(DWORD* dwWinMajor, DWORD* dwWinMinor);

// 获取MAC地址(大写字母字符串)
BOOL GetMacAddress(char* buf, int len);

// 检测是否以管理员方式运行（过UAC） 
BOOL IsRunAsAdmin();

// 以管理员方式运行 
BOOL RunAsAdmin();


#endif // _SYSTEM_INFO_HEADER_HH_H_
