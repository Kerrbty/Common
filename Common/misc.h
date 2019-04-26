#ifndef _H_MISC_INCLUDED_
#define _H_MISC_INCLUDED_
#include <windows.h>
#include <excpt.h>
#ifdef __cplusplus
#define EXTERN_C   extern "C"
#else // __cplusplus 
#define EXTERN_C
#endif // __cplusplus 

#define ODIN_SETTING  _T("Brush.ini")
#define MTK_SETTING   _T("setting.ini")


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ������Ϣ���������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum ExcepionValue
{
	ExecuteHandler = EXCEPTION_EXECUTE_HANDLER, // ��ʾ����ִ��__except���ڼ������Ĵ���
	ResumeExce = EXCEPTION_CONTINUE_EXECUTION,   //��ʾ�ص��׳��쳣����������ִ��
	ContinueSearch = EXCEPTION_CONTINUE_SEARCH  //��ʾ������һ���쳣�����������
};

void FilePrint2(int id, LPCTSTR lpszFormant, LPCTSTR lpszFileName);

#define MYDEBUG
// #undef MYDEBUG
#ifdef MYDEBUG
void DebugPrintA(LPCSTR lpszFormant, ...);
void DebugPrintW(LPCWSTR lpszFormant, ...);
EXTERN_C void _cdecl FilePrintA(LPCSTR lpszFormant, ...);
EXTERN_C void _cdecl FilePrintW(LPCWSTR lpszFormant, ...);
EXTERN_C void log_printer(LPTSTR filename, LPCTSTR lpszFormant, ...);

#else
#define DebugPrintA(...)
#define FilePrintA(...)
#define DebugPrintW(...)
#define FilePrintW(...)
#endif  // end  MYDEBUG



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// �쳣���񡢼�¼������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LONG WINAPI BrushUnhandledExceptionFilter(_EXCEPTION_POINTERS *ExceptionInfo);
extern LPTOP_LEVEL_EXCEPTION_FILTER mOldFilter;
typedef LPTOP_LEVEL_EXCEPTION_FILTER (WINAPI* F_SetUnhandledExceptionFilter)(
	LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter 
	);
extern F_SetUnhandledExceptionFilter pOldSetUnhandledExceptionFilter;
LPTOP_LEVEL_EXCEPTION_FILTER WINAPI 
	HookSetUnhandledExceptionFilter( LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter ); //

int ErrorReport(LPEXCEPTION_POINTERS p_exinfo);    // __except �д� GetExceptionInformation(); �õ� LPEXCEPTION_POINTERS �ṹָ��




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ·���ַ�����������
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL StringMatch(LPCTSTR str, LPCTSTR match);
BOOL GetAppPathA(LPSTR lpszAppPath, int * length);
BOOL GetAppPathW(LPWSTR lpszAppPath, int * length);

LPCSTR GetPathExtA(LPCSTR szFilePath);
LPCWSTR GetPathExtW(LPCWSTR szFilePath);

void AddPathExt(LPCTSTR szFilePath1, LPCTSTR extName, LPTSTR szFilePath2);
BOOL GetPrevExt(LPCTSTR szFilePath, LPTSTR szExt);
BOOL DelExtA(LPSTR szFilePath);
BOOL DelExtW(LPWSTR szFilePath);
LPCSTR GetFileNameA(LPCSTR lpszFilePath);
LPCWSTR GetFileNameW(LPCWSTR lpszFilePath);
DWORD GetMyFileSize(LPTSTR szFilePath);
char * TrimSpace(char * ps);
BOOL   IsFileExists(LPCTSTR   lpszFileName)  ; 
UINT DebugInfoSet(LPCTSTR szinfo, LPCTSTR szFileName);



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ANSIC�汾��UNICODE�汾����
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _UNICODE

#ifdef _DEBUG
#define DebugPrint DebugPrintW
#else
#define DebugPrint()
#endif  // end _DEBUG

#define FilePrint FilePrintW
#define GetFileName GetFileNameW
#define GetAppPath GetAppPathW
#define GetPathExt GetPathExtW
#define DelExt  DelExtW

#else  // _UNICODE
#ifdef _DEBUG
#define DebugPrint DebugPrintA
#else
#define DebugPrint() 
#endif  // end _DEBUG
#define FilePrint FilePrintA
#define GetFileName GetFileNameA
#define GetAppPath GetAppPathA
#define GetPathExt GetPathExtA
#define DelExt  DelExtA
#endif  // end _UNICODE



#endif  // end define  _H_MISC_INCLUDED_ 