#pragma once
#ifndef __HHCHINA_MNAME_MEMOEY_FINDER_
#define  __HHCHINA_MNAME_MEMOEY_FINDER_


#include "commondef.h"

#define PAGE_SIZE 0x1000

// ��ָ���ڴ����ָ�������ݿ�
LPVOID FromAddFindValue(
						LPVOID StartAddress /* ��ʼ���ҵĵ�ַ */ , 
						DWORD VirtualAddSize /* ��������buf��С */ , 
						LPVOID FindedBuf /* �������� */ ,
						DWORD BufSize /* ���ݴ�С */ );

// �ص�����
typedef BOOL (* PFINDED_BUF)(LPVOID BufAddress);

// �ڴ��в������� ��������
DWORD FindBufAndCallBack(HANDLE RemotePID, 
						 LPVOID buf, 
						 DWORD BufSize, 
						 PFINDED_BUF FunCallBack, 
						 DWORD StartAddress = 0x400000, 
						 DWORD EndAddress = 0x7FFFFFFF);

typedef BOOL (* READINFO)(DWORD PID);
// �����ֲ���PID
DWORD processNameToId(LPCTSTR lpszProcessName, READINFO CallBackFunc);


#endif // __HHCHINA_MNAME_MEMOEY_FINDER_