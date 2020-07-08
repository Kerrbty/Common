#include <windows.h>

class CAsyncFile
{
public:
    CAsyncFile();
    CAsyncFile(LPTSTR szFileName, __int64 laFileSize);
    BOOL FileCreate(LPTSTR szFileName, __int64 laFileSize);
    DWORD Write(LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LARGE_INTEGER* laOffset);
    DWORD Read(LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LARGE_INTEGER* laOffset);
    ~CAsyncFile();

protected:
    void CloseFile();

private:
    HANDLE m_fHandle;
    LARGE_INTEGER m_laFileSize;
};