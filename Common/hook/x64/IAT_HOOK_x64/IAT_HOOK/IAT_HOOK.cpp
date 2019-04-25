// IAT_HOOK.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "IatHook.h"

typedef int (WINAPI *pMessageBox)(  HWND hWnd,          // handle to owner window
			   LPCTSTR lpText,     // text in message box
			   LPCTSTR lpCaption,  // message box title
			   UINT uType          // message box style
			   );

int WINAPI MyMsgBox(   HWND hWnd,          // handle to owner window
					   LPCTSTR lpText,     // text in message box
					   LPCTSTR lpCaption,  // message box title
					   UINT uType          // message box style
			   )
{
	pMessageBox MsgBox= (pMessageBox)GetProcAddress(GetModuleHandle(TEXT("user32.dll")), "MessageBoxA");
	return MsgBox(hWnd, lpText, lpCaption, uType);
}

int _tmain(int argc, _TCHAR* argv[])
{
	HMODULE hmodule = GetModuleHandle(NULL);
	CAPIHook::ReplaceIATEntryInOneName("user32.dll", "MessageBoxA", (PROC)MyMsgBox, hmodule);
	MessageBoxA(NULL, "MessageBox", "Title", NULL);
	return 0;
}

