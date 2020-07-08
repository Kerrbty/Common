#pragma once
#ifndef __HHCHINA_MNAME_MEMOEY_FINDER_
#define  __HHCHINA_MNAME_MEMOEY_FINDER_


#include "commondef.h"

#define PAGE_SIZE 0x1000

// 从指定内存查找指定的数据块
LPVOID FromAddFindValue(
						LPVOID StartAddress /* 开始查找的地址 */ , 
						DWORD VirtualAddSize /* 查找数据buf大小 */ , 
						LPVOID FindedBuf /* 查找数据 */ ,
						DWORD BufSize /* 数据大小 */ );

// 回调函数
typedef BOOL (* PFINDED_BUF)(LPVOID BufAddress);

// 内存中查找数据 返回数量
DWORD FindBufAndCallBack(HANDLE RemotePID, 
						 LPVOID buf, 
						 DWORD BufSize, 
						 PFINDED_BUF FunCallBack, 
						 DWORD StartAddress = 0x400000, 
						 DWORD EndAddress = 0x7FFFFFFF);

typedef BOOL (* READINFO)(DWORD PID);
// 用名字查找PID
DWORD processNameToId(LPCTSTR lpszProcessName, READINFO CallBackFunc);


#endif // __HHCHINA_MNAME_MEMOEY_FINDER_