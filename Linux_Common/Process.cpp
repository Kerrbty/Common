#include "Process.h"
#include<unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

// Find File Name in Full path
const char* PathGetFileName(char* szPathFile)
{
        if (szPathFile == NULL)
        {
                return NULL;
        }
        char* szFileName = szPathFile;
        char* p = szFileName;
        while(*p != '\0')
        {
                if( *p == '/' || *p == '\\' )
                {
                        szFileName = p+1;
                }
                else if ( *p =='\r' || *p == '\n' )
                {
                        *p = '\0';
                        break;
                }
                p++;
        }
        return szFileName;
}

unsigned long long GetAddress(const  char* szbuf)
{
        unsigned long long findval = 0;
        for (int i=0; i<8; i++)
        {
                if (szbuf[i] >= '0' && szbuf[i] <= '9')
                {
                        findval = (findval<<4) + szbuf[i] - '0';
                }
                else if ((szbuf[i]|0x20) >= 'a' &&  (szbuf[i]|0x20)<='f')
                {
                        findval = (findval<<4) + (szbuf[i]|0x20) - 'a' +10;
                }
        }
        return findval;
}

// Find Module base Address in mempry
void*  GetModuleHandle(const char* ModuleName)
{
        unsigned char* addr = NULL;
        const char* pName = NULL;
        char* chReadBuf = NULL;
        char* szFindModuleName = (char*)ModuleName;
        if (szFindModuleName == NULL)
        {
                // Get Exe Module Name
                FILE* fp;
                szFindModuleName = (char*)malloc(PATH_MAX*2);
                if (szFindModuleName == NULL)
                {
                        return NULL;
                }

                sprintf(szFindModuleName, "/proc/%u/exe", getpid());
                if( readlink(szFindModuleName, szFindModuleName, PATH_MAX)  == -1 )
                {
                        sprintf(szFindModuleName, "/proc/%u/cmdline",  getpid());
                        fp =  fopen(szFindModuleName, "r");
                        if (fp != NULL)
                        {
                                fscanf(fp, "%s", szFindModuleName);
                                fclose(fp);
                        }
                }
        }

        pName = PathGetFileName(szFindModuleName);
        // printf("FileName: %s\n\n\n", pName);

        chReadBuf = (char*)malloc(PATH_MAX*3);
        if (chReadBuf != NULL)
        {
                FILE* fp;
                sprintf(chReadBuf, "/proc/%u/maps", getpid());
                fp = fopen(chReadBuf, "r");
                if (fp != NULL)
                {
                        // 08048000-0805b000  r-xp   00000000   08:01  679381     /usr/lib/gvfs/gvfs-gphoto2-volume-monitor
                        // 0805d000-0805e000  rw-p  00000000   00:00  0
                        while( fgets(chReadBuf, PATH_MAX*3, fp) != NULL )
                        {
                                if (strstr(chReadBuf, "/") != NULL)
                                {
                                        // printf("%s\n", chReadBuf);
                                        const char* thismodule = PathGetFileName(chReadBuf);
                                        if (strcmp(thismodule, pName) == 0)
                                        {
                                                addr = (unsigned char*)GetAddress(chReadBuf);
                                                // printf("%s\n", chReadBuf);
                                                // printf("find : %s -> %p\n", thismodule, addr);
                                                break;
                                        }
                                }
                        }
                        fclose(fp);
                }
                free(chReadBuf);
        }

        // free memory if alloc
        if(szFindModuleName != ModuleName)
        {
                free(szFindModuleName);
        }
        return addr;
}

// 08048000-0805b000  r-xp   00000000   08:01  679381     /usr/lib/gvfs/gvfs-gphoto2-volume-monitor
 // 0805d000-0805e000  rw-p  00000000   00:00  0
bool IsBadReadPtr(void* addr, unsigned int len)
{
        bool canread = false;
        char* chReadBuf = (char*)malloc(PATH_MAX*3);
        if (chReadBuf != NULL)
        {
                FILE* fp;
                sprintf(chReadBuf, "/proc/%u/maps", getpid());
                fp = fopen(chReadBuf, "r");
                if (fp != NULL)
                {
                         while( fgets(chReadBuf, PATH_MAX*3, fp) != NULL )
                         {
                                const char* pchEnd = strstr(chReadBuf, "-") ;
                                if (pchEnd != NULL)
                                {
                                        unsigned char* pStartAddr = (unsigned char*)GetAddress(chReadBuf);
                                        unsigned char* pEndAddr = (unsigned char*)GetAddress(pchEnd+1);
                                        if ((unsigned char*)addr >= pStartAddr && (unsigned char*)addr+len <= pEndAddr )
                                        {
                                                while(*pchEnd != '\0')
                                                {
                                                        // printf("%02X ", *pchEnd);
                                                        if (*pchEnd == '\x20')
                                                        {
                                                                while(*pchEnd == '\x20')
                                                                {
                                                                        pchEnd++;
                                                                }
                                                                break;
                                                        }
                                                        pchEnd++;
                                                }
                                                printf("[Result] %s\n", pchEnd);
                                                if (*pchEnd == 'r')
                                                {
                                                        canread = true;
                                                }
                                                break;
                                        }
                                        // printf("[debug] %s\n %p -> %p\n", chReadBuf, pStartAddr, pEndAddr);
                                }
                         }
                        fclose(fp);
                }
                free(chReadBuf);
        }
        return canread;
}

