#include "HttpDownload.h"
#include "../Http/Http.h"
#include "../StringFormat/StringForm.h"
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#define TMPDOWNLOADFILE   L".dltp"
#define CFGDOWNLOADFILE   L".dltp.cfg"


// 位图数量计算 
const int BitCountTable[] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8, 
};


bool CDownLoad::InitDownFile(const WCHAR* szUrl, const WCHAR* szSaveFile)
{
    // 找文件是否已经存在 
    bool inisuccess = false;
    int nTmp = 0; 
    m_lpFileName = (LPWSTR)AllocMemory((wcslen(szSaveFile)+MAX_PATH)*sizeof(WCHAR));
    wcscpy(m_lpFileName, szSaveFile);
    LPCWSTR lpExt = PathFindExtensionW(szSaveFile);
    DWORD FileNameLenth = lpExt-szSaveFile;

    while ( PathFileExistsW(m_lpFileName) )
    {
        nTmp++;
        wsprintfW(m_lpFileName+FileNameLenth, L"(%u)%s", nTmp, lpExt);
    }

    // 没有url，但可以从文件中查找 
    DWORD lpNameLen = wcslen(m_lpFileName);
    do 
    {
        wcscpy(m_lpFileName+lpNameLen, TMPDOWNLOADFILE); 
        if ( !PathFileExistsW(m_lpFileName) )
        {
            break;
        }

        // 找到的文件打开失败
        m_hfile = FileOpenW(m_lpFileName, GENERIC_WRITE, FILE_SHARE_READ, OPEN_EXISTING);
        if (m_hfile == INVALID_HANDLE_VALUE)
        {
            break;
        }

        // 已经下过了，所以直接找 .dltp.cfg 即可 
        wcscpy(m_lpFileName+lpNameLen, CFGDOWNLOADFILE); 
        if ( !PathFileExistsW(m_lpFileName) )
        {
            break;
        }

        // 嗯，没有被破坏 
        m_hbitmap = FileOpenW(m_lpFileName, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, OPEN_EXISTING);
        if (m_hbitmap == INVALID_HANDLE_VALUE)
        {
            break;
        }

        // 很小的配置文件 
        DWORD dwFileSize = GetFileSize(m_hbitmap, NULL);
        if (dwFileSize < sizeof(BitMap_Header))
        {
            DeleteHandle(m_hbitmap);
            break;
        }

        // 读取文件并校验标志位 
        DWORD dwBytes = 0;
        m_pmap = (PBitMap_Header)AllocMemory(dwFileSize);
        ReadFile(m_hbitmap, m_pmap, dwFileSize, &dwBytes, NULL);
        if (memcmp(m_pmap->MAGIC, DLTPMAGIC, sizeof(DLTPMAGIC)) != 0)
        { 
            FreeMemory(m_pmap);
            DeleteHandle(m_hbitmap);
            break;
        }

        // 所有步骤都成功通过 
        m_lpFileName[lpNameLen] = L'\0';
        m_BitPointer = (LPBYTE)m_pmap + sizeof(BitMap_Header)+m_pmap->url[0].urllen*sizeof(WCHAR);
        inisuccess = true;
    } while (FALSE);

    if (m_hbitmap == INVALID_HANDLE_VALUE && szUrl != NULL)
    {
        do 
        {
            // URL 存在 
            // 文件不存在或者文件被占用 
            if (m_hfile == INVALID_HANDLE_VALUE)
            {
                wcscpy(m_lpFileName+lpNameLen, TMPDOWNLOADFILE); 
                m_hfile = FileOpenW(m_lpFileName, GENERIC_WRITE, FILE_SHARE_READ, CREATE_ALWAYS);
                if (m_hfile == INVALID_HANDLE_VALUE)
                {
                    break;
                }
            }

            // 新建配置文件 
            wcscpy(m_lpFileName+lpNameLen, CFGDOWNLOADFILE); 
            m_hbitmap = FileOpenW(m_lpFileName, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, CREATE_ALWAYS);
            if (m_hbitmap == INVALID_HANDLE_VALUE)
            {
                break;
            }

            m_lpFileName[lpNameLen] = L'\0';
            m_pmap = (PBitMap_Header)AllocMemory(sizeof(BitMap_Header)+wcslen(szUrl)*sizeof(WCHAR));
            memcpy(m_pmap->MAGIC, DLTPMAGIC, sizeof(DLTPMAGIC));

            m_pmap->nUrlCount = 1;
            m_pmap->url[0].urllen = wcslen(szUrl);
            wcscpy(m_pmap->url[0].down_url, szUrl);
            inisuccess = true;
        } while (FALSE);
    }

    if (!inisuccess)
    {
        // bitmap 创建失败的时候 
        DeleteHandle(m_hfile);
    }
//     FreeMemory(lpFileName);
    return inisuccess;
}


