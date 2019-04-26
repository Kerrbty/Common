#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <Shlwapi.h>
#include <Setupapi.h>
#include "ComPort/Msports.h"
#include "Debug/Debug.h"
#include "defs.h"
#pragma comment(lib, "Shlwapi")
#pragma comment(lib, "setupapi")

// �޸��豸��������ʾ�����ƣ����޸�Ҳ�У�ֻ���豸�������������豸id��ʵ��ʹ�õĲ�һ�� 
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
		DIGCF_ALLCLASSES);   
	
    if (hDevInfo == INVALID_HANDLE_VALUE)   
    {   
        // Insert error handling here.   
        return ;   
    }   
	
    // ö�ٴ����豸���ҵ�Ҫ�޸ĵ��豸�����ں���ʾ��ʽΪ  "%s (COMXXX)"
	// ǰ��� %s Ϊ SPDRP_DEVICEDESC  ֵ
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
            dbgprint(TEXT("Enum Com Device Error!"));
			continue;
		}
		buffer = (PTSTR)AllocMemory(buffersize+16*sizeof(TCHAR));
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
                // �޸��豸��������ʾ����
                wsprintf(lpname, TEXT("%s)"), szDst);
                SetupDiSetDeviceRegistryProperty(
                    hDevInfo,
                    &DeviceInfoData,
                    SPDRP_FRIENDLYNAME,
                    (PBYTE)buffer,
                    _tcslen(buffer)*sizeof(TCHAR));

                // �޸� "HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Enum\ACPI\PNP0501\2\Device Parameters"   "PortName"   szDst
                HKEY hkey = SetupDiOpenDevRegKey( 
                    hDevInfo, 
                    &DeviceInfoData, 
                    DICS_FLAG_GLOBAL, 
                    0, 
                    DIREG_DEV, 
                    0x00F003F);
                if (hkey == INVALID_HANDLE_VALUE)
                {
					dbgprint(TEXT("To Open Reg HKEY_LOCAL_MACHINE\\SYSTEM\\ControlSet001\\Enum\\ACPI\\?????\\?\\Device Parameters Error!"));
                }
				else
				{
					// ��ʵ���޸�Ҳ���Ե�
					RegSetValueEx(hkey,
						TEXT("PortName"),
						0,
						REG_SZ,
						(LPBYTE)szDst,
						_tcslen(szDst)*sizeof(TCHAR));
					RegCloseKey(hkey);
				}

                // ���ñ�־�����豸����������
                SP_DEVINSTALL_PARAMS  DeviceInstallParams = {0};
                DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
                SetupDiGetDeviceInstallParams(hDevInfo, &DeviceInfoData, &DeviceInstallParams);
                DeviceInstallParams.Flags |= DI_PROPERTIES_CHANGE;
                SetupDiSetDeviceInstallParams(hDevInfo, &DeviceInfoData, &DeviceInstallParams);
                
                // ��������
                SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, hDevInfo, &DeviceInfoData);
                break;
            }
            FreeMemory(buffer); // ÿ��ѭ��������ڴ棬������ReAllocMemory
		}
        
    } 
	FreeMemory(buffer);
    SetupDiDestroyDeviceInfoList(hDevInfo);    
}   

// �޸Ĵ��ں�
BOOL ChgComPort(DWORD dwSrc, DWORD dwDst)
{
    BOOL bRetValue = FALSE;
    TCHAR szSrcPort[16];
    TCHAR szDstPort[16];
    LPTSTR lpTargetPath  = (LPTSTR)AllocMemory(80*sizeof(TCHAR));
    wsprintf(szSrcPort, TEXT("COM%u"), dwSrc);
    wsprintf(szDstPort, TEXT("COM%u"), dwDst);
    
    // �����޸�Ϊ�Ѿ����ڵĴ���
    if( QueryDosDevice(szSrcPort, lpTargetPath, 80) && !QueryDosDevice(szDstPort, lpTargetPath, 80) )
    {
        do 
        {
            wsprintf(szSrcPort, TEXT("\\\\.\\COM%d"), dwSrc);
            // �Ѿ����������̴򿪵Ĵ��ڲ������޸�
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
                dbgprint(TEXT("Open Com Port Error, The Port is be Occupied!"));
                break;
            }
            CloseHandle(handle);
            
            // �޸Ĵ��ں���
            wsprintf(szSrcPort, TEXT("COM%u"), dwSrc);
            wsprintf(szDstPort, TEXT("COM%u"), dwDst);
            if( 
                !DefineDosDevice(DDD_REMOVE_DEFINITION, szSrcPort, NULL) || // ɾ��ԭ��com�ڶ���
                !DefineDosDevice(DDD_RAW_TARGET_PATH, szDstPort, lpTargetPath)   // �����¶���
                )
            {
                dbgprint(TEXT("DefineDosDevice error! %s to %s"), szSrcPort, szDstPort);
                break;
            }
            
            // �޸� "HKLM\Hardware\DeviceMap\SerialComm"  lpTargetPath   dwDst ��ʾ����
            HKEY hKey;
            if( RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("Hardware\\DeviceMap\\SerialComm"), &hKey) == ERROR_SUCCESS)
            {
                DWORD i = 0;
                LPTSTR lpValueName = (LPTSTR)AllocMemory(80*sizeof(TCHAR));
                DWORD dwcValueName  = 80;
                DWORD dwType = 0;
                LPTSTR lpData  = (LPTSTR)AllocMemory(80*sizeof(TCHAR));
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
						dbgprint(TEXT("Change SerialComm Show Name!"));
                        break;
                    }
					i++;
					DWORD dwcValueName  = 80;
					DWORD dwType = 0;
					DWORD lpcbData = 80;
                }
                RegCloseKey(hKey);
                FreeMemory(lpValueName);
                FreeMemory(lpData);
            }
            
            // ���� HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\COM Name Arbiter  "ComDB"
            // ɾ��֮ǰʹ�õ�COM�ţ������COM��
            HCOMDB HComDB;
            ComDBOpen(&HComDB);
            ComDBReleasePort( HComDB, dwSrc );	
            ComDBClaimPort( HComDB, dwDst, TRUE, NULL);
			ComDBClose(HComDB);
            
            // �޸��豸��������ʾ��COM���ƣ��Ա�ͳһ
            // SetupDiGetDeviceRegistryProperty(SPDRP_DEVICEDESC);
            // wsprintf(, TEXT("%s (%s)"), , dwDst);
            // SetupDiSetDeviceRegistryProperty(SPDRP_FRIENDLYNAME);
            ChangeManagerShowName(szSrcPort, szDstPort);
            
            bRetValue = TRUE;
        } while (0);
    }
	FreeMemory(lpTargetPath);
    return bRetValue;
}