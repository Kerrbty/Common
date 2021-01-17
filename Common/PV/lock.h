#ifndef _LOCK_HEADER_FILE_H_
#define _LOCK_HEADER_FILE_H_
#include <Windows.h>
#include <tchar.h>

class CLock
{
public:
    CLock();

    ~CLock();

    void lock();

    void unlock();
private:
    CRITICAL_SECTION m_cs;

};

#ifdef _DEBUG
#pragma comment(lib, "PV_d.lib")
#else
#pragma comment(lib, "PV.lib")
#endif

#endif // _LOCK_HEADER_FILE_H_