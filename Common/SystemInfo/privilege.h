#pragma once
#ifndef _HHCHINA_MNAME_DEBUG_PRIVILEGE_HEANDER
#define _HHCHINA_MNAME_DEBUG_PRIVILEGE_HEANDER


#include "commondef.h"

// ��Ȩ -- ��ȡ����Ȩ��
BOOL EnableDebugPrivilege(BOOL bEnable = TRUE) ; 
void RaiseToDebug();

#endif // _HHCHINA_MNAME_DEBUG_PRIVILEGE_HEANDER