#include "Http.h"
#include "stdio.h"
#include "../defs.h"

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
//    cnblogs.SetAcceptEncodingA("gzip, deflate");  // ʹ�ü��ܱ�־λ,���յ���������Ҫ��zlib��ѹ 

    cnblogs.SetGetDataFunc(write_data, NULL);
    cnblogs.LoopToGetData();
}

int main()
{
    GetUrlData();
    CallBackReadData();
    return 0;
}