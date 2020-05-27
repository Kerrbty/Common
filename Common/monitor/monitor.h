/************************************************************************ 
  @file monitor.h 
  * Author: qinqing 
  * Date:   2009-12-2 
************************************************************************/ 
 
#ifndef _MONITOR_H 
 
#include <windows.h>
#include <tchar.h>
#if WINVER < 0x0500 
#include <multimon.h> 
#endif 
#include <map> 
#include <string>
#include <vector>
#ifdef _UNICODE
#define tstring  std::wstring
#else
#define tstring std::string
#endif

#define SINGLETON_DECLARE(classname)    \
    static classname * instance();
 
#define SINGLETON_IMPLEMENT(classname)  \
    classname * classname::instance()       \
{                                               \
    static classname * _instance = NULL;        \
    if( NULL == _instance)                      \
    {                                           \
    _instance = new classname;              \
    }                                           \
    return _instance;                           \
}

/** 
  @brief ��ʾ���� 
  @class CMonitor 
*/ 
class CMonitor 
{ 
	friend class CMonitorManager; 
public: 
	void   GetMonitorRect(RECT& rect) const; 
	void   GetWorkAreaRect(RECT& rect) const; 
	DWORD  GetPixelWidth() const; 
	DWORD  GetPixelHeight() const; 
	DWORD  GetBitsPerPixel() const; 
	DWORD  GetMonitorLeft() const; 
	DWORD  GetMonitorTop() const; 
    DWORD  GetDisplayFrequency() const;  
    bool   IsPrimaryMonitor() const; 
	tstring GetName() const { return m_strName; } 
	DWORD   GetIndex() const { return m_dwIndex; } 
	 
	operator HMONITOR() const { return m_hMonitor; } 
protected: 
	void CenterRectToMonitor(LPRECT lprc, bool bWorkArea= false) const; 
 
private: 
	CMonitor(HMONITOR hMonitor, DWORD dwIndex); 
	virtual ~CMonitor(); 
 
private: 
    HMONITOR m_hMonitor; ///< ��ʾ����� 
	DWORD    m_dwIndex;  ///< ��ʾ������ 
	tstring  m_strName;  ///< ��ʾ������ 
}; 
 
/** 
  @brief ��ʾ�������� 
  @class CMonitorManager 
  * ��֧�ֶ���������ʾģʽ(ÿ����ʾ���ֱ����Ƕ�����),����ģʽ(��ˮƽ,��ֱ,����)�ݲ����� 
*/ 
class CMonitorManager 
{ 
	SINGLETON_DECLARE(CMonitorManager) 
 
public: 
    ~CMonitorManager();
    DWORD     GetCount(); 
	CMonitor* GetMonitor(DWORD dwIndex); 
	CMonitor* GetPrimaryMonitor(); 
    CMonitor* GetNearestMonitor(const RECT& rect); 
    CMonitor* GetNearestMonitor(const POINT& pt); 
	CMonitor* GetNearestMonitor(HWND hWnd); 
    
public: 
	static void GetVirtualDesktopRect(RECT& rect); 
	bool FullScreenWindow(HWND hWnd, bool bFullScreen, int iMonitorIndex = 0); 
	bool FullScreenWindow(HWND hWndFirst, HWND hWndSecond); 
 
	void CenterWindow(HWND hWnd, int iMonitorIndex = 0, bool bUseWorkArea = false); 
 
protected: 
	static BOOL CALLBACK AddMonitorsCallBack(HMONITOR hMonitor,HDC hdcMonitor,LPRECT lprcMonitor,LPARAM dwData); 
 
private: 
    CMonitorManager();
	void      CenterWindowToAll(RECT& rect, bool bUseWorkArea = false); 
	void      UpdateMonitors(); 
	CMonitor* FindMonitor(HMONITOR hMonitor); 
	void      FreeMonitors(); 
 
private: 
    std::vector<CMonitor*> m_vec_monitor; 
	struct WndInfo 
	{ 
		WndInfo(long lStyle, long lExStyle, RECT rcWnd, bool bFull): 
		_lStyle(lStyle), _lExStyle(lExStyle), _rcWnd(rcWnd), _bFull(bFull){} 
		WndInfo(){}; 
		long _lStyle;   ///< ��ͨ��ʽ 
		long _lExStyle; ///< ��չ��ʽ 
		RECT _rcWnd;    ///< ���� 
		bool _bFull;    ///< �Ƿ���ȫ��:true��,false�� 
	}; 
	///< ����ָ�����ʱ��״̬ 
    std::map<HWND, WndInfo> m_map_wnd;   
}; 
 
#endif
