/*
 * PROJECT:     ReactOS system libraries
 * LICENSE:     GPL - See COPYING in the top level directory
 * FILE:        dlls\win32\msports\classinst.c
 * PURPOSE:     Ports class installer
 * PROGRAMMERS: Copyright 2011 Eric Kohl
 */

#include "Msports.h"
#include <tchar.h>
#include <stdarg.h>
#include <winreg.h>
#include <winuser.h>
#include <wchar.h>
#include <setupapi.h>
#include <stdio.h>
#include <string.h>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi")
#pragma comment(lib, "setupapi")



// WINE_DEFAULT_DEBUG_CHANNEL(msports);

void TRACE(LPTSTR format, ...)
{
    va_list vl;
    va_start(vl, format);
    LPTSTR buf = (LPTSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 1024*sizeof(TCHAR));
    _vstprintf(buf, format, vl);
    OutputDebugString(buf);
    printf("%d\n", buf);
    va_end(vl);
    HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, buf);
}

typedef enum _PORT_TYPE
{
    UnknownPort,
    ParallelPort,
    SerialPort
} PORT_TYPE;

LPTSTR pszCom = TEXT("COM");
LPTSTR pszLpt = TEXT("LPT");

BOOL
GetBootResourceList(HDEVINFO DeviceInfoSet,
                    PSP_DEVINFO_DATA DeviceInfoData,
                    PCM_RESOURCE_LIST *ppResourceList)
{
    HKEY hDeviceKey = NULL;
    HKEY hConfigKey = NULL;
    LPBYTE lpBuffer = NULL;
    DWORD dwDataSize;
    LONG lError;
    BOOL ret = FALSE;

    *ppResourceList = NULL;

    hDeviceKey = SetupDiCreateDevRegKey(DeviceInfoSet,
                                         DeviceInfoData,
                                         DICS_FLAG_GLOBAL,
                                         0,
                                         DIREG_DEV,
                                         NULL,
                                         NULL);
    if (!hDeviceKey)
        return FALSE;

    lError = RegOpenKeyEx(hDeviceKey,
                           TEXT("LogConf"),
                           0,
                           KEY_QUERY_VALUE,
                           &hConfigKey);
    if (lError != ERROR_SUCCESS)
        goto done;

    /* Get the configuration data size */
    lError = RegQueryValueEx(hConfigKey,
                              TEXT("BootConfig"),
                              NULL,
                              NULL,
                              NULL,
                              &dwDataSize);
    if (lError != ERROR_SUCCESS)
        goto done;

    /* Allocate the buffer */
    lpBuffer = (LPBYTE)HeapAlloc(GetProcessHeap(), 0, dwDataSize);
    if (lpBuffer == NULL)
        goto done;

    /* Retrieve the configuration data */
    lError = RegQueryValueEx(hConfigKey,
                              TEXT("BootConfig"),
                              NULL,
                              NULL,
                             (LPBYTE)lpBuffer,
                              &dwDataSize);
    if (lError == ERROR_SUCCESS)
        ret = TRUE;

done:
    if (ret == FALSE && lpBuffer != NULL)
        HeapFree(GetProcessHeap(), 0, lpBuffer);

    if (hConfigKey)
        RegCloseKey(hConfigKey);

    if (hDeviceKey)
        RegCloseKey(hDeviceKey);

    if (ret == TRUE)
        *ppResourceList = (PCM_RESOURCE_LIST)lpBuffer;

    return ret;
}