bool IsBadWritePtr(void* addr, unsigned int len)
{
        bool canwrite = false;
        char* chReadBuf = (char*)malloc(PATH_MAX*3);
        if (chReadBuf != NULL)
        {
                FILE* fp;
                sprintf(chReadBuf, "/proc/%u/maps", getpid());
                fp = fopen(chReadBuf, "r");
                if (fp != NULL)
                {
                         while( fgets(chReadBuf, PATH_MAX*3, fp) != NULL )
                         {
                                const char* pchEnd = strstr(chReadBuf, "-") ;
                                if (pchEnd != NULL)
                                {
                                        unsigned char* pStartAddr = (unsigned char*)GetAddress(chReadBuf);
                                        unsigned char* pEndAddr = (unsigned char*)GetAddress(pchEnd+1);
                                        if ((unsigned char*)addr >= pStartAddr && (unsigned char*)addr+len <= pEndAddr )
                                        {
                                                while(*pchEnd != '\0')
                                                {
                                                        // printf("%02X ", *pchEnd);
                                                        if (*pchEnd == '\x20')
                                                        {
                                                                while(*pchEnd == '\x20')
                                                                {
                                                                        pchEnd++;
                                                                }
                                                                break;
                                                        }
                                                        pchEnd++;
                                                }
                                                if (*(pchEnd+1) == 'w')
                                                {
                                                        // printf("[debug] \"%s\" %p -> %p\n", pchEnd, pStartAddr, pEndAddr);
                                                        canwrite = true;
                                                }
                                                break;
                                        }
                                }
                         }
                        fclose(fp);
                }
                free(chReadBuf);
        }
        return canwrite;
}

char* GetModuleFileName(void* hModule, char* szFileName, unsigned int len) // readlink();
{
        char* retname = NULL;
        char* chReadBuf = (char*)malloc(PATH_MAX*3);
        if (chReadBuf == NULL)
        {
                return NULL;
        }

        if (hModule == NULL)
        {
                // Process Exe Module
                // From exe link
                FILE* fp;
                sprintf(chReadBuf, "/proc/%u/exe", getpid());
                if( readlink(chReadBuf, szFileName, len)  == -1 )
                {
                        retname = szFileName;
                }
                else
                {
                        // From command line
                        sprintf(chReadBuf, "/proc/%u/cmdline",  getpid());
                        fp =  fopen(chReadBuf, "r");
                        if (fp != NULL)
                        {
                                fscanf(fp, "%s", chReadBuf);
                                fclose(fp);

                                strncpy(szFileName, chReadBuf, len);
                                retname = szFileName;
                        }
                }
        }
        else
        {
                FILE* fp;
                sprintf(chReadBuf, "/proc/%u/maps", getpid());
                fp = fopen(chReadBuf, "r");
                if (fp != NULL)
                {
                        // 08048000-0805b000  r-xp   00000000   08:01  679381     /usr/lib/gvfs/gvfs-gphoto2-volume-monitor
                        // 0805d000-0805e000  rw-p  00000000   00:00  0
                        while( fgets(chReadBuf, PATH_MAX*3, fp) != NULL )
                        {
                                const char* pName = strstr(chReadBuf, "/");
                                const char* pchEnd = strstr(chReadBuf, "-") ;
                                if ( pName != NULL && pchEnd != NULL)
                                {
                                        // printf("%s\n", chReadBuf);
                                        unsigned char* pStartAddr = (unsigned char*)GetAddress(chReadBuf);
                                        unsigned char* pEndAddr = (unsigned char*)GetAddress(pchEnd+1);
                                        if ((unsigned char*)hModule >= pStartAddr && (unsigned char*)hModule <= pEndAddr )
                                        {
                                                strncpy(szFileName, pName, len);
                                                PathGetFileName(szFileName);
                                                retname = szFileName;
                                                // break;
                                        }
                                        // printf("[debug] %p -> %p %s\n", pStartAddr, pEndAddr, pName);
                                }
                        }
                        fclose(fp);
                }
        }
        free(chReadBuf);
        return retname;
}
