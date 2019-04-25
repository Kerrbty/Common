#define _CRT_SECURE_NO_WARNINGS
#include "htziptools.h"
#include "List.h"
#include "defs.h"
#include <shlwapi.h>
#include <windows.h>
#pragma comment(lib, "shlwapi")

typedef struct _HT_ZIP_FILEINF
{
    LIST_ENTRY next;
    char* filePath; // 文件完整路径 
    char* fileName; // 文件相对路径 
}HT_ZIP_FILEINF, *PHT_ZIP_FILEINF;

// 创建目录 
bool MakeAllDir(const char* path)
{
    int ipathlen = strlen(path);
    char* makefolder = new char[ipathlen+2];
    memset(makefolder, 0, ipathlen+2);

    int ilastsp = 0;
    for (int i=0; i<ipathlen; i++)
    {
        if (path[i] == '\\')
        {
            if ( !PathFileExistsA(makefolder) )
            {
                CreateDirectoryA(makefolder, NULL);

                // 没用，因为里面新建文件以后，时间又被修改了
//                 // 如果存在需要设置文件夹时间的 
//                 if (DosTime != 0)
//                 {
//                     HANDLE hDir = CreateFileA(makefolder, GENERIC_READ|GENERIC_WRITE,
//                         FILE_SHARE_READ|FILE_SHARE_DELETE,
//                         NULL, OPEN_EXISTING,
//                         FILE_FLAG_BACKUP_SEMANTICS, NULL);
//                     if (hDir != INVALID_HANDLE_VALUE)
//                     {
//                         FILETIME filetime;
//                         FILETIME localtime;
//                         DosDateTimeToFileTime(HIWORD(DosTime), LOWORD(DosTime), &filetime);
//                         LocalFileTimeToFileTime(&filetime, &localtime);
//                         SetFileTime(hDir, &localtime, &localtime, &localtime);
//                         CloseHandle(hDir);
//                     }
//                 }
            }
            ilastsp = i;
        }
        makefolder[i] = path[i];
    }

    makefolder[ilastsp] = '\0';
    bool success = (PathFileExistsA(makefolder) == TRUE);
    delete[] makefolder;

    return success;
}

// 字符串替换字符 
char* ReplaceChar(char* szstr, char chsrc, char chdst)
{
    if (szstr != NULL)
    {
        char* szPoint = szstr;
        while (*szPoint != '\0')
        {
            if (*szPoint == chsrc)
            {
                *szPoint = chdst;
            }
            szPoint++;
        }
    }
    return szstr;
}

HT_ZIP_FILEINF* getFiles(const char* directory, HT_ZIP_FILEINF* infos)
{
    if (infos->next.Flink == NULL)
    {
        _InitializeListHead(&infos->next);
    }
    if ( !PathFileExistsA(directory) )
    {
        return infos;
    }

    int curlen = strlen(directory);
    char* curdir = (char*)AllocMemory(curlen+MAX_PATH);
    strcpy(curdir, directory);
    ReplaceChar(curdir, '/', '\\');
    if ( !PathIsDirectory(curdir) )
    {
        // 只是一个文件 
        PHT_ZIP_FILEINF newfile = (PHT_ZIP_FILEINF)AllocMemory(sizeof(HT_ZIP_FILEINF));
        newfile->fileName = (char*)AllocMemory(curlen+1);
        newfile->filePath = (char*)AllocMemory(curlen+1);
        strcpy(newfile->fileName, PathFindFileNameA(curdir));
        PathRemoveFileSpecA(curdir);
        strcpy(newfile->filePath, curdir);
        _InsertTailList(&infos->next, &newfile->next);
        return infos;
    }

    // 是目录 
    WIN32_FIND_DATAA FindFileData = {0};
    if (curdir[curlen-1] != '\\')
    {
        curdir[curlen] = '\\';
        curlen++;
    }
    strcpy(curdir+curlen, "*.*");
    HANDLE hFind = FindFirstFileA(curdir, &FindFileData);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do 
        {
            if (
                strcmp(FindFileData.cFileName, ".") == 0 ||
                strcmp(FindFileData.cFileName, "..") == 0
                )
            {
                continue;
            }

            strcpy(curdir+curlen, FindFileData.cFileName);
            if( FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY )
            {
                // 递归枚举 
                getFiles(curdir, infos);
            }
            else
            {
                // 记录数据 
                PHT_ZIP_FILEINF newfile = (PHT_ZIP_FILEINF)AllocMemory(sizeof(HT_ZIP_FILEINF));
                newfile->fileName = (char*)AllocMemory(curlen+MAX_PATH);
                newfile->filePath = (char*)AllocMemory(curlen+MAX_PATH);
                strcpy(newfile->fileName, PathFindFileNameA(curdir));
//                 PathRemoveFileSpecA(curdir);
                strcpy(newfile->filePath, curdir);
                _InsertTailList(&infos->next, &newfile->next);
                printf("List Add: %s\n", curdir);
            }
        } while ( FindNextFileA(hFind, &FindFileData) );
    }
    FreeMemory(curdir);
    FindClose(hFind);
    return infos;
}