DWORD
GetSerialPortNumber(IN HDEVINFO DeviceInfoSet,
                    IN PSP_DEVINFO_DATA DeviceInfoData)
{
    PCM_RESOURCE_LIST lpResourceList = NULL;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR lpResDes;
    ULONG i;
    DWORD dwBaseAddress = 0;
    DWORD dwPortNumber = 0;

    TRACE("GetSerialPortNumber(%p, %p)\n",
          DeviceInfoSet, DeviceInfoData);

    if (GetBootResourceList(DeviceInfoSet,
                            DeviceInfoData,
                            &lpResourceList))
    {
        TRACE("Full resource descriptors: %ul\n", lpResourceList->Count);
        if (lpResourceList->Count > 0)
        {
            TRACE("Partial resource descriptors: %ul\n", lpResourceList->List[0].PartialResourceList.Count);

            for (i = 0; i < lpResourceList->List[0].PartialResourceList.Count; i++)
            {
                lpResDes = &lpResourceList->List[0].PartialResourceList.PartialDescriptors[i];
                TRACE("Type: %u\n", lpResDes->Type);

                switch (lpResDes->Type)
                {
                    case CmResourceTypePort:
                        TRACE("Port: Start: %I64x  Length: %lu\n",
                              lpResDes->u.Port.Start.QuadPart,
                              lpResDes->u.Port.Length);
                        if (lpResDes->u.Port.Start.HighPart == 0)
                            dwBaseAddress = (DWORD)lpResDes->u.Port.Start.LowPart;
                        break;

                    case CmResourceTypeInterrupt:
                        TRACE("Interrupt: Level: %lu  Vector: %lu\n",
                              lpResDes->u.Interrupt.Level,
                              lpResDes->u.Interrupt.Vector);
                        break;
                }
            }
        }

        HeapFree(GetProcessHeap(), 0, lpResourceList);
    }

    switch (dwBaseAddress)
    {
        case 0x3f8:
            dwPortNumber = 1;
            break;

        case 0x2f8:
            dwPortNumber = 2;
            break;

        case 0x3e8:
            dwPortNumber = 3;
            break;

        case 0x2e8:
            dwPortNumber = 4;
            break;
    }

    return dwPortNumber;
}


DWORD
GetParallelPortNumber(IN HDEVINFO DeviceInfoSet,
                      IN PSP_DEVINFO_DATA DeviceInfoData)
{
    PCM_RESOURCE_LIST lpResourceList = NULL;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR lpResDes;
    ULONG i;
    DWORD dwBaseAddress = 0;
    DWORD dwPortNumber = 0;

    TRACE("GetParallelPortNumber(%p, %p)\n",
          DeviceInfoSet, DeviceInfoData);

    if (GetBootResourceList(DeviceInfoSet,
                            DeviceInfoData,
                            &lpResourceList))
    {
        TRACE("Full resource descriptors: %ul\n", lpResourceList->Count);
        if (lpResourceList->Count > 0)
        {
            TRACE("Partial resource descriptors: %ul\n", lpResourceList->List[0].PartialResourceList.Count);

            for (i = 0; i < lpResourceList->List[0].PartialResourceList.Count; i++)
            {
                lpResDes = &lpResourceList->List[0].PartialResourceList.PartialDescriptors[i];
                TRACE("Type: %u\n", lpResDes->Type);

                switch (lpResDes->Type)
                {
                    case CmResourceTypePort:
                        TRACE("Port: Start: %I64x  Length: %lu\n",
                              lpResDes->u.Port.Start.QuadPart,
                              lpResDes->u.Port.Length);
                        if (lpResDes->u.Port.Start.HighPart == 0)
                            dwBaseAddress = (DWORD)lpResDes->u.Port.Start.LowPart;
                        break;

                    case CmResourceTypeInterrupt:
                        TRACE("Interrupt: Level: %lu  Vector: %lu\n",
                              lpResDes->u.Interrupt.Level,
                              lpResDes->u.Interrupt.Vector);
                        break;
                }

            }

        }

        HeapFree(GetProcessHeap(), 0, lpResourceList);
    }

    switch (dwBaseAddress)
    {
        case 0x378:
            dwPortNumber = 1;
            break;

        case 0x278:
            dwPortNumber = 2;
            break;
    }

    return dwPortNumber;
}


