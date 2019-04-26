#ifndef _PARENT_INFO_HEADER_H_
#define _PARENT_INFO_HEADER_H_
#include "../defs.h"
#ifdef __cplusplus
extern "C" {
#endif

DWORD WINAPI GetParentID(DWORD dwChildPID = 0);  // ��ѯ������ID  
BOOL WINAPI GetParentExeModule(LPTSTR lpBuf, DWORD dwSize, DWORD dwChildPID = 0); // ��ѯ������exeģ������·�� 
BOOL WINAPI IsParentGUI(BOOL* bGui, DWORD dwChildPID = 0); // ��ѯ�������Ƿ���GUI����CUI���ط� 


#ifdef __cplusplus
};
#endif
#endif  // _PARENT_INFO_HEADER_H_