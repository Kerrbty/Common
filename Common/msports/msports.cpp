#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <Shlwapi.h>
#include <Setupapi.h>
#include "Msports.h"
#pragma comment(lib, "Shlwapi")
#pragma comment(lib, "setupapi")


// 修改设备管理器显示的名称，不修改也行，只是设备管理器看到的设备id跟实际使用的不一致
VOID ChangeManagerShowName(LPTSTR szSrc, LPTSTR szDst)    
{   
    HDEVINFO hDevInfo;  
	LPTSTR buffer = NULL;
    SP_DEVINFO_DATA DeviceInfoData = {0};   
    DWORD i;   
	
	GUID serial_guid = {0x4D36E978, 0xE325, 0x11CE, {0xBF, 0xC1, 0x08, 0x00, 0x2B, 0xE1, 0x03, 0x18}};
    // Create a HDEVINFO with serial devices.   
    hDevInfo = SetupDiGetClassDevs( &serial_guid,     
		0, // Enumerator   
		0,   
		DIGCF_DEVICEINTERFACE);   
	
    if (hDevInfo == INVALID_HANDLE_VALUE)   
    {   
        // Insert error handling here.   
        return ;   
    }   
	
    // 枚举串口设备，找到要修改的设备，串口号显示形式为  "%s (COMXXX)"
	// 前面的 %s 为 SPDRP_DEVICEDESC  值
    DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);  
    for ( i=0; SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData); i++)   
    {   
        DWORD DataT;  
        DWORD buffersize = 0;   
		
		SetupDiGetDeviceRegistryProperty( 
			hDevInfo,   
			&DeviceInfoData,   
			SPDRP_FRIENDLYNAME , 
			&DataT,   
			NULL,   
			0,   
			&buffersize); 
		if (GetLastError() !=  ERROR_INSUFFICIENT_BUFFER)   
		{   
			continue;
		}
		buffer = (PTSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, buffersize+8*sizeof(TCHAR));
		if(SetupDiGetDeviceRegistryProperty( 
			hDevInfo,   
			&DeviceInfoData,   
			SPDRP_FRIENDLYNAME,   
			&DataT,   
			(PBYTE)buffer,   
			buffersize,   
			&buffersize))
		{
            DWORD dwLen = _tcslen(szSrc);
            LPTSTR lpname = StrStrI(buffer, szSrc);
            if ( lpname != NULL && lpname[dwLen] == TEXT(')') )
            {
                // 修改设备管理器显示名称
                wsprintf(lpname, TEXT("%s)"), szDst);
                SetupDiSetDeviceRegistryProperty(
                    hDevInfo,
                    &DeviceInfoData,
                    SPDRP_FRIENDLYNAME,
                    (PBYTE)buffer,
                    _tcslen(buffer)*sizeof(TCHAR));

                // 修改 "HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Enum\ACPI\PNP0501\1\Device Parameters"   "PortName"   szDst
                HKEY hkey = SetupDiOpenDevRegKey( 
                    hDevInfo, 
                    &DeviceInfoData, 
                    DICS_FLAG_GLOBAL, 
                    0, 
                    DIREG_DEV, 
                    0x00F003F);
                if (hkey != INVALID_HANDLE_VALUE)
                {
                    // 其实不修改也可以滴
					RegSetValueEx(hkey,
						TEXT("PortName"),
						0,
						REG_SZ,
						(LPBYTE)szDst,
						_tcslen(szDst)*sizeof(TCHAR));
                    RegCloseKey(hkey);
                }

                // 设置标志，让设备管理器更新
                SP_DEVINSTALL_PARAMS  DeviceInstallParams = {0};
                DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
                SetupDiGetDeviceInstallParams(hDevInfo, &DeviceInfoData, &DeviceInstallParams);
                DeviceInstallParams.Flags |= DI_PROPERTIES_CHANGE;
                SetupDiSetDeviceInstallParams(hDevInfo, &DeviceInfoData, &DeviceInstallParams);
                
                // 天气不错
                SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, hDevInfo, &DeviceInfoData);
                break;
            }
		}
		HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, buffer);
		buffer = NULL;
    } 
	if (buffer)
	{
		HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, buffer);
	}
    SetupDiDestroyDeviceInfoList(hDevInfo);    
}   

