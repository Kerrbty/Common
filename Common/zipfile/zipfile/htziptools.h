#ifndef _HTZIPTOOLS_H
#define _HTZIPTOOLS_H
#include "zip.h"
#include "unzip.h"

const int HT_ZIP_FILE_APPEND = 0;// ����ļ�����׷�ӣ�ѹ����
const int HT_ZIP_FILE_REPLACE = 1; // ����ļ������滻��ѹ����ѹ����
const int HT_ZIP_FILE_EXIT = 2;// ����ļ������˳���ѹ����
const int HT_ZIP_FILE_SKIP = 3;// ����ļ�������������ѹ����
const int HT_ZIP_RETURN_FLAG_OK = 0;
const int HT_ZIP_RETURN_FLAG_FILE_NOT_FOUND = -1;
const int HT_ZIP_RETURN_FLAG_FILE_IS_EXIST = -2;
const int HT_ZIP_RETURN_FLAG_EXTRACT_IS_FILE = -2;
const int HT_ZIP_RETURN_FLAG_IO_ERROR = -3;
const int HT_ZIP_RETURN_FLAG_ERROR = -4;


/**********************************************************************************
 * ѹ���ļ����ļ��У�������Ŀ¼��
 * directory����ѹ����Ŀ¼���ļ�
 * zipfile��������ļ�
 * replaceFlag���������ļ����ڵĴ���ʽ������ΪHT_ZIP_REPLACE���滻ԭ���ļ������
 *              ����ΪHT_ZIP_APPEND����ԭ�еĻ�����׷�����ļ�������Ϊֹͣѹ������
 * ����-1����ʾ��ѹ���ļ������ڣ����ļ���Ϊ��
 * ����-2����ʾ����ļ��Ѵ��ڣ���������replaceFlagΪHT_ZIP_FILE_EXIT
 * ����-3����ʾ�ļ���д��������
 * ����-4����ʾδ֪����
 * ���� 0����ʾ�����ɹ�
 **********************************************************************************/
int ht_zipCompress(char* directory, char* zipfile, int replaceFlag = 0);


/**********************************************************************************
 * ��ѹ���ļ���ָ��Ŀ¼
 * zipfile��Ҫ��ѹ�����ļ�
 * outDirectory�����Ŀ¼�����Ŀ¼�����ڣ��Զ�����
 * replaceFlag���������ļ����ڵĴ���ʽ������ΪHT_ZIP_REPLACE���滻ԭ���ļ������
 *              ����ΪHT_ZIP_FILE_SKIP�������ļ���������������Ϊ�滻
 * ����-1����ʾ����ѹ���ļ������� 
 * ����-2����ʾ���Ŀ¼��һ���ļ�
 * ����-3����ʾ�ļ���д��������
 * ����-4����ʾδ֪���� 
 * ���� 0����ʾ�����ɹ�
 **********************************************************************************/
int ht_zipExtract(char* zipfile, char* outDirectory, int replaceFlag = 1);


/**********************************************************************************
 * ��ȡѹ������ĳ��ָ�����ļ����ڴ� 
 * zipfile��ѹ�����ļ�
 * findfilename�������ļ�����Ҫ�������Ŀ¼
 * buf�� ���buf��Ҳ������NULL������Է�������û���ã�����ֻ�ǲ�ѯ�ļ���ѹ���С
 * buflen�� ָ�����buf�Ĵ�С 
 *
 * ��������ļ���ѹ����ļ���С(�������ڴ��С)
 **********************************************************************************/
unsigned __int64 ht_zipFindExtractToMemory(char* zipfile, char* findfilename, unsigned char* buf, unsigned long* buflen);


#endif