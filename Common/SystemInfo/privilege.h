#pragma once
#ifndef _HHCHINA_MNAME_DEBUG_PRIVILEGE_HEANDER
#define _HHCHINA_MNAME_DEBUG_PRIVILEGE_HEANDER


#include "commondef.h"

// 提权 -- 获取调试权限
BOOL EnableDebugPrivilege(BOOL bEnable = TRUE) ; 
void RaiseToDebug();

#endif // _HHCHINA_MNAME_DEBUG_PRIVILEGE_HEANDER