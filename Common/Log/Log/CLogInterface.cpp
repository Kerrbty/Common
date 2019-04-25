#include <windows.h>
#include "CLogInterface.h"
#include "APIInterface.h"

enum SWITCH_STATE
{
    OFF = 0,
    ON = 1,
};

CRITICAL_SECTION  CriticalSection ;  // 日志锁

class CLog
{
public:
    CLog();
    CLog(LPCWSTR);
    ~CLog();
    
    SWITCH_STATE open_log();
    SWITCH_STATE close_log();
    
    void log(char* format);
    
    SWITCH_STATE query_log_switch();
protected:
private:
    SWITCH_STATE m_Log_switch; // 日志开关
    HANDLE m_hfile;
    BOOL isConsole;
};

CLog* g_log_object = NULL;

CLog::CLog()
{
    AllocConsole();
    m_hfile = GetStdHandle(STD_OUTPUT_HANDLE);
    m_Log_switch = OFF;
    isConsole = TRUE;
}


CLog::CLog(LPCWSTR lplogfile)
{
    m_hfile = API_CreateFileW(lplogfile, 
        GENERIC_WRITE, 
        FILE_SHARE_READ, 
        NULL, 
        OPEN_ALWAYS, 
        FILE_ATTRIBUTE_NORMAL, 
        NULL);
    if (m_hfile != INVALID_HANDLE_VALUE)
    {
        API_SetFilePointer(m_hfile, 0, NULL, FILE_END);
    }
    m_Log_switch = OFF;
    isConsole = FALSE;
}


CLog::~CLog()
{
    m_Log_switch = OFF;
    if (!isConsole)
    {
        API_CloseHandle(m_hfile);
    }
    
}

SWITCH_STATE CLog::open_log()
{
    SWITCH_STATE ist = m_Log_switch;
    m_Log_switch = ON;
    return ist;
}

SWITCH_STATE CLog::close_log()
{
    SWITCH_STATE ist = m_Log_switch;
    m_Log_switch = OFF;
    return ist;
}


void CLog::log(char* format)
{
    DWORD dwBytes = 0;

    if ( m_Log_switch == OFF )
    {
        return;
    }

    if (!isConsole)
    {
        API_WriteFile(m_hfile, format, strlen(format), &dwBytes, NULL);
    }
    else
    {
        WriteConsole(m_hfile, format, strlen(format), &dwBytes, NULL);
    }
    
}

SWITCH_STATE CLog::query_log_switch()
{
    return m_Log_switch;
}



//////////////////////////////////////////////////////////////////////////
//  
//  接口
// 
//////////////////////////////////////////////////////////////////////////

BOOL WINAPI SetLogFileA(CHAR* szFile)
{
    PWSTR  pElementText;
    int    iTextLen;
    
    iTextLen = MultiByteToWideChar( CP_ACP,
        0,
        (PCHAR)szFile,
        -1,
        NULL,
        0 );
    
    pElementText = 
        (PWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (iTextLen+1)*sizeof(WCHAR));
    
    MultiByteToWideChar( CP_ACP,
        0,
        (PCHAR)szFile,
        -1,
        pElementText,
        iTextLen );
    
    BOOL retval = SetLogFileW( pElementText );
    
    HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, pElementText);
    return retval;
}


BOOL WINAPI SetLogFileW(WCHAR* szFile)
{
    BOOL retval = FALSE;

    EnterCriticalSection(&CriticalSection);
    if (g_log_object != NULL)
    {
        delete g_log_object;
    }
    if (szFile != NULL)
    {
        g_log_object = new CLog(szFile);
    }
    else
    {
        g_log_object = new CLog();
    }
    if (g_log_object != NULL)
    {
        retval = TRUE;
    }
    LeaveCriticalSection(&CriticalSection);
    return retval;
}


VOID _cdecl LogPrintA(LPCSTR lpFormat, ...)
{
    va_list arglist;
    CHAR* wbuf = NULL;
    
    va_start(arglist, lpFormat);
    wbuf = (CHAR*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 2048);
    wvsprintfA(wbuf, lpFormat, arglist);
    va_end(arglist);

    EnterCriticalSection(&CriticalSection);
    if (g_log_object != NULL)
    {
        g_log_object->log(wbuf);
    }
    else
    {
        OutputDebugStringA(wbuf);
    }
    LeaveCriticalSection(&CriticalSection);

    HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, wbuf);
}


VOID _cdecl LogPrintW(LPCWSTR lpFormat, ...)
{
    PSTR  pElementText;
    int    iTextLen;
    va_list arglist;
    WCHAR* wbuf = NULL;

    va_start(arglist, lpFormat);
    wbuf = (WCHAR*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 2048*sizeof(WCHAR));
    wvsprintfW(wbuf, lpFormat, arglist);
    va_end(arglist);
    
    
    iTextLen = WideCharToMultiByte( CP_ACP,
        0,
        wbuf,
        -1,
        NULL,
        0,
        NULL,
        NULL);
    
    pElementText = 
        (PSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, iTextLen+1 );
    
    WideCharToMultiByte( CP_ACP,
        0,
        wbuf,
        -1,
        pElementText,
        iTextLen,
        NULL,
		NULL);
    HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, wbuf);

    EnterCriticalSection(&CriticalSection);
    if (g_log_object != NULL)
    {
        g_log_object->log(pElementText);
    }
    else
    {
        OutputDebugStringA(pElementText);
    }
    LeaveCriticalSection(&CriticalSection);

    HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, pElementText);
}

VOID WINAPI LogOn()
{
    EnterCriticalSection(&CriticalSection);
    if (g_log_object == NULL)
    {
        g_log_object = new CLog();   
    }

    if (g_log_object != NULL)
    {
        g_log_object->open_log();
    }
    LeaveCriticalSection(&CriticalSection);
}

VOID WINAPI LogOff()
{
    EnterCriticalSection(&CriticalSection);
    if (g_log_object != NULL)
    {
        g_log_object->close_log();
    }
    LeaveCriticalSection(&CriticalSection);
}


BOOL WINAPI IsLogOpen()
{
    BOOL retval = FALSE;
    EnterCriticalSection(&CriticalSection);
    if (g_log_object == NULL)
    {
        retval = g_log_object->query_log_switch();
    }
    LeaveCriticalSection(&CriticalSection);
    return retval;
}


VOID WINAPI LogClose()
{
    EnterCriticalSection( &CriticalSection );
    if (g_log_object != NULL)
    {
        delete g_log_object;
        g_log_object = NULL;
    }
    LeaveCriticalSection( &CriticalSection );
}