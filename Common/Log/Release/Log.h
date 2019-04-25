#ifndef _LOG_INTERFACE_H_LGJ_2015_03_04_
#define _LOG_INTERFACE_H_LGJ_2015_03_04_
#include <windows.h>
#pragma comment(lib, "Log.lib")

#ifndef _UNICODE
#define SetLogFile SetLogFileA
#define LogPrint LogPrintA
#else
#define SetLogFile SetLogFileW
#define LogPrint LogPrintW
#endif

#ifdef __cplusplus  
extern "C" {  
#endif  

BOOL WINAPI SetLogFileA(CHAR* szFile);
BOOL WINAPI SetLogFileW(WCHAR* szFile);

VOID LogPrintA(LPCSTR lpFormat, ...);
VOID LogPrintW(LPCWSTR lpFormat, ...);

VOID WINAPI LogOn();
VOID WINAPI LogOff();
BOOL WINAPI IsLogOpen();
VOID WINAPI LogClose();   // πÿ±’∂‘œÛ


#ifdef __cplusplus
};
#endif

#endif  // _LOG_INTERFACE_H_LGJ_2015_03_04_