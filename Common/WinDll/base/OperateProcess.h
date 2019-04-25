#pragma once
#include <WinSock2.h>

enum TcpOrUdp
{
    TcpType,
    UdpType,
};

class CProcOperate
{
public:
    CProcOperate();
    ~CProcOperate();

    BOOL KillProcess(DWORD pid);
    LPTSTR GetFileNameByPid(DWORD pid, LPTSTR szProcName, DWORD dwNameLenth);
    DWORD GetProcessIdByPort(TcpOrUdp type, DWORD dwPort);

protected:
    bool EnableDebugPrivilege();
private:
};