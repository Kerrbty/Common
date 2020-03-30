#include <Http/Http.h>
#include "stdio.h"
#include <defs.h>

#ifdef _DEBUG
#pragma comment(lib, "Http_d.lib")
#else
#pragma comment(lib, "Http.lib")
#endif


/////////////////////////////////////////////////////////////////////////
VOID GetUrlData()
{
    DWORD dwSize = 0;
    CHttp httpdata("https://www.baidu.com/");
    httpdata.SetRefererA("https://bbs.xxxxxx.com/");

    const char* lenth = httpdata.GetDataLenthA();
    if (lenth == NULL)
    {
        dwSize = 4 * 1024 * 1024; // 4M 
    }
    else
    {
        dwSize = atol(lenth);
    }
    PBYTE pBuf = (PBYTE)AllocMemory(dwSize + 4);
    httpdata.GetData(pBuf, dwSize);
//   printf("%s\n", pBuf);
    FreeMemory(pBuf);
}

/////////////////////////////////////////////////////////////////////////
unsigned long WINAPI write_data(unsigned char* lpBuf, unsigned long dwSize, void* userdata)
{
    printf("%s", lpBuf);
    return dwSize;
}
// 
VOID CallBackReadData()
{
    CHttp cnblogs("https://www.cnblogs.com/lifan3a/articles/7479256.html");
    cnblogs.SetAcceptA("text/html, application/xhtml+xml, */*");
    cnblogs.SetAcceptLanguageA("zh-CN");
    cnblogs.SetUserAgentA("Mozilla/5.0 (Windows NT 6.1; WOW64; Trident/7.0; rv:11.0) like Gecko");
//    cnblogs.SetAcceptEncodingA("gzip, deflate");  // 使用加密标志位,在收到数据上需要用zlib解压 

    cnblogs.SetGetDataFunc(write_data, NULL);
    cnblogs.LoopToGetData();
}

int main()
{
    GetUrlData();
    CallBackReadData();
    return 0;
}


int main2()
{
    CHttp csdnblogs("https://blog.csdn.net/mycar001/article/details/78391028");
    csdnblogs.SetAcceptEncodingA("gzip, deflate");   // 用压缩模式接收数据 
    csdnblogs.SetAutoUnzip(TRUE);  // 设置自动解压gzip数据 
    csdnblogs.SetUserAgentA("Mozilla/5.0 (Windows NT 6.1; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/80.0.3987.132 Safari/537.36");
    csdnblogs.SetAcceptLanguageA("zh-CN,zh;q=0.9,en;q=0.8");
    csdnblogs.SetAcceptA("*/*");
    
    DWORD dwDataLen = 0;
    const char* datalen = csdnblogs.GetDataLenthA();
    if (datalen)
    {
        dwDataLen = strtoul(datalen, NULL, 10);
        dwDataLen = dwDataLen*5;
    }
    else
    {
        dwDataLen = 4*1024*1024;
    }
    LPBYTE lpHtml = (LPBYTE)AllocMemory(dwDataLen);
    if (lpHtml)
    {
        csdnblogs.GetData(lpHtml, dwDataLen, TRUE);
        printf("%s\n", (char*)lpHtml);
        FreeMemory(lpHtml);
    }
    return 0;
}