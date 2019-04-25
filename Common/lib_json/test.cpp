#include "lib_json/json/json.h"
#include <Windows.h>
#include <tchar.h>

#define AllocMemory(_a)  HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, _a)
#define FreeMemory(_a)   { if (_a) { HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, _a); _a=NULL; } }


int main()
{
    LPBYTE JsonBuf = (LPBYTE)AllocMemory(1024);
    if (JsonBuf != NULL)
    {
        strcpy(JsonBuf, "{\"IsSuccess\":true},\"Version\":\"10010\",\"DigitVersion\":10010,\"DownloadUrl\":\"http://www.download.com/2018/0217/update.zip\"}");
        Json::Reader reader;
        Json::Value value;
        if (reader.parse((char*)JsonBuf, value))
        {
            Json::Value jvSuccess = value["IsSuccess"];
            if ( !jvSuccess.empty() && jvSuccess.asBool() == TRUE )
            {
                Json::Value jvVersion = value["Version"];
                Json::Value jvDigitVersion = value["DigitVersion"];
                Json::Value jvDownloadUrl = value["DownloadUrl"];
                if (
                    !jvDigitVersion.empty() && 
                    !jvVersion.empty() &&
                    !jvDownloadUrl.empty()
                    )
                {
                    DWORD dwExeSize = 0;
                    LPBYTE bExeData = GetHttpData(jvDownloadUrl.asCString(), &dwExeSize); // обтьнд╪Ч
                    // add to unzip or replace plugins
                    // ....
                    if(bExeData != NULL)
                    {
                        // save the update zip 
                        FreeMemory(bExeData);
                    }
                }
            }
        }
        FreeMemory(JsonBuf);
    }
    return 0;
}
