#include "stdafx.h"   
#include "monitor.h"   
#include <stack>   
#include <cassert>   
/*************************************************************************************************  
   2009-12-5 ����ʾ��������ʱ,Ҫɾ�����ڼ�¼��Ϣ  
   2009-12-8 ��Ϊʹ������ڸ����ڵ�λ�����ָ��Ӵ���,������������λ�úʹ�С�仯ʱ,�Ӵ��ڶ�����ȷ�ָ�  
**************************************************************************************************/   
   
//////////////////////////////////////////////////////////////////////////   
//CMonitorʵ��   
CMonitor::CMonitor(HMONITOR hMonitor, DWORD dwIndex):   
m_hMonitor(hMonitor),   
m_dwIndex(dwIndex)   
{   
    MONITORINFOEX mi;   
    mi.cbSize = sizeof( mi );   
    ::GetMonitorInfo( m_hMonitor, &mi );   
    m_strName = mi.szDevice;   
}   
   
CMonitor::~CMonitor()   
{   
   
}   
   
void CMonitor::GetMonitorRect(RECT& rect) const   
{   
    MONITORINFO mi;   
    RECT        rc;   
    mi.cbSize = sizeof( mi );   
    ::GetMonitorInfo( m_hMonitor, &mi );   
    rc = mi.rcMonitor;   
   
    ::SetRect(&rect, rc.left, rc.top, rc.right, rc.bottom );   
}   
   
void CMonitor::GetWorkAreaRect(RECT& rect) const   
{   
    MONITORINFO mi;   
    RECT        rc;   
    mi.cbSize = sizeof( mi );   
    ::GetMonitorInfo( m_hMonitor, &mi );   
    rc = mi.rcWork;   
   
    ::SetRect(&rect, rc.left, rc.top, rc.right, rc.bottom );   
}   
   
DWORD CMonitor::GetPixelHeight() const   
{   
    DEVMODE devmode;   
    devmode.dmSize = sizeof(DEVMODE);   
    EnumDisplaySettings(m_strName.c_str(), ENUM_CURRENT_SETTINGS, &devmode);   
    return devmode.dmPelsHeight;   
}   
   
DWORD CMonitor::GetPixelWidth() const   
{   
    DEVMODE devmode;   
    devmode.dmSize = sizeof(DEVMODE);   
    EnumDisplaySettings(m_strName.c_str(), ENUM_CURRENT_SETTINGS, &devmode);   
    return devmode.dmPelsWidth;    
}   
   
DWORD CMonitor::GetBitsPerPixel() const   
{   
    DEVMODE devmode;   
    devmode.dmSize = sizeof(DEVMODE);   
    EnumDisplaySettings(m_strName.c_str(), ENUM_CURRENT_SETTINGS, &devmode);   
    return devmode.dmBitsPerPel;   
}   
   
DWORD CMonitor::GetDisplayFrequency() const   
{   
    DEVMODE devmode;   
    devmode.dmSize = sizeof(DEVMODE);   
    EnumDisplaySettings(m_strName.c_str(), ENUM_CURRENT_SETTINGS, &devmode);   
    return devmode.dmDisplayFrequency;   
}   
   
DWORD CMonitor::GetMonitorLeft() const   
{   
    DEVMODE devmode;   
    devmode.dmSize = sizeof(DEVMODE);   
    EnumDisplaySettings(m_strName.c_str(), ENUM_CURRENT_SETTINGS, &devmode);   
    return devmode.dmPosition.y;   
}   
   
DWORD CMonitor::GetMonitorTop() const   
{   
    DEVMODE devmode;   
    devmode.dmSize = sizeof(DEVMODE);   
    EnumDisplaySettings(m_strName.c_str(), ENUM_CURRENT_SETTINGS, &devmode);   
    return devmode.dmPosition.y;   
}   
   
bool CMonitor::IsPrimaryMonitor() const   
{   
    MONITORINFO mi;   
    mi.cbSize = sizeof( mi );   
    ::GetMonitorInfo( m_hMonitor, &mi );   
    return mi.dwFlags == MONITORINFOF_PRIMARY;   
}   
   
void CMonitor::CenterRectToMonitor( LPRECT lprc, bool bWorkArea /*=false*/) const   
{   
    int  w = lprc->right - lprc->left;   
    int  h = lprc->bottom - lprc->top;   
   
    RECT rect;   
    if (bWorkArea)   
        GetWorkAreaRect(rect);   
    else   
        GetMonitorRect(rect);   
   
    lprc->left = rect.left + ( rect.right - rect.left - w ) / 2;   
    lprc->top = rect.top + ( rect.bottom - rect.top - h ) / 2;   
    lprc->right  = lprc->left + w;   
    lprc->bottom = lprc->top + h;   
}   
   
