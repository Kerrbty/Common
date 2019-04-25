#ifndef _HTTP_DWONLOAD_HEADER_HH_H_
#define _HTTP_DWONLOAD_HEADER_HH_H_
#include "../pv/lock.h"


#define DLTPMAGIC  "DLTP\0\0\0"
#define BLOCK_SIZE  4*1024*1024    // 4M 
#pragma pack(push, 1)

typedef struct _url_str 
{
    unsigned int urllen;     // 下载地址长度   
    WCHAR down_url[1];            // 下载地址列表 
}url_str, *purl_str;

typedef struct _BitMap_Header
{
    char    MAGIC[8];
    unsigned long long filesize;  // 文件大小 
    unsigned long long filedown;  // 已经下载的大小 
    unsigned long  blockcount;     // 文件块个数 
    unsigned long  blocksize;     // 一个块表示文件的大小 
    unsigned long  lastblocksize;  // 最后一个块的数据字节数 
    unsigned long  nUrlCount;     // 可以同时支持多个URL，用于p2p 
    url_str        url[1];        // 下载的url列表 
}BitMap_Header, *PBitMap_Header;

// typedef union _BitFilePoint
// {
//     BYTE lpBuf[1];
//     BitMap_Header bitmap;
// }BitFilePoint, *PBitFilePoint;
#pragma pack(pop)


// xxx.dltp  是下载文件
// xxx.dlp.cfg 是下载过程信息文件

class CDownLoad
{
public:
    CDownLoad(const char* szUrl, const char* szSaveFile, int threadcount = 6);
    CDownLoad(const WCHAR* szUrl, const WCHAR* szSaveFile, int threadcount = 6);
    ~CDownLoad();

    static DWORD WINAPI DownloadThread(LPVOID lparam);
    static DWORD WINAPI DownloadCalcSpeed(LPVOID lparam);
    static unsigned long WINAPI Write(unsigned char* lpBuf, unsigned long dwSize, void* userdata);

    int Run();    // 开始下载，返回0 表示成功，其他表示失败原因 

    unsigned long long  DownloadSize(); // 获取下载大小 
    unsigned long long  FileTotalSize(); // 网络端文件大小
    unsigned long long  DwonloadSpeed(); // 下载速度 

    int Pause();

    BOOL IsDownSuccess();

    unsigned long GetSpeed();
private:
    void Initialize(const WCHAR* szUrl, const WCHAR* szSaveFile, int threadcount);
    void Release();
    void SetSuccess();
    int  GetBit(int bitsite);
    bool SetBit(int bitsite, int val);
    bool InitDownFile(const WCHAR* szUrl, const WCHAR* szSaveFile);
    void UpdateBitFile();   // 实时更新 

private:
    // 这几个变量主要是 DownloadThread 调用 
    int    m_nCurrentBlock;     // 当前第几个线程，计算下载块在哪 
    int    m_nthread;         // 线程个数 
    BOOL   m_CanDownLoad;     // 是否下载（暂停，或程序退出）

    BOOL   m_downSuccess;

    // 计算下载速度用 
    unsigned long long m_down_speed;
    unsigned long long m_down_size;
    HANDLE m_hSpeedThread;

    LPHANDLE m_hThreadList;   // 线程指针 
    BOOL  m_CanMulThread;     // URL是否支持多线程

    PBitMap_Header m_pmap;    // 读取出来的文件指针 
    PBYTE  m_BitPointer;      // 位图位置指针 

    LPWSTR m_ETagW;  // etag 
    LPWSTR m_lpFileName; 

    HANDLE m_hfile;
    HANDLE m_hbitmap;
    CLock  m_file_lock;    // 下载文件锁定 
    CLock  m_bitmap_lock;  // 位图文件锁 
};


#endif