void CDownLoad::UpdateBitFile()
{
    DWORD dwBytes = 0;
    SetFilePointer(m_hbitmap, 0, NULL, FILE_BEGIN);

    m_bitmap_lock.lock();
    WriteFile(m_hbitmap, m_pmap, sizeof(BitMap_Header)+m_pmap->url[0].urllen*sizeof(WCHAR), &dwBytes, NULL);
    WriteFile(m_hbitmap, m_BitPointer, m_pmap->blockcount/8+1, &dwBytes, NULL);  // 直接多写一字节，无所谓 
    m_bitmap_lock.unlock();

    SetEndOfFile(m_hbitmap);
    FlushFileBuffers(m_hbitmap);
}

void CDownLoad::Initialize(const WCHAR* szUrl, const WCHAR* szSaveFile, int threadcount)
{
    m_nthread = 0;
    m_nCurrentBlock = 0;
    m_CanDownLoad = FALSE;

    m_down_speed = 0;
    m_down_size = 0;
    m_hSpeedThread = NULL;

    m_hThreadList = NULL; 
    
    m_pmap = NULL;
    m_BitPointer = NULL; 
    m_ETagW = NULL;
    m_lpFileName = NULL;

    m_CanMulThread = FALSE;
    m_downSuccess = FALSE;

    m_hfile = INVALID_HANDLE_VALUE;
    m_hbitmap = INVALID_HANDLE_VALUE; 

    // 线程数量只能在 1~29 之间,默认3 
    if (threadcount <= 0 || threadcount > 30)
    {
        threadcount = 3;
    }

    // 保存文件名一定不能为空 
    if (szSaveFile != NULL)
    {
        if ( InitDownFile(szUrl, szSaveFile) )
        {
            CHttp hhttp(szUrl);
            LPCWSTR eTagtt = hhttp.GetETagW();
            if (eTagtt)
            {
                m_ETagW = (LPWSTR)AllocMemory((wcslen(eTagtt)+1)*sizeof(WCHAR));
                wcscpy(m_ETagW, eTagtt);
            }

            if (m_pmap->filesize == 0)
            {
                // 文件大小为0，说明没有下载过 
                m_BitPointer = NULL;
                m_pmap->filedown = 0;
                m_pmap->filesize = _atoi64(hhttp.GetDataLenthA());
                m_pmap->blocksize = BLOCK_SIZE;
                if (m_pmap->filesize%m_pmap->blocksize == 0)
                {
                    m_pmap->blockcount = m_pmap->filesize/m_pmap->blocksize; // 刚好整除没有多余量 
                    m_pmap->lastblocksize = BLOCK_SIZE;
                }
                else
                {
                    m_pmap->blockcount = m_pmap->filesize/m_pmap->blocksize + 1; // 多最后一个块，这个块不是全部由数据 
                    m_pmap->lastblocksize = m_pmap->filesize%m_pmap->blocksize;
                }
            }

            if (m_BitPointer == NULL)
            {
                // 下载文件设置大小哦 
                LARGE_INTEGER laMove;
                LARGE_INTEGER laRet;
                laMove.QuadPart = m_pmap->filesize;
                laRet.QuadPart = 0;
                m_file_lock.lock();
                SetFilePointerEx(m_hfile, laMove, &laRet, FILE_BEGIN);
                SetEndOfFile(m_hfile);
                m_file_lock.unlock();

                m_pmap = (PBitMap_Header)ReAllocMemory(m_pmap, sizeof(BitMap_Header)+m_pmap->url[0].urllen*sizeof(WCHAR)+m_pmap->blockcount/8+1);
                m_BitPointer = (LPBYTE)m_pmap + sizeof(BitMap_Header)+m_pmap->url[0].urllen*sizeof(WCHAR);
//                 // 测试 
//                 m_BitPointer[0] = 0x80;
                UpdateBitFile();
            }

            hhttp.SetUrlW(szUrl);
            hhttp.SetRangeA("bytes=0-10000");
            const char* szcode = hhttp.GetReturnCodeIdA(); // == 206 
            if (szcode != NULL && strcmp(szcode, "206") == 0)
            {
                // 支持分段下载 
                m_nthread = threadcount; 
                m_CanMulThread = TRUE; 
            }
            else
            {
                // 不支持分段下载 
                m_nthread = 1;
            }

            m_hThreadList = (LPHANDLE)AllocMemory(m_nthread*sizeof(HANDLE));
            for (int i=0; i<m_nthread; i++)
            {
                m_hThreadList[i] = NULL;
            }
        }
    }
    
}