//////////////////////////////////////////////////////////////////////////   
//CMonitorManager ʵ��   
SINGLETON_IMPLEMENT(CMonitorManager)   
   
CMonitorManager::CMonitorManager()
{   
   UpdateMonitors();   
}   
   
CMonitorManager::~CMonitorManager()   
{   
   FreeMonitors();   
}   
   
void CMonitorManager::UpdateMonitors()   
{   
   FreeMonitors();   
   ::EnumDisplayMonitors( NULL, NULL, AddMonitorsCallBack, (LPARAM)this);   
}   
   
void CMonitorManager::FreeMonitors()   
{   
    for (std::vector<CMonitor*>::iterator iter = m_vec_monitor.begin(); iter != m_vec_monitor.end();)   
    {   
        delete (*iter); iter = m_vec_monitor.erase(iter);   
    }   
}   
   
DWORD CMonitorManager::GetCount()   
{   
   return m_vec_monitor.size();   
}   
   
CMonitor* CMonitorManager::GetMonitor(DWORD dwIndex)   
{   
    assert(dwIndex > 0);   
    if (dwIndex > m_vec_monitor.size())  return 0;   
    return m_vec_monitor[dwIndex-1];   
}   
   
CMonitor* CMonitorManager::GetPrimaryMonitor()   
{   
   for (std::vector<CMonitor*>::iterator iter = m_vec_monitor.begin(); iter != m_vec_monitor.end(); ++iter)   
   {   
       if ((*iter)->IsPrimaryMonitor()) return (*iter);   
   }   
   return 0;   
}   
   
CMonitor* CMonitorManager::GetNearestMonitor(const RECT& rect)   
{   
    HMONITOR hMonitor = ::MonitorFromRect(&rect, MONITOR_DEFAULTTONEAREST);   
    assert(hMonitor);   
    return FindMonitor(hMonitor);   
}   
   
CMonitor* CMonitorManager::GetNearestMonitor(const POINT& pt)   
{   
    HMONITOR hMonitor = ::MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);   
    assert(hMonitor);   
    return FindMonitor(hMonitor);   
}   
   
CMonitor* CMonitorManager::GetNearestMonitor(HWND hWnd)   
{   
    HMONITOR hMonitor = ::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);   
    assert(hMonitor);   
    return FindMonitor(hMonitor);   
}   
   
CMonitor* CMonitorManager::FindMonitor(HMONITOR hMonitor)   
{   
    for (std::vector<CMonitor*>::iterator iter = m_vec_monitor.begin(); iter != m_vec_monitor.end(); ++iter)   
    {   
        if ((*iter)->m_hMonitor == hMonitor) return (*iter);   
    }   
    return 0;   
}   
   
BOOL CALLBACK CMonitorManager::AddMonitorsCallBack(HMONITOR hMonitor,HDC hdcMonitor,LPRECT lprcMonitor,LPARAM dwData)   
{   
   CMonitorManager* pMonitorManger = (CMonitorManager*)dwData;   
   int nIndex = pMonitorManger->m_vec_monitor.size();   
   CMonitor* pMonitor = new CMonitor(hMonitor, ++nIndex);   
   pMonitorManger->m_vec_monitor.push_back(pMonitor);   
   return TRUE;   
}   
   
void CMonitorManager::GetVirtualDesktopRect(RECT& rect)   
{   
    ::SetRect(&rect,::GetSystemMetrics( SM_XVIRTUALSCREEN ),::GetSystemMetrics( SM_YVIRTUALSCREEN ),   
             ::GetSystemMetrics( SM_CXVIRTUALSCREEN ),::GetSystemMetrics( SM_CYVIRTUALSCREEN ));   
}   
   