static DWORD
InstallSerialPort(IN HDEVINFO DeviceInfoSet,
                  IN PSP_DEVINFO_DATA DeviceInfoData)
{
    TCHAR szDeviceDescription[256];
    TCHAR szFriendlyName[256];
    TCHAR szPortName[8];
    DWORD dwPortNumber = 0;
    DWORD dwSize;
    HCOMDB hComDB = HCOMDB_INVALID_HANDLE_VALUE;
    HKEY hKey;
    LONG lError;

    TRACE("InstallSerialPort(%p, %p)\n",
          DeviceInfoSet, DeviceInfoData);

    /* Open the com port database */
    ComDBOpen(&hComDB);

    /* Try to read the 'PortName' value and determine the port number */
    hKey = SetupDiCreateDevRegKey(DeviceInfoSet,
                                   DeviceInfoData,
                                   DICS_FLAG_GLOBAL,
                                   0,
                                   DIREG_DEV,
                                   NULL,
                                   NULL);
    if (hKey != INVALID_HANDLE_VALUE)
    {
        dwSize = sizeof(szPortName);
        lError = RegQueryValueEx(hKey,
                                 TEXT("PortName"),
                                 NULL,
                                 NULL,
                                 (PBYTE)szPortName,
                                 &dwSize);
        if (lError  == ERROR_SUCCESS)
        {
            if (_tcsnicmp(szPortName, pszCom, _tcslen(pszCom)) == 0)
            {
#ifdef _UNICODE
                dwPortNumber = _wtoi(szPortName + _tcslen(pszCom));
#else
                dwPortNumber = atoi(szPortName+_tcslen(pszCom));
#endif
                TRACE("COM port number found: %lu\n", dwPortNumber);
            }
        }

        RegCloseKey(hKey);
    }

    /* Determine the port number from its resources ... */
    if (dwPortNumber == 0)
        dwPortNumber = GetSerialPortNumber(DeviceInfoSet,
                                           DeviceInfoData);

    if (dwPortNumber != 0)
    {
        /* ... and claim the port number in the database */
        ComDBClaimPort(hComDB,
                       dwPortNumber,
                       FALSE,
                       NULL);
    }
    else
    {
        /* ... or claim the next free port number */
        ComDBClaimNextFreePort(hComDB,
                               &dwPortNumber);
    }

    /* Build the name of the port device */
    wsprintf(szPortName, TEXT("%s%u"), pszCom, dwPortNumber);

    /* Close the com port database */
    if (hComDB != HCOMDB_INVALID_HANDLE_VALUE)
        ComDBClose(hComDB);

    /* Set the 'PortName' value */
    hKey = SetupDiCreateDevRegKey(DeviceInfoSet,
                                   DeviceInfoData,
                                   DICS_FLAG_GLOBAL,
                                   0,
                                   DIREG_DEV,
                                   NULL,
                                   NULL);
    if (hKey != INVALID_HANDLE_VALUE)
    {
        RegSetValueEx(hKey,
                       TEXT("PortName"),
                       0,
                       REG_SZ,
                       (LPBYTE)szPortName,
                       (_tcslen(szPortName) + 1) * sizeof(TCHAR));

        RegCloseKey(hKey);
    }

    /* Install the device */
    if (!SetupDiInstallDevice(DeviceInfoSet,
                              DeviceInfoData))
    {
        return GetLastError();
    }

    /* Get the device description... */
    if (SetupDiGetDeviceRegistryPropertyW(DeviceInfoSet,
                                          DeviceInfoData,
                                          SPDRP_DEVICEDESC,
                                          NULL,
                                          (LPBYTE)szDeviceDescription,
                                          256 * sizeof(WCHAR),
                                          NULL))
    {
        /* ... and use it to build a new friendly name */
        wsprintf(szFriendlyName,
                 TEXT("%s (%s)"),
                 szDeviceDescription,
                 szPortName);
    }
    else
    {
        /* ... or build a generic friendly name */
        wsprintf(szFriendlyName,
                 TEXT("Serial Port (%s)"),
                 szPortName);
    }

    /* Set the friendly name for the device */
    SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                      DeviceInfoData,
                                      SPDRP_FRIENDLYNAME,
                                      (LPBYTE)szFriendlyName,
                                      (_tcslen(szFriendlyName) + 1) * sizeof(TCHAR));

    return ERROR_SUCCESS;
}


