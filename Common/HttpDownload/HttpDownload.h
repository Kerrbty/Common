#ifndef _HTTP_DWONLOAD_HEADER_HH_H_
#define _HTTP_DWONLOAD_HEADER_HH_H_
#include "../pv/lock.h"


#define DLTPMAGIC  "DLTP\0\0\0"
#define BLOCK_SIZE  4*1024*1024    // 4M 
#pragma pack(push, 1)

typedef struct _url_str 
{
    unsigned int urllen;     // ���ص�ַ����   
    WCHAR down_url[1];            // ���ص�ַ�б� 
}url_str, *purl_str;

typedef struct _BitMap_Header
{
    char    MAGIC[8];
    unsigned long long filesize;  // �ļ���С 
    unsigned long long filedown;  // �Ѿ����صĴ�С 
    unsigned long  blockcount;     // �ļ������ 
    unsigned long  blocksize;     // һ�����ʾ�ļ��Ĵ�С 
    unsigned long  lastblocksize;  // ���һ����������ֽ��� 
    unsigned long  nUrlCount;     // ����ͬʱ֧�ֶ��URL������p2p 
    url_str        url[1];        // ���ص�url�б� 
}BitMap_Header, *PBitMap_Header;

// typedef union _BitFilePoint
// {
//     BYTE lpBuf[1];
//     BitMap_Header bitmap;
// }BitFilePoint, *PBitFilePoint;
#pragma pack(pop)


// xxx.dltp  �������ļ�
// xxx.dlp.cfg �����ع�����Ϣ�ļ�

class CDownLoad
{
public:
    CDownLoad(const char* szUrl, const char* szSaveFile, int threadcount = 6);
    CDownLoad(const WCHAR* szUrl, const WCHAR* szSaveFile, int threadcount = 6);
    ~CDownLoad();

    static DWORD WINAPI DownloadThread(LPVOID lparam);
    static DWORD WINAPI DownloadCalcSpeed(LPVOID lparam);
    static unsigned long WINAPI Write(unsigned char* lpBuf, unsigned long dwSize, void* userdata);

    int Run();    // ��ʼ���أ�����0 ��ʾ�ɹ���������ʾʧ��ԭ�� 

    unsigned long long  DownloadSize(); // ��ȡ���ش�С 
    unsigned long long  FileTotalSize(); // ������ļ���С
    unsigned long long  DwonloadSpeed(); // �����ٶ� 

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
    void UpdateBitFile();   // ʵʱ���� 

private:
    // �⼸��������Ҫ�� DownloadThread ���� 
    int    m_nCurrentBlock;     // ��ǰ�ڼ����̣߳��������ؿ����� 
    int    m_nthread;         // �̸߳��� 
    BOOL   m_CanDownLoad;     // �Ƿ����أ���ͣ��������˳���

    BOOL   m_downSuccess;

    // ���������ٶ��� 
    unsigned long long m_down_speed;
    unsigned long long m_down_size;
    HANDLE m_hSpeedThread;

    LPHANDLE m_hThreadList;   // �߳�ָ�� 
    BOOL  m_CanMulThread;     // URL�Ƿ�֧�ֶ��߳�

    PBitMap_Header m_pmap;    // ��ȡ�������ļ�ָ�� 
    PBYTE  m_BitPointer;      // λͼλ��ָ�� 

    LPWSTR m_ETagW;  // etag 
    LPWSTR m_lpFileName; 

    HANDLE m_hfile;
    HANDLE m_hbitmap;
    CLock  m_file_lock;    // �����ļ����� 
    CLock  m_bitmap_lock;  // λͼ�ļ��� 
};


#endif