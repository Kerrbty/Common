#include <defs.h>
#include <myhook/Hook.h>

typedef BOOL (WINAPI* PMSGA)(HWND,const char*,const char*, unsigned int);

PMSGA msgA = MessageBoxA;

VOID WINAPI realMsg(pcontext pctx)
{
    const char** buf = (const char**)&pctx->others[2];
    *buf = "更新，文本内容哦";
}

BOOL WINAPI MyMsgA(HWND hwnd,const char* strText,const char* strTitle, unsigned int type)
{
    return msgA(hwnd, "New String", "Title", type);
}

int _tmain(int argc, _TCHAR* argv[])
{
    // Detours hook 方式(头部hook) 
    HookProc(&(LPVOID&)msgA, MyMsgA);
    MessageBoxA(NULL, "新消息", "标题", MB_OK);
    UnHookProc(&(LPVOID&)msgA, MyMsgA);

    // 任意地址Inline hook 
    LPBYTE lpaddr = (LPBYTE)
        GetProcAddress(LoadLibrary(TEXT("user32.dll")), "MessageBoxA");
    HookMiddleProc(lpaddr, realMsg);
    MessageBoxA(NULL, "新消息", "标题", MB_OK);
    UnHookMiddleProc(lpaddr, realMsg);
    return 0;
}