static DWORD
InstallParallelPort(IN HDEVINFO DeviceInfoSet,
                    IN PSP_DEVINFO_DATA DeviceInfoData)
{
    TCHAR szDeviceDescription[256];
    TCHAR szFriendlyName[256];
    TCHAR szPortName[8];
    DWORD dwPortNumber = 0;
    DWORD dwSize;
    LONG lError;
    HKEY hKey;

    TRACE("InstallParallelPort(%p, %p)\n",
          DeviceInfoSet, DeviceInfoData);

    /* Try to read the 'PortName' value and determine the port number */
    hKey = SetupDiCreateDevRegKey(DeviceInfoSet,
                                   DeviceInfoData,
                                   DICS_FLAG_GLOBAL,
                                   0,
                                   DIREG_DEV,
                                   NULL,
                                   NULL);
    if (hKey != INVALID_HANDLE_VALUE)
    {
        dwSize = sizeof(szPortName);
        lError = RegQueryValueEx(hKey,
                                 TEXT("PortName"),
                                 NULL,
                                 NULL,
                                 (PBYTE)szPortName,
                                 &dwSize);
        if (lError  == ERROR_SUCCESS)
        {
            if (_tcsnicmp(szPortName, pszLpt, _tcslen(pszLpt)) == 0)
            {
#ifdef _UNICODE
                dwPortNumber = _wtoi(szPortName + _tcslen(pszLpt));
#else
                dwPortNumber = atoi(szPortName + _tcslen(pszLpt));
#endif
                TRACE("LPT port number found: %lu\n", dwPortNumber);
            }
        }

        RegCloseKey(hKey);
    }

    /* ... try to determine the port number from its resources */
    if (dwPortNumber == 0)
        dwPortNumber = GetParallelPortNumber(DeviceInfoSet,
                                             DeviceInfoData);

    if (dwPortNumber == 0)
    {
        /* FIXME */
    }

    if (dwPortNumber != 0)
    {
        wsprintf(szPortName, TEXT("%s%u"), pszLpt, dwPortNumber);
    }
    else
    {
        _tcscpy(szPortName, TEXT("LPTx"));
    }

    if (dwPortNumber != 0)
    {
        /* Set the 'PortName' value */
        hKey = SetupDiCreateDevRegKeyW(DeviceInfoSet,
                                       DeviceInfoData,
                                       DICS_FLAG_GLOBAL,
                                       0,
                                       DIREG_DEV,
                                       NULL,
                                       NULL);
        if (hKey != INVALID_HANDLE_VALUE)
        {
            RegSetValueEx(hKey,
                           TEXT("PortName"),
                           0,
                           REG_SZ,
                           (LPBYTE)szPortName,
                           (_tcslen(szPortName) + 1) * sizeof(WCHAR));

            RegCloseKey(hKey);
        }
    }

    /* Install the device */
    if (!SetupDiInstallDevice(DeviceInfoSet,
                              DeviceInfoData))
    {
        return GetLastError();
    }

    /* Get the device description... */
    if (SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                          DeviceInfoData,
                                          SPDRP_DEVICEDESC,
                                          NULL,
                                          (LPBYTE)szDeviceDescription,
                                          256 * sizeof(WCHAR),
                                          NULL))
    {
        /* ... and use it to build a new friendly name */
        wsprintf(szFriendlyName,
                 TEXT("%s (%s)"),
                 szDeviceDescription,
                 szPortName);
    }
    else
    {
        /* ... or build a generic friendly name */
        wsprintf(szFriendlyName,
                 TEXT("Parallel Port (%s)"),
                 szPortName);
    }

    /* Set the friendly name for the device */
    SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                      DeviceInfoData,
                                      SPDRP_FRIENDLYNAME,
                                      (LPBYTE)szFriendlyName,
                                      (_tcslen(szFriendlyName) + 1) * sizeof(WCHAR));

    return ERROR_SUCCESS;
}


