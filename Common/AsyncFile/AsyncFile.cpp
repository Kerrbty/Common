#include "AsyncFile.h"

#if _MSC_VER < 1400 
extern "C" BOOL WINAPI SetFilePointerEx(
                                        HANDLE hFile,                    // handle to file
                                        LARGE_INTEGER liDistanceToMove,  // bytes to move pointer
                                        PLARGE_INTEGER lpNewFilePointer, // new file pointer
                                        DWORD dwMoveMethod               // starting point
                                          );
#endif


CAsyncFile::CAsyncFile()
{
    m_fHandle = INVALID_HANDLE_VALUE;
    m_laFileSize.HighPart = 0;
    m_laFileSize.LowPart = 0;
}

CAsyncFile::CAsyncFile(LPTSTR szFileName, __int64 laFileSize)
{
    m_laFileSize.LowPart = 0;
    m_laFileSize.HighPart = 0;

    FileCreate(szFileName, laFileSize);
}

BOOL CAsyncFile::FileCreate(LPTSTR szFileName, __int64 laFileSize)
{
    BOOL bRetVal = FALSE;
    m_fHandle = CreateFile(
        szFileName, 
        GENERIC_READ|GENERIC_WRITE, 
        FILE_SHARE_READ, 
        NULL, 
        CREATE_ALWAYS, 
        FILE_FLAG_OVERLAPPED, 
        NULL);
    if (m_fHandle != INVALID_HANDLE_VALUE)
    {
        m_laFileSize.QuadPart = laFileSize;
        SetFilePointerEx(m_fHandle, m_laFileSize, NULL, FILE_BEGIN);
        SetEndOfFile(m_fHandle);
        bRetVal = TRUE;
    }
    return bRetVal;
}

DWORD CAsyncFile::Write(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LARGE_INTEGER* laOffset)
{
    DWORD dwBytes = 0;
    OVERLAPPED ol = {0};
    ol.hEvent = CreateEvent(NULL, FALSE, FALSE,NULL);
    ol.Offset = laOffset->LowPart;
    ol.OffsetHigh = laOffset->HighPart;
    
    BOOL wf = WriteFile(m_fHandle, lpBuffer, nNumberOfBytesToWrite, &dwBytes, &ol);
    if (wf == FALSE && GetLastError() == ERROR_IO_PENDING)
    {
        WaitForSingleObject(ol.hEvent, INFINITE);
    }    
    CloseHandle(ol.hEvent);
    return dwBytes;
}

DWORD CAsyncFile::Read(LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LARGE_INTEGER* laOffset)
{
    DWORD dwBytes = 0;
    OVERLAPPED ol = {0};
    ol.hEvent = CreateEvent(NULL, FALSE, FALSE,NULL);
    ol.Offset = laOffset->LowPart;
    ol.OffsetHigh = laOffset->HighPart;
    
    BOOL wf = ReadFile(m_fHandle, lpBuffer, nNumberOfBytesToRead, &dwBytes, &ol);
    if (wf == FALSE && GetLastError() == ERROR_IO_PENDING)
    {
        WaitForSingleObject(ol.hEvent, INFINITE);
    }    
    CloseHandle(ol.hEvent);
    return dwBytes;
}

CAsyncFile::~CAsyncFile()
{
    CloseFile();
}

void CAsyncFile::CloseFile()
{
    if (INVALID_HANDLE_VALUE != m_fHandle)
    {
        CloseHandle(m_fHandle);
    }
}
    