void CDownLoad::Release()
{
    if (m_hThreadList)
    {
        if (m_CanDownLoad)
        {
            m_CanDownLoad = FALSE;
            WaitForMultipleObjects(m_nthread, m_hThreadList, TRUE, INFINITE);
        }
        
        for (int i=0; i<m_nthread; i++)
        {
            CloseHandle(m_hThreadList[i]);
            m_hThreadList[i] = NULL;
        }
        CloseHandle(m_hSpeedThread);
        m_hSpeedThread = NULL;
        FreeMemory(m_hThreadList);
    }

    if (m_pmap)
    {
        FreeMemory(m_pmap);
    }

    FreeMemory(m_lpFileName);
    FreeMemory(m_ETagW);
    DeleteHandle(m_hfile);
    DeleteHandle(m_hbitmap);
}

CDownLoad::CDownLoad(const char* szUrl, const char* szSaveFile, int threadcount)
{
    if (szUrl != NULL && szSaveFile != NULL)
    {
        LPWSTR lpszUrl = MulToWide(szUrl);
        LPWSTR lpszSaveFile = MulToWide(szSaveFile);
        Initialize(lpszUrl, lpszSaveFile, threadcount);
    }
    else if (szSaveFile != NULL)
    {
        LPWSTR lpszSaveFile = MulToWide(szSaveFile);
        Initialize(NULL, lpszSaveFile, threadcount);
    }
    else
    {
        Initialize(NULL, NULL, 0);
    }
}


CDownLoad::CDownLoad(const WCHAR* szUrl, const WCHAR* szSaveFile, int threadcount)
{
    Initialize(szUrl, szSaveFile, threadcount);
}


// 获取某个位是否为1 
int CDownLoad::GetBit(int bitsite)
{
    if (m_BitPointer == NULL)
    {
        return 0;
    }
    int nBof = bitsite/8;
    int nIof = bitsite%8;
    return ((m_BitPointer[nBof]>>nIof)&1); // 读就不加了 
}

// 设置某个位 
bool CDownLoad::SetBit(int bitsite, int val)
{
    if (m_BitPointer == NULL)
    {
        return false;
    }
    int nBof = bitsite/8;
    int nIof = bitsite%8;

    if (GetBit(bitsite) == val)
    {
        return false;
    }
    m_bitmap_lock.lock();
    if (val)
    {
        m_BitPointer[nBof] = m_BitPointer[nBof]|(0x1<<nIof);
    }
    else
    {
        m_BitPointer[nBof] = m_BitPointer[nBof]&(~(0x1<<nIof));
    }
    m_bitmap_lock.unlock();
    return true;
}

typedef struct _callbakc 
{
    CDownLoad *cdown;
    DWORD nCurBlock;
    DWORD HaveSave;
}callbakc, *pcallbakc;