VOID
InstallDeviceData(IN HDEVINFO DeviceInfoSet,
                  IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL)
{
    HKEY hKey = NULL;
    HINF hInf = INVALID_HANDLE_VALUE;
    SP_DRVINFO_DATA DriverInfoData;
    PSP_DRVINFO_DETAIL_DATA DriverInfoDetailData;
    TCHAR InfSectionWithExt[256];
    BYTE buffer[2048];
    DWORD dwRequired;

    TRACE("InstallDeviceData()\n");

    hKey = SetupDiCreateDevRegKey(DeviceInfoSet,
                                   DeviceInfoData,
                                   DICS_FLAG_GLOBAL,
                                   0,
                                   DIREG_DRV,
                                   NULL,
                                   NULL);
    if (hKey == NULL)
        goto done;

    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    if (!SetupDiGetSelectedDriver(DeviceInfoSet,
                                   DeviceInfoData,
                                   &DriverInfoData))
    {
        goto done;
    }

    DriverInfoDetailData = (PSP_DRVINFO_DETAIL_DATA)buffer;
    DriverInfoDetailData->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
    if (!SetupDiGetDriverInfoDetail(DeviceInfoSet,
                                     DeviceInfoData,
                                     &DriverInfoData,
                                     DriverInfoDetailData,
                                     2048,
                                     &dwRequired))
    {
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
            goto done;
    }

    TRACE("Inf file name: %S\n", DriverInfoDetailData->InfFileName);

    hInf = SetupOpenInfFile(DriverInfoDetailData->InfFileName,
                             NULL,
                             INF_STYLE_WIN4,
                             NULL);
    if (hInf == INVALID_HANDLE_VALUE)
        goto done;

    TRACE("Section name: %S\n", DriverInfoDetailData->SectionName);

    SetupDiGetActualSectionToInstall(hInf,
                                      DriverInfoDetailData->SectionName,
                                      InfSectionWithExt,
                                      256,
                                      NULL,
                                      NULL);

    TRACE("InfSectionWithExt: %S\n", InfSectionWithExt);

    SetupInstallFromInfSection(NULL,
                                hInf,
                                InfSectionWithExt,
                                SPINST_REGISTRY,
                                hKey,
                                NULL,
                                0,
                                NULL,
                                NULL,
                                NULL,
                                NULL);

    TRACE("Done\n");

done:;
    if (hKey != NULL)
        RegCloseKey(hKey);

    if (hInf != INVALID_HANDLE_VALUE)
        SetupCloseInfFile(hInf);
}



PORT_TYPE
GetPortType(IN HDEVINFO DeviceInfoSet,
            IN PSP_DEVINFO_DATA DeviceInfoData)
{
    HKEY hKey = NULL;
    DWORD dwSize;
    DWORD dwType = 0;
    BYTE bData = 0;
    PORT_TYPE PortType = UnknownPort;
    LONG lError;

    TRACE("GetPortType()\n");

    hKey = SetupDiCreateDevRegKeyW(DeviceInfoSet,
                                   DeviceInfoData,
                                   DICS_FLAG_GLOBAL,
                                   0,
                                   DIREG_DRV,
                                   NULL,
                                   NULL);
    if (hKey == NULL)
    {
        goto done;
    }

    dwSize = sizeof(BYTE);
    lError = RegQueryValueExW(hKey,
                              L"PortSubClass",
                              NULL,
                              &dwType,
                              &bData,
                              &dwSize);

    TRACE("lError: %ld\n", lError);
    TRACE("dwSize: %lu\n", dwSize);
    TRACE("dwType: %lu\n", dwType);

    if (lError == ERROR_SUCCESS &&
        dwSize == sizeof(BYTE) &&
        dwType == REG_BINARY)
    {
        if (bData == 0)
            PortType = ParallelPort;
        else
            PortType = SerialPort;
    }

done:;
    if (hKey != NULL)
        RegCloseKey(hKey);

    TRACE("GetPortType() returns %u \n", PortType);

    return PortType;
}


static DWORD
InstallPort(IN HDEVINFO DeviceInfoSet,
            IN PSP_DEVINFO_DATA DeviceInfoData)
{
    PORT_TYPE PortType;

    InstallDeviceData(DeviceInfoSet, DeviceInfoData);

    PortType = GetPortType(DeviceInfoSet, DeviceInfoData);
    switch (PortType)
    {
        case ParallelPort:
            return InstallParallelPort(DeviceInfoSet, DeviceInfoData);

        case SerialPort:
            return InstallSerialPort(DeviceInfoSet, DeviceInfoData);

        default:
            return ERROR_DI_DO_DEFAULT;
    }
}


