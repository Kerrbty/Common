#ifndef _PARENT_INFO_HEADER_H_
#define _PARENT_INFO_HEADER_H_
#include "../defs.h"
#ifdef __cplusplus
extern "C" {
#endif

DWORD WINAPI GetParentID(DWORD dwChildPID = 0);  // 查询父进程ID  
BOOL WINAPI GetParentExeModule(LPTSTR lpBuf, DWORD dwSize, DWORD dwChildPID = 0); // 查询父进程exe模块完整路径 
BOOL WINAPI IsParentGUI(BOOL* bGui, DWORD dwChildPID = 0); // 查询父进程是否是GUI程序，CUI返回否 


#ifdef __cplusplus
};
#endif
#endif  // _PARENT_INFO_HEADER_H_