#pragma once

#include <AtlBase.h>

#include <atlapp.h>
#include <atlcrack.h>
#include <atltypes.h>
#include <atlwinx.h>

#include <string>
#include <vector>

#include "resource.h"
#include "FloatCom.h"

#define ID_CM_FIRST 1000
#define ID_CM_LAST (ID_CM_FIRST + 100)

class CFloatCom;
struct IDeskBand2;
struct IContextMenu;

typedef CWinTraits<WS_POPUPWINDOW, WS_EX_TOOLWINDOW>		CToolWinTraits;
typedef CWinTraitsOR<WS_THICKFRAME, 0, CToolWinTraits>		CFloatWinTraits;

class CFloatWnd :
    public CWindowImpl<CFloatWnd, CWindowEx, CFloatWinTraits>
{
private:
    static std::vector<CFloatWnd*> s_wnds;
    static int s_n;

public:
    static std::vector<CFloatWnd*>& GetWnds() { return s_wnds; }

    DECLARE_WND_CLASS_EX(NULL, CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, COLOR_WINDOWFRAME)

    CFloatWnd()
    {
        s_wnds.push_back(this);
    }

    ~CFloatWnd()
    {
        s_wnds.erase(std::find(s_wnds.begin(), s_wnds.end(), this));
    }

    BOOL Create(HWND hwndParent, _In_ REFCLSID rclsid);
    BOOL Create(HWND hwndParent, const CRegKey& key, LPCTSTR sid);
    void Save(const CRegKey& key);

    const std::wstring& GetId() const { return  m_id; }

    BEGIN_MSG_MAP(CFloatWnd)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_SIZE(OnSize)
        MSG_WM_GETMINMAXINFO(OnGetMinMaxInfo)
        MSG_WM_TIMER(OnTimer)
        MSG_WM_CONTEXTMENU(OnContextMenu)
        MSG_WM_NCHITTEST(OnNcHitTest)
        MSG_WM_NCLBUTTONDOWN(OnNcLButtonDown)
        MSG_WM_NCMOUSEMOVE(OnNcMouseMove);
        MSG_WM_PARENTNOTIFY(OnParentNotify)
        MSG_WM_INITMENUPOPUP(OnInitMenuPopup)
        MSG_WM_UNINITMENUPOPUP(OnUnInitMenuPopup)
        COMMAND_ID_HANDLER_EX(ID_MAIN_ALWAYSONTOP, OnCmdAlwaysOnTop)
        COMMAND_ID_HANDLER_EX(ID_MAIN_AUTOHIDE, OnCmdAutoHide)
        COMMAND_ID_HANDLER_EX(ID_MAIN_TRANSPARENT, OnCmdTransparent)
        COMMAND_ID_HANDLER_EX(ID_MAIN_CLOSE, OnCmdClose)
        COMMAND_RANGE_HANDLER_EX(ID_CM_FIRST, ID_CM_LAST, OnCmdContextMenu)
    END_MSG_MAP()

private:
    LRESULT OnCreate(LPCREATESTRUCT lpCreateStruct);
    void OnDestroy();
    void OnSize(UINT nType, CSize size);
    void OnGetMinMaxInfo(LPMINMAXINFO lpMMI);
    void OnTimer(UINT_PTR nIDEvent);
    void OnContextMenu(CWindow wnd, CPoint point);
    UINT OnNcHitTest(CPoint point);
    void OnNcLButtonDown(UINT nHitTest, CPoint point);
    void OnNcMouseMove(UINT nHitTest, CPoint point);
    void OnParentNotify(UINT message, UINT nChildID, LPARAM lParam);
    void OnInitMenuPopup(CMenuHandle menuPopup, UINT nIndex, BOOL bSysMenu);
    void OnUnInitMenuPopup(UINT nID, CMenuHandle menu);
    void OnCmdAlwaysOnTop(UINT uNotifyCode, int nID, CWindow wndCtl);
    void OnCmdAutoHide(UINT uNotifyCode, int nID, CWindow wndCtl);
    void OnCmdTransparent(UINT uNotifyCode, int nID, CWindow wndCtl);
    void OnCmdClose(UINT uNotifyCode, int nID, CWindow wndCtl);
    void OnCmdContextMenu(UINT uNotifyCode, int nID, CWindow wndCtl);

    virtual void OnFinalMessage(_In_ HWND hWnd) override;

    BOOL IsAlwaysOnTop() const { return GetWindowLong(GWL_EXSTYLE) & WS_EX_TOPMOST ? TRUE : FALSE; }
    BOOL IsTransparent() const { return GetWindowLong(GWL_EXSTYLE) & WS_EX_LAYERED ? TRUE : FALSE; }
    BOOL IsThisOrChild(HWND hWnd) const
    {
        return hWnd == m_hWnd || IsChild(hWnd);
    }

    BOOL IsRolledUp() const
    {
        CRect rc;
        GetClientRect(&rc);
        return rc.Height() <= 0;
    }

    BOOL m_bRollinUp = FALSE;
    std::wstring m_id;
    CLSID m_clsid = {};
    DESKBANDINFO m_bi = {};
    CComPtr<CFloatCom> m_pCom;
    CComPtr<IDeskBand2> m_pDB;
    CComPtr<IContextMenu> m_pContextMenu;
    BOOL m_bAutoHide = FALSE;
    CRect m_rcWindow;
};