BOOL ChgComPort(DWORD dwSrc, DWORD dwDst)
{
    TCHAR szSrcPort[16];
    TCHAR szDstPort[8];
    LPTSTR lpTargetPath  = new TCHAR[80];
    wsprintf(szSrcPort, TEXT("COM%u"), dwSrc);
    wsprintf(szDstPort, TEXT("COM%u"), dwDst);
    
    // 不能修改为已经存在的串口
    if( QueryDosDevice(szSrcPort, lpTargetPath, 80) && !QueryDosDevice(szDstPort, lpTargetPath, 80) )
    {
        do 
        {
            wsprintf(szSrcPort, TEXT("\\\\.\\COM%d"), dwSrc);
            // 已经被其他进程打开的串口不允许修改
            HANDLE handle = CreateFile(
                szSrcPort,
                GENERIC_READ,
                0,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL);
            if (handle == INVALID_HANDLE_VALUE)
            {
                break;
            }
            CloseHandle(handle);
            
            // 修改串口号了
            wsprintf(szSrcPort, TEXT("COM%u"), dwSrc);
            wsprintf(szDstPort, TEXT("COM%u"), dwDst);
            DefineDosDevice(DDD_REMOVE_DEFINITION, szSrcPort, NULL); // 删除原来com口定义
            DefineDosDevice(DDD_RAW_TARGET_PATH, szDstPort, lpTargetPath);   // 加入新定义
            
            // 修改 "HKLM\Hardware\DeviceMap\SerialComm"  lpTargetPath   dwDst 显示名称
            HKEY hKey;
            if( RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("Hardware\\DeviceMap\\SerialComm"), &hKey) == ERROR_SUCCESS)
            {
                DWORD i = 0;
                LPTSTR lpValueName = new TCHAR[80];
                DWORD dwcValueName  = 80;
                DWORD dwType = 0;
                LPTSTR lpData  = new TCHAR[80];
                DWORD lpcbData = 80;
                while ( RegEnumValue(hKey, i, lpValueName, &dwcValueName, NULL, &dwType, (LPBYTE)lpData, &lpcbData) == ERROR_SUCCESS)
                {
                    if ( _tcsicmp(lpData, szSrcPort) == 0 )
                    {
                        RegSetValueEx(hKey,
                            lpValueName,
                            0,
                            dwType,
                            (LPBYTE)szDstPort,
                            _tcslen(szDstPort)*sizeof(TCHAR));
                        break;
                    }
					i++;
					DWORD dwcValueName  = 80;
					DWORD dwType = 0;
					DWORD lpcbData = 80;
                }
                RegCloseKey(hKey);
				delete []lpValueName;
				delete []lpData;
            }
            
            // 操作 HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\COM Name Arbiter  "ComDB"
            // 删除之前使用的COM号，标记新COM号
            HCOMDB HComDB;
            ComDBOpen(&HComDB);
            ComDBReleasePort( HComDB, dwSrc );	
            ComDBClaimPort( HComDB, dwDst, TRUE, NULL);
            
            // 修改设备管理器显示的COM名称，以便统一
            // SetupDiGetDeviceRegistryProperty(SPDRP_DEVICEDESC);
            // wsprintf(, TEXT("%s (%s)"), , dwDst);
            // SetupDiSetDeviceRegistryProperty(SPDRP_FRIENDLYNAME);
            ChangeManagerShowName(szSrcPort, szDstPort);
            
            return TRUE;
        } while (0);
    }
    return FALSE;
}


int main(int argc, char* argv[])
{
    if (argc < 3 )
    {
        return EXIT_FAILURE;
    }

    ChgComPort(atoi(argv[1]), atoi(argv[2]));

    TCHAR Port[18] = {0};
    wsprintf(Port, "\\\\.\\COM%s", argv[2]);
	HANDLE handle = CreateFile(Port, 
		GENERIC_READ, 
		FILE_SHARE_READ|FILE_SHARE_WRITE, 
		NULL, 
		OPEN_EXISTING, 
		0, 
		NULL);
	if (handle == INVALID_HANDLE_VALUE)
	{
		printf("Open %s error!\n", Port);
	}
	else
	{
		printf("open %s success!\n", Port);
		CloseHandle(handle);
	}

	return 0;
}

