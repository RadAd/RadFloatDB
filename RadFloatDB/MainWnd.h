#pragma once

#include <AtlBase.h>

#include <atlapp.h>
#include <atlcrack.h>
#include <atltypes.h>

#include <map>
#include <string>

#include "resource.h"
#include "TaskBarIcon.h"

#define ID_TBFIRST  UINT(1000)

class CMainWnd :
    public CWindowImpl<CMainWnd, CWindow, CFrameWinTraits>
{
private:
    static TCHAR s_Title[100];

public:
    static ATL::CWndClassInfo& GetWndClassInfo()
    {
	    static ATL::CWndClassInfo wc =
	    {
		    { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, StartWindowProc,
		      0, 0, NULL, AtlLoadIcon(IDI_RADFLOATDB), NULL, (HBRUSH)(COLOR_WINDOW + 1), NULL, _T("RADFLOATDB"), NULL },
		    NULL, NULL, IDC_ARROW, TRUE, 0, _T("")
	    };
	    return wc;
    }

    static LPCTSTR GetWndCaption();

    BEGIN_MSG_MAP(CMainWnd)
        CHAIN_MSG_MAP_MEMBER(m_taskicon)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_CLOSE(OnClose)
        MSG_WM_CONTEXTMENU(OnContextMenu)
        COMMAND_RANGE_HANDLER_EX(ID_TBFIRST, ID_TBFIRST + 100, OnToolbar)
        COMMAND_ID_HANDLER_EX(ID_MAIN_CLOSE, OnCmdClose)
    END_MSG_MAP()

    HRESULT Load();
    void Save();

private:
    LRESULT OnCreate(LPCREATESTRUCT lpCreateStruct);
    void OnClose();
    void OnContextMenu(CWindow wnd, CPoint point);
    LRESULT OnToolbar(WORD code, WORD id, HWND lParam);
    void OnCmdClose(UINT uNotifyCode, int nID, CWindow wndCtl);

    virtual void OnFinalMessage(_In_ HWND hWnd) override;

    HRESULT CreateToolbar(REFCLSID clsid);

    std::map<std::wstring, CLSID> m_toolbars;

    CTaskBarIcon m_taskicon;
};
