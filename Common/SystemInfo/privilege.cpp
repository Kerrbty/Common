#include "privilege.h"

BOOL EnableDebugPrivilege(BOOL bEnable) 
{ 

	BOOL fOK = FALSE;
	HANDLE hToken;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) //�򿪽��̷�������
	{ 
		//��ͼ�޸ġ����ԡ���Ȩ
		TOKEN_PRIVILEGES tp;
		tp.PrivilegeCount = 1;
		LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid);
		tp.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;
		AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
		fOK = (GetLastError() == ERROR_SUCCESS);
		CloseHandle(hToken); 
	} 
	return fOK; 
}


void RaiseToDebug()
{
	HANDLE hToken;
	HANDLE hProcess = GetCurrentProcess();  // ��ȡ��ǰ���̾��

	// �򿪵�ǰ���̵�Token������һ��Ȩ�����ƣ��ڶ�������������TOKEN_ALL_ACCESS
	if (OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		TOKEN_PRIVILEGES tkp;
		if (LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tkp.Privileges[0].Luid))
		{
			tkp.PrivilegeCount = 1;
			tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

			//֪ͨϵͳ�޸Ľ���Ȩ��
			BOOL bREt = AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, NULL, 0);
		}
		CloseHandle(hToken);
	}   
}