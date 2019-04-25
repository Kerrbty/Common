#ifndef _PROCESS_INFO_HEADER_HH_H_
#define _PROCESS_INFO_HEADER_HH_H_
#include <Windows.h>
#include <tchar.h>
#ifdef __cplusplus
extern "C" {
#endif


BOOL  WINAPI RunProcess(LPTSTR lpImage, LPTSTR commandLine, BOOL isUIProcess = FALSE);
DWORD WINAPI GetPIDByName(LPTSTR lpName);
BOOL  WINAPI GetTokenByName(HANDLE &hToken, LPTSTR lpName) ;
BOOL  WINAPI KillProcessByName(LPTSTR lpName);
BOOL  WINAPI KillProcessByPid(DWORD pid);


#ifdef __cplusplus
};
#endif
#endif // _PROCESS_INFO_HEADER_HH_H_