unsigned long WINAPI CDownLoad::Write(unsigned char* lpBuf, unsigned long dwSize, void* userdata)
{
    LARGE_INTEGER laSet;
    LARGE_INTEGER laRet;
    DWORD dwBytes = 0;
    pcallbakc ud = (pcallbakc)userdata;

    laSet.QuadPart = (ud->nCurBlock * ud->cdown->m_pmap->blocksize) + ud->HaveSave;
    
    // 写入对应位置
    ud->cdown->m_file_lock.lock();
    SetFilePointerEx(ud->cdown->m_hfile, laSet, &laRet, FILE_BEGIN);
    WriteFile(ud->cdown->m_hfile, lpBuf, dwSize, &dwBytes, NULL);
    ud->cdown->m_pmap->filedown += dwSize;
    ud->cdown->m_down_size += dwSize;
    ud->cdown->m_file_lock.unlock();

    // 感觉应该这里去设置位的哦 
    ud->HaveSave += dwSize;
    return dwSize;
}

DWORD WINAPI CDownLoad::DownloadThread(LPVOID lparam)
{
    callbakc back;
    unsigned long long laFrom;
    unsigned long long laTo;
    CDownLoad* cdown = (CDownLoad*)lparam;
    back.cdown = cdown;
    back.HaveSave = 0;
    DWORD dwThreadId = 0;

    DWORD nCurrentDownBlock = 0;
    LPWSTR lpRangeString = (LPWSTR)AllocMemory(MAX_PATH*sizeof(WCHAR));

    cdown->m_file_lock.lock();
    dwThreadId = cdown->m_nCurrentBlock;
    cdown->m_nCurrentBlock++;
    cdown->m_file_lock.unlock();
    nCurrentDownBlock = (cdown->m_pmap->blockcount/cdown->m_nthread)*dwThreadId;

    CHttp OneDown(cdown->m_pmap->url[0].down_url);
    OneDown.SetETagW(cdown->m_ETagW);
    OneDown.SetGetDataFunc(Write, &back);
//     for (int j=0; cdown->m_CanDownLoad && j<3; j++) // 做三次,以防有些失败的 
    {
        for (int i=nCurrentDownBlock; cdown->m_CanDownLoad && i<cdown->m_pmap->blockcount; i++)
        {
            if ( cdown->GetBit(i) == 0 )
            {
                laFrom = i * cdown->m_pmap->blocksize;
                laTo = laFrom + cdown->m_pmap->blocksize - 1;

                // bytes=0-10000
                wsprintfW(lpRangeString, L"bytes=%I64d-%I64d", laFrom, laTo);
                OneDown.SetRangeW(lpRangeString);
                OneDown.SetUrlW(cdown->m_pmap->url[0].down_url);
                const char* szcode = OneDown.GetReturnCodeIdA(); // == 206 
                if (szcode != NULL && strcmp(szcode, "206") == 0)
                {
                    // 写入文件
                    back.HaveSave = 0;
                    back.nCurBlock = i;
                    OneDown.LoopToGetData();
                    if( back.HaveSave >= cdown->m_pmap->blocksize )
                    {
                        cdown->SetBit(i, 1);
                        cdown->UpdateBitFile();
                    }
                }
            }
        }
    } 

    // 老大哥，第一个下载线程要下载完所有数据 
    if (dwThreadId == 0)
    {
        for (int i=0; i<cdown->m_pmap->blockcount; )
        {
            if (cdown->GetBit(i) == 0)
            {
                laFrom = i * cdown->m_pmap->blocksize;
                laTo = laFrom + cdown->m_pmap->blocksize - 1;

                // bytes=0-10000
                wsprintfW(lpRangeString, L"bytes=%I64d-%I64d", laFrom, laTo);
                OneDown.SetRangeW(lpRangeString);
                OneDown.SetUrlW(cdown->m_pmap->url[0].down_url);
                const char* szcode = OneDown.GetReturnCodeIdA(); // == 206 
                if (szcode != NULL && strcmp(szcode, "206") == 0)
                {
                    // 写入文件
                    back.HaveSave = 0;
                    back.nCurBlock = i;
                    OneDown.LoopToGetData();
                    // 是否为最后一个块，如果是最后一个块，可能比BlockSize小 
                    if (i+1 == cdown->m_pmap->blockcount) 
                    {
                        if (back.HaveSave >= cdown->m_pmap->lastblocksize)
                        {
                            cdown->SetBit(i, 1);
                            cdown->UpdateBitFile();
                        }
                    }
                    else
                    {
                        if( back.HaveSave >= cdown->m_pmap->blocksize )
                        {
                            cdown->SetBit(i, 1);
                            cdown->UpdateBitFile();
                        }
                    }
                }
            }
            else
            {
                i++;
            }
        }

        cdown->m_CanDownLoad = FALSE;
        Sleep(1000);
        cdown->SetSuccess();
    }

    FreeMemory(lpRangeString);
    return 0;
}