void CMonitorManager::CenterWindowToAll(RECT& rect, bool bUseWorkArea /* = false */)   
{   
    RECT rcAll, rcWork;   
    int  iWidth, iHeight;   
    if (bUseWorkArea)   
    {   
        GetPrimaryMonitor()->GetWorkAreaRect(rcWork);   
        GetVirtualDesktopRect(rcAll);   
        iWidth = rect.right-rect.left, iHeight = rect.bottom-rect.top;   
        rect.left = rcWork.left + (rcAll.right-rcAll.left-iWidth)/2;   
        rect.top  = rcWork.top + (rcAll.bottom-rcAll.top-iHeight)/2;   
        rect.right = rect.left + iWidth;   
        rect.bottom = rect.top + iHeight;   
    }   
    else   
    {   
        GetVirtualDesktopRect(rcAll);   
        iWidth = rect.right-rect.left, iHeight = rect.bottom-rect.top;   
        rect.left = rcAll.left + (rcAll.right-rcAll.left-iWidth)/2;   
        rect.top  = rcAll.top + (rcAll.bottom-rcAll.top-iHeight)/2;   
        rect.right = rect.left + iWidth;   
        rect.bottom = rect.top + iHeight;   
    }   
}   
/**   
  @brief  ���д���  
  @param hWnd  ���ھ��  
  @param iMonitorIndex  ��ʾ������,�������������:    
  1)<0--���ʾ������������� 2)0--���ڵ�ǰ��ʾ������ 3)>=1--���ʾ��������ʾ������  
  @param bUseWorkArea  �Ƿ��ڶ�Ӧ��ʾ���������о���: true����ʾ������������,false����ʾ���������    
*/   
void CMonitorManager::CenterWindow(HWND hWnd,int iMonitorIndex /* = 0 */, bool bUseWorkArea /* = false*/)   
{   
    RECT rect;   
    ::GetWindowRect(hWnd, &rect );   
    if (0 > iMonitorIndex )   
    {   
       CenterWindowToAll(rect, bUseWorkArea);   
    }   
    else if (0 == iMonitorIndex)   
    {   
       CMonitorManager::GetNearestMonitor(hWnd)->CenterRectToMonitor(&rect,bUseWorkArea);   
    }   
    else   
    {   
        CMonitor* pMonitor = CMonitorManager::GetMonitor(iMonitorIndex);   
        if (!pMonitor)  return;   
        pMonitor->CenterRectToMonitor(&rect, bUseWorkArea);   
    }   
    ::SetWindowPos(hWnd, NULL, rect.left, rect.top, 0, 0, SWP_NOSIZE|SWP_NOZORDER);   
}   
   