long getFileLastAccessTime(const char* filePath)
{
    FILETIME lpCreationTime; // 文件夹的创建时间
    FILETIME lpLastAccessTime; // 对文件夹的最近访问时间
    FILETIME lpLastWriteTime; // 文件夹的最近修改时间
    SYSTEMTIME systemtime;

	long result = 0;
    GetSystemTime(&systemtime);
    SystemTimeToFileTime(&systemtime, &lpLastAccessTime);
    if ( PathFileExistsA(filePath) )
    {
        if (PathIsDirectory(filePath))
        {
            HANDLE hDir = CreateFileA (filePath, GENERIC_READ,
                FILE_SHARE_READ|FILE_SHARE_DELETE,
                NULL, OPEN_EXISTING,
                FILE_FLAG_BACKUP_SEMANTICS, NULL);
            if (hDir != INVALID_HANDLE_VALUE)
            {
                // 获取文件夹时间属性信息
                GetFileTime(hDir, &lpCreationTime, &lpLastAccessTime, &lpLastWriteTime);
                CloseHandle(hDir); // 关闭打开过的文件夹
            }
        }
        else
        {
            HANDLE hFile = FileOpenA (filePath, GENERIC_READ,FILE_SHARE_READ,OPEN_EXISTING );
            if (hFile != INVALID_HANDLE_VALUE)
            {
                GetFileTime(hFile, &lpCreationTime, &lpLastAccessTime, &lpLastWriteTime);
                CloseHandle(hFile); // 关闭打开过的文件夹
            }
        }
    }

    FILETIME ftime;
    WORD wData;
    WORD wTime;
    FileTimeToLocalFileTime(&lpLastAccessTime, &ftime); // 转换成本地时间
    FileTimeToDosDateTime(&ftime, &wData, &wTime); // 转换成系统时间格式
	return MAKELONG(wTime, wData);
}

int compress(zipFile& file, char* showPath, char* path)
{
    DWORD readLen;

	int ret = HT_ZIP_RETURN_FLAG_OK;

	zip_fileinfo fileinfo;
	fileinfo.dosDate = getFileLastAccessTime(path);
	zipOpenNewFileInZip64(file, showPath, &fileinfo, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION, 1);

    DWORD buflen = 20*1024*1024; // 20M 
    unsigned char* in = (unsigned char*)AllocMemory(buflen); // 文件读取缓冲区
    HANDLE hFile = FileOpenA(path, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        do
        {
            ReadFile(hFile, in, buflen, &readLen, NULL);
            // 写文件
            ret = zipWriteInFileInZip(file, in, readLen);
            if (ret != Z_OK)
            {
                zipCloseFileInZip(file);
                ret = HT_ZIP_RETURN_FLAG_IO_ERROR;
                break;
            }
        }while(readLen == buflen);
        zipCloseFileInZip(file);
        CloseHandle(hFile);
    }
    FreeMemory(in);
	return ret;
}