void CDownLoad::SetSuccess()
{
    // 重命名文件 
    DeleteHandle(m_hfile);
    DeleteHandle(m_hbitmap);
    if (m_lpFileName)
    {
        DWORD dwLen = wcslen(m_lpFileName);
        LPWSTR lpFileName = (LPWSTR)AllocMemory((dwLen+MAX_PATH)*sizeof(WCHAR));
        wcscpy(lpFileName, m_lpFileName);
        wcscpy(lpFileName+dwLen, TMPDOWNLOADFILE);
        DeleteFileW(m_lpFileName);
        MoveFile(lpFileName, m_lpFileName);

        wcscpy(lpFileName+dwLen, CFGDOWNLOADFILE);
        DeleteFileW(lpFileName);

        FreeMemory(lpFileName);
    }

    // 设置成功 
    m_downSuccess = TRUE;
}

int CDownLoad::Run()
{
    if (m_pmap == NULL || m_hfile == INVALID_HANDLE_VALUE || m_hbitmap == INVALID_HANDLE_VALUE)
    {
        return -1;
    }
    if (!m_CanDownLoad)
    {
        m_CanDownLoad = TRUE;
        if (m_hSpeedThread != NULL)
        {
            CloseHandle(m_hSpeedThread);
        }
        m_hSpeedThread = CreateThread(NULL, 0, DownloadCalcSpeed, this, 0, NULL);
        for (int i=0; i<m_nthread; i++)
        {
            if (m_hThreadList[i] != NULL)
            {
                CloseHandle(m_hSpeedThread);
            }
            m_hThreadList[i] = CreateThread(NULL, 0, DownloadThread, this, 0, NULL);
        }
    }    
    return 0;
}

unsigned long long  CDownLoad::DownloadSize()
{
    if ( m_pmap )
    {
        return m_pmap->filedown;
    }
    return 0;
}

unsigned long long  CDownLoad::FileTotalSize()
{
    if ( m_pmap )
    {
        return m_pmap->filesize;
    }
    return 0;
}

DWORD WINAPI CDownLoad::DownloadCalcSpeed(LPVOID lparam)
{
    CDownLoad* cdown = (CDownLoad*)lparam;
    while (cdown->m_CanDownLoad)
    {
        DWORD dStartMs = GetTickCount(); // 毫秒数
        if (cdown->m_down_speed == 0)
        {
            Sleep(1000);
        }
        else
        {
            Sleep(3000);
        }
        DWORD dEndMs = GetTickCount();
        cdown->m_down_speed = ((cdown->m_down_size*1000/(dEndMs-dStartMs))+cdown->m_down_speed*2)/3;
        cdown->m_down_size = 0;
    }

    // 结束下载，速度归零，退出线程 
    cdown->m_down_speed = 0;
    return 0;
}


unsigned long CDownLoad::GetSpeed()
{
    return m_down_speed;
}

int CDownLoad::Pause()
{
    m_CanDownLoad = FALSE;
    return 0;
}


BOOL CDownLoad::IsDownSuccess()
{
    return m_downSuccess;
}

CDownLoad::~CDownLoad()
{
    Release();
}