/**  
    @brief ����ʾ��ȫ��  
    @param hWnd ��ȫ���Ĵ���  
    @param bFullScreen �Ƿ�ȫ��: true��ʾȫ��,false��ʾ�ָ�  
    @param iMonitorIndex ��ʾ������,�������������:   
    1)<0--���ʾ����������ȫ�� 2)0--���ڵ�ǰ��ʾ��ȫ�� 3)>=1--���ʾ��������ʾ��ȫ��  
    * ����ʵ��ȫ��,����ȥ���߿���ʽ,ֻ���ͻ���  
    @return  true��ʾ�����ɹ�,�Ѿ�ȫ����ָ�;false��ʾ����ʧ��,û��ȫ����ָ�  
    * FullScreenWindow(hWnd,true,iMonitorIndex)��FullScreenWindow(hWnd,false)Ҫ�ɶԵ�������ȫ���ͻָ�  
*/   
bool CMonitorManager::FullScreenWindow(HWND hWnd, bool bFullScreen, int iMonitorIndex /*= 0*/)   
{   
    if (!hWnd||!IsWindow(hWnd))  return false;   
   
    std::stack<HWND>  WndStack;   
    HWND         hParent;   
    std::map<HWND, WndInfo>::iterator iter;    
   
    long lOldStyle, lNewStyle;   
    long lOldExStyle, lNewExStyle;   
    RECT rcNew, rcOld, rcMonitor;   
    lNewStyle = lOldStyle = GetWindowLong(hWnd, GWL_STYLE);   
    lOldExStyle = lNewExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);   
   
    if (bFullScreen)   
    {   
        GetWindowRect(hWnd, &rcOld);   
        hParent = GetParent(hWnd);   
        if (hParent && (lOldStyle&WS_CHILD))   
        {   
            ScreenToClient(hParent, rcOld);   
        }   
        m_map_wnd.insert(make_pair(hWnd, WndInfo(lOldStyle, lOldExStyle, rcOld, false)));   
    }      
    if (lOldStyle & WS_CHILD)   
    {   
        for (HWND hTempParent = GetParent(hWnd); hTempParent; )   
        {   
            hParent = hTempParent; hTempParent = GetParent(hParent);   
            WndStack.push(hParent);   
            lOldStyle = GetWindowLong(hParent, GWL_STYLE);   
            if (bFullScreen)   
            {   
                GetWindowRect(hParent, &rcOld);   
                lOldExStyle = GetWindowLong(hParent, GWL_EXSTYLE);   
                if (hTempParent && (lOldStyle&WS_CHILD))   
                {   
                    ::ScreenToClient(hTempParent, rcOld);   
                }   
                m_map_wnd.insert(make_pair(hParent, WndInfo(lOldStyle, lOldExStyle, rcOld, false)));   
            }   
            if (lOldStyle&WS_POPUP || lOldStyle&WS_OVERLAPPED)  break;   
        }   
    }   
    else   //����ʽ���ص�����   
    {   
        if (bFullScreen)   
        {   
           if (lOldStyle & WS_CAPTION)  lNewStyle &= ~WS_CAPTION;   
           if (lOldStyle & WS_DLGFRAME) lNewStyle &= ~WS_DLGFRAME;   
           if (lOldStyle & WS_THICKFRAME) lNewStyle &= ~WS_THICKFRAME;   
           if (lOldStyle & WS_BORDER) lNewStyle &= ~WS_BORDER;   
           if ((lOldExStyle & WS_EX_DLGMODALFRAME)) lNewExStyle &= ~WS_EX_DLGMODALFRAME;   
           if (lOldExStyle & WS_EX_WINDOWEDGE) lNewExStyle &= ~WS_EX_WINDOWEDGE;   
           if (lOldExStyle != lNewExStyle)  SetWindowLong(hWnd, GWL_EXSTYLE, lNewExStyle);   
           if (lOldStyle != lNewStyle)      SetWindowLong(hWnd, GWL_STYLE, lNewStyle);   
        }   
    }   
    //���ȫ����ָ��丸����   
    if (bFullScreen)   
    {   
        if (0 > iMonitorIndex)   
        {   
            GetVirtualDesktopRect(rcMonitor);   
        }   
        else if (0 == iMonitorIndex)   
        {   
            GetNearestMonitor(hWnd)->GetMonitorRect(rcMonitor);   
        }   
        else    
        {   
            CMonitor* pMonitor = GetMonitor(iMonitorIndex);   
            if (!pMonitor)    
            {   
                m_map_wnd.erase(hWnd);  return false;   
            }   
            pMonitor->GetMonitorRect(rcMonitor);   
        }   
        rcNew = rcMonitor;   
    }   
    for (; !WndStack.empty(); WndStack.pop())   
    {   
        hParent = WndStack.top();   
        iter = m_map_wnd.find(hParent);   
        if (bFullScreen)   
        {   
            if (iter != m_map_wnd.end() && (*iter).second._bFull) continue;   
            lOldStyle = lNewStyle = GetWindowLong(hParent,GWL_STYLE);   
            lOldExStyle = lNewExStyle = GetWindowLong(hParent, GWL_EXSTYLE);   
            if (lOldStyle & WS_CAPTION)  lNewStyle &= ~WS_CAPTION;   
            if (lOldStyle & WS_BORDER)   lNewStyle &= ~WS_BORDER;   
            if (lOldStyle & WS_DLGFRAME) lNewStyle &= ~WS_DLGFRAME;   
            if (lOldStyle & WS_THICKFRAME) lNewStyle &= ~WS_THICKFRAME;   
            if (lOldExStyle & WS_EX_DLGMODALFRAME) lNewExStyle &= ~WS_EX_DLGMODALFRAME;   
            if (lOldExStyle & WS_EX_WINDOWEDGE)    lNewExStyle &= ~WS_EX_WINDOWEDGE;   
            if (lOldExStyle != lNewExStyle) SetWindowLong(hParent, GWL_EXSTYLE, lNewExStyle);   
            if (lOldStyle != lNewStyle)     SetWindowLong(hParent, GWL_STYLE, lNewStyle);   
            if (lOldStyle & WS_CHILD)   
            {   
                ScreenToClient(hParent, rcNew);   
            }   
            SetWindowPos(hParent, 0, rcNew.left,rcNew.top,rcNew.right-rcNew.left,rcNew.bottom-rcNew.top,   
                         SWP_SHOWWINDOW|SWP_NOZORDER|SWP_FRAMECHANGED);   
            (*iter).second._bFull = true;   
        }   
        else   
        {   
            if (iter == m_map_wnd.end() || !(*iter).second._bFull)  continue;   
            WndInfo& wndinfo = (*iter).second; wndinfo._bFull = false;   
            rcOld = wndinfo._rcWnd;   
            SetWindowLong(hParent, GWL_STYLE, wndinfo._lStyle);   
            SetWindowLong(hParent, GWL_EXSTYLE, wndinfo._lExStyle);   
            SetWindowPos(hParent, 0, rcOld.left,rcOld.top,rcOld.right-rcOld.left,rcOld.bottom-rcOld.top,   
                         SWP_SHOWWINDOW|SWP_NOZORDER);   
            m_map_wnd.erase(iter);   
        }   
    }   
    //ȫ����ָ�������   
    hParent = GetParent(hWnd);  iter = m_map_wnd.find(hWnd);   
    if (bFullScreen)   
    {   
        if (iter != m_map_wnd.end() && (*iter).second._bFull)  return true;   
        lOldStyle = GetWindowLong(hWnd, GWL_STYLE);    
        if (hParent && (lOldStyle & WS_CHILD))    
        {   
            for(HWND hChild = ::GetTopWindow(hParent); hChild; hChild = ::GetNextWindow(hChild, GW_HWNDNEXT))   
            {   
                if (hChild != hWnd) ::ShowWindow(hChild, SW_HIDE);   
            }   
            ScreenToClient(hParent, rcNew);   
        }   
        SetWindowPos(hWnd, 0, rcNew.left,rcNew.top,rcNew.right-rcNew.left,rcNew.bottom-rcNew.top,   
            SWP_SHOWWINDOW|SWP_FRAMECHANGED|SWP_NOZORDER);   
        (*iter).second._bFull = true;   
    }   
    else   
    {   
       if (iter == m_map_wnd.end() || !(*iter).second._bFull)  return true;   
       WndInfo& wndinfo = (*iter).second; wndinfo._bFull = false;   
       rcOld = wndinfo._rcWnd;   
       SetWindowLong(hWnd, GWL_STYLE, wndinfo._lStyle);   
       SetWindowLong(hWnd, GWL_EXSTYLE, wndinfo._lExStyle);   
       if (hParent && (wndinfo._lStyle & WS_CHILD))   
       {   
           for(HWND hChild = ::GetTopWindow(hParent); hChild; hChild = ::GetNextWindow(hChild, GW_HWNDNEXT))   
           {   
              ::ShowWindow(hChild, SW_SHOWNORMAL);   
           }   
       }   
       SetWindowPos(hWnd, 0, rcOld.left,rcOld.top,rcOld.right-rcOld.left,rcOld.bottom-rcOld.top,   
           SWP_SHOWWINDOW|SWP_NOZORDER);   
       m_map_wnd.erase(iter);   
    }   
    return true;   
}   
   