// 压缩zip方法 
int ht_zipCompress(char* directory, char* zipfile, int replaceFlag)
{
    int ret = HT_ZIP_RETURN_FLAG_OK;	
    if ( !PathFileExistsA(directory) )
    {
        return HT_ZIP_RETURN_FLAG_ERROR;
    }

    HT_ZIP_FILEINF fileinfo = {0}; 
    // zip相对路径不需要从'\\'开始 
    int directlen = strlen(directory);
    if (directory[directlen-1] == '\\')
    {
        directlen++;
    }
    __try
    {
        getFiles(directory, &fileinfo); // 获取要压缩的文件列表

        int openMode = APPEND_STATUS_CREATE;
        if (PathFileExistsA(zipfile))
        {
            if (replaceFlag = HT_ZIP_FILE_REPLACE)
            {
               DeleteFileA(zipfile);
            }
            else if(replaceFlag = HT_ZIP_FILE_APPEND)
            {
               openMode = APPEND_STATUS_ADDINZIP;
            }else
            {
               return HT_ZIP_RETURN_FLAG_FILE_IS_EXIST; // 文件存在，并且不允许替换或追加
            }
        }
		
        if( _IsListEmpty(&fileinfo.next) )
        {
           return HT_ZIP_RETURN_FLAG_FILE_NOT_FOUND;
        }

		// 打开
		zipFile _zipfile = zipOpen64(zipfile, openMode);
		for (PLIST_ENTRY plist = fileinfo.next.Flink; plist != &fileinfo.next; plist = plist->Flink )
        {
            PHT_ZIP_FILEINF pZipFileInfo = CONTAINING_RECORD(plist, HT_ZIP_FILEINF, next);
            char* RelativePath = pZipFileInfo->filePath + directlen + 1;
            char* AbsolutePath = pZipFileInfo->filePath;

            printf("Read List: %s\n", AbsolutePath);

            ret = compress(_zipfile, RelativePath, AbsolutePath);
            if (ret != HT_ZIP_RETURN_FLAG_OK)
            {
                // 移除已经压缩的文件，并结束压缩操作
                zipClose(_zipfile, NULL);
                DeleteFileA(zipfile);
                break;
            }
		}
		zipClose(_zipfile, NULL);
	}
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
		ret = HT_ZIP_RETURN_FLAG_ERROR;
	}

    // 清除List工作 
    if (fileinfo.next.Flink != NULL)
    {
        while( !_IsListEmpty(&fileinfo.next) )
        {
            PLIST_ENTRY plist = fileinfo.next.Flink;
            PHT_ZIP_FILEINF pZipFileInfo = CONTAINING_RECORD(plist, HT_ZIP_FILEINF, next);
            _RemoveEntryList(plist);
            FreeMemory(pZipFileInfo->fileName);
            FreeMemory(pZipFileInfo->filePath);
            FreeMemory(pZipFileInfo);
        }
    }
	return ret;
}


/*
const int HT_ZIP_RETURN_FLAG_OK = 0;
const int HT_ZIP_RETURN_FLAG_FILE_NOT_FOUND = -1;
const int HT_ZIP_RETURN_FLAG_EXTRACT_IS_FILE = -2;
const int HT_ZIP_RETURN_FLAG_IO_ERROR = -3;
const int HT_ZIP_RETURN_FLAG_ERROR = -4;
*/
// 解压zip文件 
int ht_zipExtract(char* zipfile, char* outDirectory, int replaceFlag)
{
    DWORD dwBytes;
    int ret = HT_ZIP_RETURN_FLAG_OK;

    if (zipfile == NULL || outDirectory == NULL)
    {
        return HT_ZIP_RETURN_FLAG_FILE_NOT_FOUND;
    }
	
    // zip文件是否存在 
#ifdef _MSC_VER // vs 
    if ( !PathFileExistsA(zipfile) )
#else  // gcc 
    if ( access(zipfile, 0) != 0 )
#endif
    {
		return HT_ZIP_RETURN_FLAG_FILE_NOT_FOUND;
	}

	char* OutDirFileName = (char*)AllocMemory(MAX_PATH*2);
    strcpy(OutDirFileName, outDirectory);
    int ilen = strlen(OutDirFileName);
    if (OutDirFileName[ilen-1] != '\\')
    {
        OutDirFileName[ilen] = '\\';
        ilen++;
    }
    do 
    {
        // 判断输出目录是否存在 
        if ( !PathFileExistsA(OutDirFileName) )
        {
            if( !MakeAllDir(ReplaceChar(OutDirFileName, '/', '\\')) )
            {
                ret = HT_ZIP_RETURN_FLAG_IO_ERROR;
                break;
            }
        }
        else if ( !PathIsDirectoryA(OutDirFileName) )
        {
            ret = HT_ZIP_RETURN_FLAG_EXTRACT_IS_FILE;
            break;
        }


        DWORD buflen = 20*1024*1024; // 20M 
        unsigned char* in = (unsigned char*)AllocMemory(buflen); // 文件读取缓冲区

        // 开始解压 
        __try
        {
            unzFile unzfile = unzOpen64(zipfile); // 打开文件
            unsigned int readLen; // 读入文件长度
            if(unzfile != NULL)
            {
                unz_global_info64 unzInfo;
                if (unzGetGlobalInfo64(unzfile, &unzInfo) == UNZ_OK)
                {				
                    // 分步解压缩文件
                    for(ZPOS64_T i = 0, j = unzInfo.number_entry; i < j && ret == HT_ZIP_RETURN_FLAG_OK; i ++)
                    {
                        unz_file_info64 unzfileinfo;
                        unzGetCurrentFileInfo64(unzfile, &unzfileinfo, OutDirFileName+ilen, MAX_PATH*2-ilen, NULL, 0, NULL, 0); // 获取文件

                        // 修改文件路径信息
                        ReplaceChar(OutDirFileName, '/', '\\');
                        if ( !MakeAllDir(OutDirFileName) )
                        {
                            ret = HT_ZIP_RETURN_FLAG_IO_ERROR;
                            break;
                        }
                        // 判断文件是否存在，如果存在，根据设置状态进行处理
                        if ( PathFileExistsA(OutDirFileName) )
                        {
                            if (replaceFlag == HT_ZIP_FILE_SKIP)
                            {
                                unzGoToNextFile(unzfile);
                                continue;
                            }
                            DeleteFileA(OutDirFileName);
                        }

                        // 读取压缩文件块
                        unzOpenCurrentFile(unzfile);
                        HANDLE hFile = FileOpenA(OutDirFileName, GENERIC_WRITE, FILE_SHARE_READ, CREATE_ALWAYS);
                        while(hFile != INVALID_HANDLE_VALUE)
                        {
                            readLen = unzReadCurrentFile(unzfile, in, buflen);
                            if (readLen < 0)
                            {
                                ret = HT_ZIP_RETURN_FLAG_IO_ERROR;
                                // 读文件错误
                                break;
                            }
                            if (readLen == 0)
                            {
                                break;
                            }
                            WriteFile(hFile, in, readLen, &dwBytes, NULL);
                        }
                        SetEndOfFile(hFile);

                        // 设置文件时间 
                        FILETIME filetime;
                        FILETIME localtime;
                        DosDateTimeToFileTime(HIWORD(unzfileinfo.dosDate), LOWORD(unzfileinfo.dosDate), &filetime);
                        LocalFileTimeToFileTime(&filetime, &localtime);
                        SetFileTime(hFile, &localtime, &localtime, &localtime);
                        CloseHandle(hFile);
                        
                        unzCloseCurrentFile(unzfile);
                        unzGoToNextFile(unzfile);
                    }
                }
                else
                {
                    ret = HT_ZIP_RETURN_FLAG_IO_ERROR; // 读取压缩文件头信息失败！
                }
                unzClose(unzfile);
            }
            else
            {
                ret = HT_ZIP_RETURN_FLAG_IO_ERROR;// 打开压缩文件失败
            }
            
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            ret = HT_ZIP_RETURN_FLAG_ERROR;
    	}
        FreeMemory(in);
    } while (0);

    FreeMemory(OutDirFileName);
	return ret;
}

