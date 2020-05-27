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
  @brief 显示器类 
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
    HMONITOR m_hMonitor; ///< 显示器句柄 
	DWORD    m_dwIndex;  ///< 显示器索引 
	tstring  m_strName;  ///< 显示器名称 
}; 
 
/** 
  @brief 显示器管理类 
  @class CMonitorManager 
  * 仅支持多屏独立显示模式(每个显示器分辨率是独立的),其它模式(大水平,大垂直,复制)暂不考虑 
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
		long _lStyle;   ///< 普通样式 
		long _lExStyle; ///< 扩展样式 
		RECT _rcWnd;    ///< 矩形 
		bool _bFull;    ///< 是否已全屏:true是,false否 
	}; 
	///< 保存恢复窗口时的状态 
    std::map<HWND, WndInfo> m_map_wnd;   
}; 
 
#endif