/**   
  @brief ˫��ʾ��ȫ��  
  @param hWndFirst  ����ʾ��1ȫ���Ĵ��ھ��  
  @param hWndSecond ����ʾ��2ȫ���Ĵ��ھ��  
  @return  true��ʾȫ�����ɹ�,false��ʾ������һ��ȫ��ʧ��  
*/   
bool CMonitorManager::FullScreenWindow(HWND hWndFirst, HWND hWndSecond)   
{   
    if (!hWndFirst || !IsWindow(hWndFirst) ) return false;   
    if (!hWndSecond || !IsWindow(hWndSecond)) return false;   
   
    RECT rcOld;    
    long lStyle, lExStyle;   
    HWND hFirstParent = 0, hSecondParent = 0, hTempParent;   
    bool bFullMainWnd = true;   
    for (hTempParent = GetParent(hWndFirst); hTempParent; )   
    {   
        hFirstParent = hTempParent; hTempParent = GetParent(hFirstParent);   
        if (hTempParent && (GetWindowLong(hFirstParent, GWL_STYLE)&WS_POPUP))    
        {   
            bFullMainWnd = false;   
        }   
    }   
    for (hTempParent = GetParent(hWndSecond); hTempParent; )   
    {   
        hSecondParent = hTempParent; hTempParent = GetParent(hSecondParent);   
        if (hTempParent && (GetWindowLong(hSecondParent, GWL_STYLE)&WS_POPUP))    
        {   
            bFullMainWnd = false;   
        }   
    }   
    if (hFirstParent && hSecondParent)   
    {   
        if (hSecondParent == hFirstParent && bFullMainWnd)   
        {   
            HWND hWnds[] = {hWndFirst, hWndSecond};   
            for (int i = 0; i < 2; ++i)   
            {   
                GetWindowRect(hWnds[i], &rcOld);   
                ScreenToClient(hFirstParent, rcOld);   
                lStyle = GetWindowLong(hWnds[i], GWL_STYLE);   
                lExStyle = GetWindowLong(hWnds[i], GWL_EXSTYLE);   
                m_map_wnd.insert(make_pair(hWnds[i], WndInfo(lStyle, lExStyle, rcOld, false)));   
            }   
            if (!FullScreenWindow(hFirstParent, true, -1))  return false;   
        }   
    }   
    if (!FullScreenWindow(hWndFirst, true, 1))   return false;   
    if (!FullScreenWindow(hWndSecond, true, 2))  return false;   
    ShowWindow(hWndFirst, SW_SHOWNORMAL); ShowWindow(hWndSecond, SW_SHOWNORMAL);   
    return true;   
}  