#pragma once
#ifndef _HHCHINA_MNAME_DEBUG_PRIVILEGE_HEANDER
#define _HHCHINA_MNAME_DEBUG_PRIVILEGE_HEANDER


#include <windows.h>

// ��Ȩ -- ��ȡ����Ȩ��
BOOL EnableDebugPrivilege(BOOL bEnable = TRUE) ; 
void RaiseToDebug();

//  �õ�����������Ȩ��
BOOL EnableLoadDriverPrivilege(BOOL bEnable);

#endif // _HHCHINA_MNAME_DEBUG_PRIVILEGE_HEANDER