static DWORD
RemovePort(IN HDEVINFO DeviceInfoSet,
           IN PSP_DEVINFO_DATA DeviceInfoData)
{
    PORT_TYPE PortType;
    HCOMDB hComDB = HCOMDB_INVALID_HANDLE_VALUE;
    HKEY hKey;
    LONG lError;
    DWORD dwPortNumber;
    DWORD dwPortNameSize;
    TCHAR szPortName[8];

    /* If we are removing a serial port ... */
    PortType = GetPortType(DeviceInfoSet, DeviceInfoData);
    if (PortType == SerialPort)
    {
        /* Open the port database */
        if (ComDBOpen(&hComDB) == ERROR_SUCCESS)
        {
            /* Open the device key */
            hKey = SetupDiOpenDevRegKey(DeviceInfoSet,
                                        DeviceInfoData,
                                        DICS_FLAG_GLOBAL,
                                        0,
                                        DIREG_DEV,
                                        KEY_READ);
            if (hKey != INVALID_HANDLE_VALUE)
            {
                /* Query the port name */
                dwPortNameSize = sizeof(szPortName);
                lError = RegQueryValueEx(hKey,
                                         TEXT("PortName"),
                                         NULL,
                                         NULL,
                                         (PBYTE)szPortName,
                                         &dwPortNameSize);

                /* Close the device key */
                RegCloseKey(hKey);

                /* If we got a valid port name ...*/
                if (lError == ERROR_SUCCESS)
                {
                    /* Get the port number */
#ifdef _UNICODE
                    dwPortNumber = _wtoi(szPortName + _tcslen(pszCom));
#else
                    dwPortNumber = atoi(szPortName + _tcslen(pszCom));
#endif
                    

                    /* Release the port */
                    ComDBReleasePort(hComDB, dwPortNumber);
                }
            }

            /* Close the port database */
            ComDBClose(hComDB);
        }
    }

    /* Remove the device */
    if (!SetupDiRemoveDevice(DeviceInfoSet, DeviceInfoData))
        return GetLastError();

    return ERROR_SUCCESS;
}


DWORD
WINAPI
PortsClassInstaller(IN DI_FUNCTION InstallFunction,
                    IN HDEVINFO DeviceInfoSet,
                    IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL)
{
    TRACE("PortsClassInstaller(%lu, %p, %p)\n",
          InstallFunction, DeviceInfoSet, DeviceInfoData);

    switch (InstallFunction)
    {
        case DIF_INSTALLDEVICE:
            return InstallPort(DeviceInfoSet, DeviceInfoData);

        case DIF_REMOVE:
            return RemovePort(DeviceInfoSet, DeviceInfoData);

        default:
            TRACE("Install function %u ignored\n", InstallFunction);
            return ERROR_DI_DO_DEFAULT;
    }
}

/* EOF */




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
	
    // Enumerate through all devices in Set.  
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

                // 修改 HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Enum\ACPI\PNP0501\2\Device Parameters  "PortName"  dwDst
                HKEY hkey = SetupDiOpenDevRegKey( 
                    hDevInfo, 
                    &DeviceInfoData, 
                    DICS_FLAG_GLOBAL, 
                    0, 
                    DIREG_DEV, 
                    0x00F003F);
                if (hkey == INVALID_HANDLE_VALUE)
                {
                    GetLastError();
                    break;
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
    
    // 不能修改为已近存在的串口
    if( QueryDosDevice(szSrcPort, lpTargetPath, 80) && !QueryDosDevice(szDstPort, lpTargetPath, 80) )
    {
        do 
        {
            wsprintf(szSrcPort, TEXT("\\\\.\\COM%d"), dwSrc);
            // 已经被其他进程打开的串口允许修改
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
            
            // 修改串口名称了
            wsprintf(szSrcPort, TEXT("COM%u"), dwSrc);
            wsprintf(szDstPort, TEXT("COM%u"), dwDst);
            DefineDosDevice(DDD_REMOVE_DEFINITION, szSrcPort, NULL); // 删除原来com口定义
            DefineDosDevice(DDD_RAW_TARGET_PATH, szDstPort, lpTargetPath);   // 加入新定义
            
            // 修改 "HKLM\Hardware\DeviceMap\SerialComm"  lpTargetPath   dwDst
            // RegEnumValue  //  _tcscmp( , dwSrc);
            // RegSetValueEx
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
			ComDBClose(HComDB);
            
//             // 修改设备管理器显示的COM名称，以便统一
//             SetupDiGetDeviceRegistryProperty(SPDRP_DEVICEDESC);
//             wsprintf(, TEXT("%s (%s)"), , dwDst);
//             SetupDiSetDeviceRegistryProperty(SPDRP_FRIENDLYNAME);
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

    char Port[18] = {0};
    wsprintfA(Port, "\\\\.\\COM%s", argv[2]);
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

