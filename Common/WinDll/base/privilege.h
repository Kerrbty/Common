#pragma once
#ifndef _HHCHINA_MNAME_DEBUG_PRIVILEGE_HEANDER
#define _HHCHINA_MNAME_DEBUG_PRIVILEGE_HEANDER


#include <windows.h>

// 提权 -- 获取调试权限
BOOL EnableDebugPrivilege(BOOL bEnable = TRUE) ; 
void RaiseToDebug();

//  得到加载驱动的权限
BOOL EnableLoadDriverPrivilege(BOOL bEnable);

#endif // _HHCHINA_MNAME_DEBUG_PRIVILEGE_HEANDER