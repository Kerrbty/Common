#include "MemoryFinder.h"
//#include "StringForm.h"


DWORD processNameToId(LPCTSTR lpszProcessName, READINFO CallBackFunc)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 pe;
	DWORD RetValue = 0;
	pe.dwSize = sizeof(PROCESSENTRY32);
	
	if (!Process32First(hSnapshot, &pe)) 
	{
		return -1;
	}
	
	while (Process32Next(hSnapshot, &pe)) {
		if (!_tcsicmp(lpszProcessName, pe.szExeFile)) {
			if( !CallBackFunc(pe.th32ProcessID) )
			{
				RetValue = pe.th32ProcessID;
				break;
			}
		}
	}
	CloseHandle(hSnapshot);
	return RetValue;
}

LPVOID FromAddFindValue(
						LPVOID StartAddress /* 开始查找的地址 */ , 
						DWORD VirtualAddSize /* 查找数据buf大小 */ , 
						LPVOID FindedBuf /* 查找数据 */ ,
						DWORD BufSize /* 数据大小 */ )
{
	DWORD i = 0;
	for ( i=0; i<=VirtualAddSize-BufSize; i++)
	{
		if ( memcmp((LPBYTE)StartAddress+i, FindedBuf, BufSize) == NULL )
		{
			return (LPBYTE)StartAddress+i;
		}
	}
	return NULL;
}


// 内存中查找数据 返回数量
DWORD FindBufAndCallBack(HANDLE RemotePID, 
						 LPVOID buf, 
						 DWORD BufSize, 
						 PFINDED_BUF FunCallBack, 
						 DWORD StartAddress, 
						 DWORD EndAddress)
{
	PBYTE ReadProcBuf= new BYTE[PAGE_SIZE*2];
	PBYTE pBuf = ReadProcBuf + PAGE_SIZE/2;
	DWORD i = StartAddress & 0xFFFFF000;
	DWORD dwBytes = 0;
	DWORD BufOffset = StartAddress & 0xFFF;
	DWORD TotalCount = 0;
	LPVOID FindAddress;
	BOOL FindNext = TRUE;
	MEMORY_BASIC_INFORMATION MemoryInfo;
	for ( ; i<EndAddress && FindNext; i += PAGE_SIZE )
	{
		if ( VirtualQueryEx(RemotePID, (LPVOID)i, &MemoryInfo, sizeof(MEMORY_BASIC_INFORMATION)) )
		{
			if(MemoryInfo.Protect&0x3)
			{
				continue;
			}
		}
		if (!::ReadProcessMemory(RemotePID, (LPVOID)i, pBuf, PAGE_SIZE, &dwBytes))
		{
			// 此页不可读
			continue;
		}
		// 前后多读半页，以防止数据在边界而产生错误
		::ReadProcessMemory(RemotePID, (LPVOID)(i-PAGE_SIZE/2), ReadProcBuf, PAGE_SIZE/2, &dwBytes);
		::ReadProcessMemory(RemotePID, (LPVOID)(i+PAGE_SIZE), &pBuf[PAGE_SIZE], PAGE_SIZE/2, &dwBytes);
		while ( (	FindAddress = 
			FromAddFindValue(pBuf+BufOffset, PAGE_SIZE-BufOffset, buf, BufSize)
			) != NULL)
		{
			BufOffset = (DWORD)FindAddress - (DWORD)pBuf + BufSize;
			FindNext = FunCallBack(FindAddress);
			TotalCount++;
			if (BufOffset >= PAGE_SIZE-BufSize || FindNext == FALSE)
			{
				break;
			}
		}
		BufOffset = 0;
	}

	delete []ReadProcBuf;
	return TotalCount;
}