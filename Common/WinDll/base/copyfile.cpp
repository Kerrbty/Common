// copyfile.cpp : Defines the entry point for the console application.
//

#include "copyfile.h"
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi")



BOOL WINAPI CopyAllFileA(LPCSTR SrcDir, LPCSTR DstDir)
{
    PWSTR  pUnicodeSrc, pUnicodeDst;
    int    iTextLen;
    BOOL   bsucccess = FALSE;

    if (
        SrcDir == NULL || 
        DstDir == NULL
        )
    {
        return bsucccess;
    }

    // SrcDir转Unicode编码
    iTextLen = MultiByteToWideChar( CP_ACP,
        0,
        (PCHAR)SrcDir,
        -1,
        NULL,
        0 );

    pUnicodeSrc = 
        (PWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (iTextLen+1)*sizeof(WCHAR));

    MultiByteToWideChar( CP_ACP,
        0,
        (PCHAR)SrcDir,
        -1,
        pUnicodeSrc,
        iTextLen );

    // DstDir转Unicode编码
    iTextLen = MultiByteToWideChar( CP_ACP,
        0,
        (PCHAR)DstDir,
        -1,
        NULL,
        0 );

    pUnicodeDst = 
        (PWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (iTextLen+1)*sizeof(WCHAR));

    MultiByteToWideChar( CP_ACP,
        0,
        (PCHAR)DstDir,
        -1,
        pUnicodeDst,
        iTextLen );

    CopyAllFileW(pUnicodeSrc, pUnicodeDst);
    HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, pUnicodeDst);
    HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, pUnicodeSrc);
    return bsucccess;
}

// 替换字符串中的符号的啦! 
LPWSTR ReplaceSymbal(LPWSTR lpbuf, WCHAR chS, WCHAR chD)
{
    if (lpbuf == NULL)
    {
        return NULL;
    }
    size_t ilen = wcslen(lpbuf);
    for (size_t i=0; i<ilen; i++)
    {
        if (lpbuf[i] == chS)
        {
            lpbuf[i] = chD;
        }
    }
    return lpbuf;
}

// 创建多级目录
BOOL CreateAllDirectory(LPCWSTR pPath)
{
    BOOL bret = FALSE;
    PWSTR pOneDir = 
        (PWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (MAX_PATH*2)*sizeof(WCHAR));

    DWORD itab = 0; // 标签吧
    PWSTR pCopy = pOneDir;
    do 
    {
        for (; pPath[itab] != L'\\' && pPath[itab] != L'\0'; itab++)
        {
            *pCopy++ = pPath[itab];
        }
        *pCopy = L'\0';
        if ( !PathFileExistsW(pOneDir) )
        {
            if ( !CreateDirectoryW(pOneDir, NULL) )
            {
                // 创建目录失败
                break;
            }
        }

        if (pPath[itab] == L'\0')
        {
            bret = TRUE;
            break;
        }

        // 下一个目录
        *pCopy++ = L'\\';
        itab++;
    } while (TRUE);

    HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, pOneDir);
    return bret;
}


LPWSTR PathRemoveSlash(LPWSTR pPath)
{
    if (pPath == NULL)
    {
        return NULL;
    }
    DWORD ilast = wcslen(pPath);
    for (DWORD i=ilast-1; i>1; i--)
    {
        if (pPath[i] != L'\\')
        {
            break;
        }
        pPath[i] = L'\0';
    }
    return pPath;
}


BOOL WINAPI CopyAllFileW(LPCWSTR SrcDir, LPCWSTR DstDir)
{
    BOOL bsuccess = FALSE;

    DWORD iSrcLen = wcslen(SrcDir);
    DWORD iDsttLen = wcslen(DstDir);
    if (
        iSrcLen > MAX_PATH ||
        iDsttLen > MAX_PATH
        )
    {
        // 路径超过windows支持
        return bsuccess;
    }

    // 拷贝一份先
    PWSTR pSrcDirCopy = 
        (PWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (iSrcLen+2)*sizeof(WCHAR));
    wcscpy(pSrcDirCopy, SrcDir);

    
    PWSTR pDstDirCopy = 
        (PWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (iDsttLen+2)*sizeof(WCHAR));
    wcscpy(pDstDirCopy, DstDir);

    // 将 '/' 斜杠换成 '\'
    ReplaceSymbal(pSrcDirCopy, L'/', L'\\');
    ReplaceSymbal(pDstDirCopy, L'/', L'\\');

    // 把目录最后的符号去掉
    PathRemoveSlash(pSrcDirCopy);
    PathRemoveSlash(pDstDirCopy);

    do 
    {
        // 判断源地址,目的地址是否存在 
        if ( !PathFileExistsW(SrcDir) )
        {
            break;
        }

        if ( !PathFileExistsW(DstDir) )
        {
            // 目的地址不存在需要创建 
            CreateAllDirectory(DstDir);
        }

        // 枚举文件递归调用
        PWSTR pSearch = 
            (PWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (MAX_PATH+10)*sizeof(WCHAR));
        wsprintfW(pSearch, L"%s\\*.*", pSrcDirCopy); 

        WIN32_FIND_DATAW FindFileData = {0};
        HANDLE hFind = FindFirstFileW(pSearch, &FindFileData);
        if (INVALID_HANDLE_VALUE == hFind)
        {
            // 查找文件失败
            HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, pSearch);
            break;
        }


        PWSTR pNextSrcDir = 
            (PWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (MAX_PATH*2)*sizeof(WCHAR));
        PWSTR pNextDstDir = 
            (PWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (MAX_PATH*2)*sizeof(WCHAR));
        do 
        {
            if (
                wcscmp(FindFileData.cFileName, L".") == NULL ||
                wcscmp(FindFileData.cFileName, L"..") == NULL 
                )
            {
                continue;
            }
            wsprintfW(pNextSrcDir, L"%s\\%s", pSrcDirCopy, FindFileData.cFileName);
            wsprintfW(pNextDstDir, L"%s\\%s", pDstDirCopy, FindFileData.cFileName);

            // 如果是目录
            if (FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
            {
                if( !CopyAllFileW(pNextSrcDir, pNextDstDir) )
                {
                    // 里面有东西复制失败了,还是不做处理吧
                    ;
                }
            }
            else
            {
                // 处理文件
                CopyFileW(pNextSrcDir, pNextDstDir, FALSE);
                SetFileAttributes(pNextDstDir, FindFileData.dwFileAttributes);
            }

        } while ( FindNextFileW(hFind, &FindFileData) );
        HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, pNextDstDir);
        HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, pNextSrcDir);


        HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, pSearch);
    } while (FALSE);
    
    HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, pDstDirCopy);
    HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, pSrcDirCopy);
    return bsuccess;
}