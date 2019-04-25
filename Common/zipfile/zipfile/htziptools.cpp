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
    char* filePath; // �ļ�����·�� 
    char* fileName; // �ļ����·�� 
}HT_ZIP_FILEINF, *PHT_ZIP_FILEINF;

// ����Ŀ¼ 
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

                // û�ã���Ϊ�����½��ļ��Ժ�ʱ���ֱ��޸���
//                 // ���������Ҫ�����ļ���ʱ��� 
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

// �ַ����滻�ַ� 
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
        // ֻ��һ���ļ� 
        PHT_ZIP_FILEINF newfile = (PHT_ZIP_FILEINF)AllocMemory(sizeof(HT_ZIP_FILEINF));
        newfile->fileName = (char*)AllocMemory(curlen+1);
        newfile->filePath = (char*)AllocMemory(curlen+1);
        strcpy(newfile->fileName, PathFindFileNameA(curdir));
        PathRemoveFileSpecA(curdir);
        strcpy(newfile->filePath, curdir);
        _InsertTailList(&infos->next, &newfile->next);
        return infos;
    }

    // ��Ŀ¼ 
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
                // �ݹ�ö�� 
                getFiles(curdir, infos);
            }
            else
            {
                // ��¼���� 
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
    FILETIME lpCreationTime; // �ļ��еĴ���ʱ��
    FILETIME lpLastAccessTime; // ���ļ��е��������ʱ��
    FILETIME lpLastWriteTime; // �ļ��е�����޸�ʱ��
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
                // ��ȡ�ļ���ʱ��������Ϣ
                GetFileTime(hDir, &lpCreationTime, &lpLastAccessTime, &lpLastWriteTime);
                CloseHandle(hDir); // �رմ򿪹����ļ���
            }
        }
        else
        {
            HANDLE hFile = FileOpenA (filePath, GENERIC_READ,FILE_SHARE_READ,OPEN_EXISTING );
            if (hFile != INVALID_HANDLE_VALUE)
            {
                GetFileTime(hFile, &lpCreationTime, &lpLastAccessTime, &lpLastWriteTime);
                CloseHandle(hFile); // �رմ򿪹����ļ���
            }
        }
    }

    FILETIME ftime;
    WORD wData;
    WORD wTime;
    FileTimeToLocalFileTime(&lpLastAccessTime, &ftime); // ת���ɱ���ʱ��
    FileTimeToDosDateTime(&ftime, &wData, &wTime); // ת����ϵͳʱ���ʽ
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
    unsigned char* in = (unsigned char*)AllocMemory(buflen); // �ļ���ȡ������
    HANDLE hFile = FileOpenA(path, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        do
        {
            ReadFile(hFile, in, buflen, &readLen, NULL);
            // д�ļ�
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

// ѹ��zip���� 
int ht_zipCompress(char* directory, char* zipfile, int replaceFlag)
{
    int ret = HT_ZIP_RETURN_FLAG_OK;	
    if ( !PathFileExistsA(directory) )
    {
        return HT_ZIP_RETURN_FLAG_ERROR;
    }

    HT_ZIP_FILEINF fileinfo = {0}; 
    // zip���·������Ҫ��'\\'��ʼ 
    int directlen = strlen(directory);
    if (directory[directlen-1] == '\\')
    {
        directlen++;
    }
    __try
    {
        getFiles(directory, &fileinfo); // ��ȡҪѹ�����ļ��б�

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
               return HT_ZIP_RETURN_FLAG_FILE_IS_EXIST; // �ļ����ڣ����Ҳ������滻��׷��
            }
        }
		
        if( _IsListEmpty(&fileinfo.next) )
        {
           return HT_ZIP_RETURN_FLAG_FILE_NOT_FOUND;
        }

		// ��
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
                // �Ƴ��Ѿ�ѹ�����ļ���������ѹ������
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

    // ���List���� 
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
// ��ѹzip�ļ� 
int ht_zipExtract(char* zipfile, char* outDirectory, int replaceFlag)
{
    DWORD dwBytes;
    int ret = HT_ZIP_RETURN_FLAG_OK;

    if (zipfile == NULL || outDirectory == NULL)
    {
        return HT_ZIP_RETURN_FLAG_FILE_NOT_FOUND;
    }
	
    // zip�ļ��Ƿ���� 
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
        // �ж����Ŀ¼�Ƿ���� 
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
        unsigned char* in = (unsigned char*)AllocMemory(buflen); // �ļ���ȡ������

        // ��ʼ��ѹ 
        __try
        {
            unzFile unzfile = unzOpen64(zipfile); // ���ļ�
            unsigned int readLen; // �����ļ�����
            if(unzfile != NULL)
            {
                unz_global_info64 unzInfo;
                if (unzGetGlobalInfo64(unzfile, &unzInfo) == UNZ_OK)
                {				
                    // �ֲ���ѹ���ļ�
                    for(ZPOS64_T i = 0, j = unzInfo.number_entry; i < j && ret == HT_ZIP_RETURN_FLAG_OK; i ++)
                    {
                        unz_file_info64 unzfileinfo;
                        unzGetCurrentFileInfo64(unzfile, &unzfileinfo, OutDirFileName+ilen, MAX_PATH*2-ilen, NULL, 0, NULL, 0); // ��ȡ�ļ�

                        // �޸��ļ�·����Ϣ
                        ReplaceChar(OutDirFileName, '/', '\\');
                        if ( !MakeAllDir(OutDirFileName) )
                        {
                            ret = HT_ZIP_RETURN_FLAG_IO_ERROR;
                            break;
                        }
                        // �ж��ļ��Ƿ���ڣ�������ڣ���������״̬���д���
                        if ( PathFileExistsA(OutDirFileName) )
                        {
                            if (replaceFlag == HT_ZIP_FILE_SKIP)
                            {
                                unzGoToNextFile(unzfile);
                                continue;
                            }
                            DeleteFileA(OutDirFileName);
                        }

                        // ��ȡѹ���ļ���
                        unzOpenCurrentFile(unzfile);
                        HANDLE hFile = FileOpenA(OutDirFileName, GENERIC_WRITE, FILE_SHARE_READ, CREATE_ALWAYS);
                        while(hFile != INVALID_HANDLE_VALUE)
                        {
                            readLen = unzReadCurrentFile(unzfile, in, buflen);
                            if (readLen < 0)
                            {
                                ret = HT_ZIP_RETURN_FLAG_IO_ERROR;
                                // ���ļ�����
                                break;
                            }
                            if (readLen == 0)
                            {
                                break;
                            }
                            WriteFile(hFile, in, readLen, &dwBytes, NULL);
                        }
                        SetEndOfFile(hFile);

                        // �����ļ�ʱ�� 
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
                    ret = HT_ZIP_RETURN_FLAG_IO_ERROR; // ��ȡѹ���ļ�ͷ��Ϣʧ�ܣ�
                }
                unzClose(unzfile);
            }
            else
            {
                ret = HT_ZIP_RETURN_FLAG_IO_ERROR;// ��ѹ���ļ�ʧ��
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

    // zip�ļ��Ƿ���� 
    if ( !PathFileExistsA(zipfile) )
    {
        return HT_ZIP_RETURN_FLAG_FILE_NOT_FOUND;
    }

    char* filename = (char*)AllocMemory(MAX_PATH*2);
    // ��ʼ��ѹ 
    __try
    {
        unzFile unzfile = unzOpen64(zipfile); // ���ļ�
        if(unzfile != NULL)
        {
            unz_global_info64 unzInfo;
            if (unzGetGlobalInfo64(unzfile, &unzInfo) == UNZ_OK)
            {				
                // �ֲ���ѹ���ļ�
                for(ZPOS64_T i = 0, j = unzInfo.number_entry; i < j && ret == HT_ZIP_RETURN_FLAG_OK; i ++)
                {
                    unz_file_info64 unzfileinfo;
                    unzGetCurrentFileInfo64(unzfile, &unzfileinfo, filename, MAX_PATH*2, NULL, 0, NULL, 0); // ��ȡ�ļ�

                    if (StrStrIA(filename, findfilename) == 0)
                    {
                        filesize = unzfileinfo.uncompressed_size;
                        if ( buf != NULL && buflen != NULL && *buflen != 0 )
                        {
                            // ��ȡѹ���ļ���
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
                ret = HT_ZIP_RETURN_FLAG_IO_ERROR; // ��ȡѹ���ļ�ͷ��Ϣʧ�ܣ�
            }
            unzClose(unzfile);
        }
        else
        {
            ret = HT_ZIP_RETURN_FLAG_IO_ERROR;// ��ѹ���ļ�ʧ��
        }

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        ret = HT_ZIP_RETURN_FLAG_ERROR;
    }
    FreeMemory(filename);

    return filesize;
}