#ifndef _GET_COMMAND_LINE_FROM_PID_
#define _GET_COMMAND_LINE_FROM_PID_

#include <windows.h>
#include <tchar.h>

BOOL GetCommandLineA(DWORD pid, LPSTR CmdLns, DWORD dwlenth);
BOOL GetCommandLineW(DWORD pid, LPWSTR CmdLns, DWORD dwlenth);



#ifdef _UNICODE
#define GetCommandLine GetCommandLineW
#else 
#define GetCommandLine GetCommandLineA
#endif // _UNICODE




#endif // _GET_COMMAND_LINE_FROM_PID_