unsigned __int64 ht_zipFindExtractToMemory(char* zipfile, char* findfilename, unsigned char* buf, unsigned long* buflen)
{
    int ret = HT_ZIP_RETURN_FLAG_OK;
    unsigned __int64 filesize = 0;

    if (zipfile == NULL || findfilename == NULL)
    {
        return HT_ZIP_RETURN_FLAG_FILE_NOT_FOUND;
    }

    // zip文件是否存在 
    if ( !PathFileExistsA(zipfile) )
    {
        return HT_ZIP_RETURN_FLAG_FILE_NOT_FOUND;
    }

    char* filename = (char*)AllocMemory(MAX_PATH*2);
    // 开始解压 
    __try
    {
        unzFile unzfile = unzOpen64(zipfile); // 打开文件
        if(unzfile != NULL)
        {
            unz_global_info64 unzInfo;
            if (unzGetGlobalInfo64(unzfile, &unzInfo) == UNZ_OK)
            {				
                // 分步解压缩文件
                for(ZPOS64_T i = 0, j = unzInfo.number_entry; i < j && ret == HT_ZIP_RETURN_FLAG_OK; i ++)
                {
                    unz_file_info64 unzfileinfo;
                    unzGetCurrentFileInfo64(unzfile, &unzfileinfo, filename, MAX_PATH*2, NULL, 0, NULL, 0); // 获取文件

                    if (StrStrIA(filename, findfilename) == 0)
                    {
                        filesize = unzfileinfo.uncompressed_size;
                        if ( buf != NULL && buflen != NULL && *buflen != 0 )
                        {
                            // 读取压缩文件块
                            unzOpenCurrentFile(unzfile);
                            *buflen = unzReadCurrentFile(unzfile, buf, *buflen);
                        }
                        break;
                    }
                    unzCloseCurrentFile(unzfile);
                    unzGoToNextFile(unzfile);
                }
            }
            else
            {
                ret = HT_ZIP_RETURN_FLAG_IO_ERROR; // 读取压缩文件头信息失败！
            }
            unzClose(unzfile);
        }
        else
        {
            ret = HT_ZIP_RETURN_FLAG_IO_ERROR;// 打开压缩文件失败
        }

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        ret = HT_ZIP_RETURN_FLAG_ERROR;
    }
    FreeMemory(filename);

    return filesize;
}