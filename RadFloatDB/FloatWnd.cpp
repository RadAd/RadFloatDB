#include "FloatWnd.h"

#include "resource.h"

// TODO Check min/max/step size during resizing

namespace
{
    LPCTSTR REG_CLSID = _T("clsid");
    LPCTSTR REG_X = _T("x");
    LPCTSTR REG_Y = _T("y");
    LPCTSTR REG_WIDTH = _T("width");
    LPCTSTR REG_HEIGHT = _T("height");
    LPCTSTR REG_AUTOHIDE = _T("AutoHide");
    LPCTSTR REG_ALWAYSONTOP = _T("AlwaysOnTop");
    LPCTSTR REG_TRANSPARENT = _T("Transparent");

    template <class T>
    inline T RegGetDWORD(CRegKey& key, _In_opt_z_ LPCTSTR pszValueName, const T& def)
    {
        DWORD temp;
        if (key.QueryDWORDValue(pszValueName, temp) == ERROR_SUCCESS)
            return temp;
        else
            return def;
    }

    inline int MenuFind(HMENU hMenu, UINT wID)
    {
        CMenuHandle menu(hMenu);
        for (int i = 0; i < menu.GetMenuItemCount(); ++i)
        {
            if (menu.GetMenuItemID(i) == wID)
                return i;

        }
        return -1;
    }
};

std::vector<CFloatWnd*> CFloatWnd::s_wnds;
int CFloatWnd::s_n = 0;

BOOL CFloatWnd::Create(HWND hwndParent, _In_ REFCLSID rclsid)
{
    m_clsid = rclsid;

    m_id = _T("Toolbar") + std::to_wstring(s_n++);

    CComObject<CFloatCom>* pCom;
    ATLVERIFY(SUCCEEDED(CComObject<CFloatCom>::CreateInstance(&pCom)));
    m_pCom = pCom;

    ATLENSURE_RETURN_VAL(SUCCEEDED(m_pDB.CoCreateInstance(m_clsid, nullptr, CLSCTX_INPROC_SERVER)), FALSE);

    CRect r;
    if (!__super::Create(hwndParent, &r, _T("")))
        return FALSE;

    return TRUE;
}

BOOL CFloatWnd::Create(HWND hwndParent, const CRegKey& key, LPCTSTR sid)
{
    HRESULT hr = S_OK;

    m_id = sid;

    size_t o = m_id.find_last_not_of(_T("0123456789"));
    int n = std::stoi(m_id.substr(o + 1));
    s_n = max(s_n, n + 1);

    CRegKey tb;
    ATLENSURE_RETURN_VAL(tb.Open(key, m_id.c_str(), KEY_READ) == ERROR_SUCCESS, FALSE);

    TCHAR value[1024];
    ULONG vlen = ARRAYSIZE(value);
    tb.QueryStringValue(REG_CLSID, value, &vlen);

    ATLENSURE_RETURN_VAL(SUCCEEDED(CLSIDFromString(value, &m_clsid)), FALSE);

    CRect r;
    r.left = RegGetDWORD(tb, REG_X, 0);
    r.top = RegGetDWORD(tb, REG_Y, 0);
    r.right = r.left + RegGetDWORD(tb, REG_WIDTH, 50);
    r.bottom = r.top + RegGetDWORD(tb, REG_HEIGHT, 50);

    m_bAutoHide = RegGetDWORD(tb, REG_AUTOHIDE, m_bAutoHide);

    DWORD dwStyle = GetWndStyle(0);
    DWORD dwExStyle = GetWndExStyle(0);
    if (RegGetDWORD(tb, REG_ALWAYSONTOP, FALSE))
        dwExStyle |= WS_EX_TOPMOST;
    if (RegGetDWORD(tb, REG_TRANSPARENT, FALSE))
        dwExStyle |= WS_EX_LAYERED;

    CComObject<CFloatCom>* pCom;
    ATLVERIFY(SUCCEEDED(CComObject<CFloatCom>::CreateInstance(&pCom)));
    m_pCom = pCom;

    ATLENSURE_RETURN_VAL(SUCCEEDED(m_pDB.CoCreateInstance(m_clsid, nullptr, CLSCTX_INPROC_SERVER)), FALSE);

    if (!__super::Create(hwndParent, &r, _T(""), dwStyle, dwExStyle))
        return FALSE;

    return TRUE;
}

void CFloatWnd::Save(const CRegKey& key)
{
    HRESULT hr = S_OK;

    CRegKey tb;
    if (tb.Create(key, m_id.c_str()) == ERROR_SUCCESS)
    {
        TCHAR* s;
        hr = StringFromCLSID(m_clsid, &s);
        ATLASSERT(hr == S_OK);
        tb.SetStringValue(REG_CLSID, s);
        CoTaskMemFree(s);

        CRect r = m_rcWindow;
        //GetWindowRect(&r);
        tb.SetDWORDValue(REG_X, r.left);
        tb.SetDWORDValue(REG_Y, r.top);
        tb.SetDWORDValue(REG_WIDTH, r.Width());
        tb.SetDWORDValue(REG_HEIGHT, r.Height());

        tb.SetDWORDValue(REG_AUTOHIDE, m_bAutoHide);
        tb.SetDWORDValue(REG_ALWAYSONTOP, IsAlwaysOnTop());
        tb.SetDWORDValue(REG_TRANSPARENT, IsTransparent());
    }
}

LRESULT CFloatWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    m_pCom->Init(m_hWnd);

    CComPtr<IObjectWithSite> pObjectWithSite;
    ATLVERIFY(SUCCEEDED(m_pDB->QueryInterface(IID_PPV_ARGS(&pObjectWithSite))));
    ATLVERIFY(SUCCEEDED(pObjectWithSite->SetSite(m_pCom->GetUnknown())));

    m_bi.dwMask = 0x7F;
    ATLVERIFY(SUCCEEDED(m_pDB->GetBandInfo(0, DBIF_VIEWMODE_FLOATING, &m_bi)));

    if (lpCreateStruct->cx == 0 || lpCreateStruct->cy == 0)
    {
        CRect r(0, 0, m_bi.ptActual.x, m_bi.ptActual.y);
        AdjustWindowRectEx(&r, GetWndStyle(0), FALSE, GetWndExStyle(0));

        r.MoveToXY(lpCreateStruct->x, lpCreateStruct->y);
        SetWindowPos(NULL, &r, SWP_NOZORDER | SWP_NOMOVE);
    }

    ATLVERIFY(SUCCEEDED(m_pDB->ShowDW(TRUE)));

    SetTimer(0, 100);
    SetLayeredWindowAttributes(0, 128, LWA_ALPHA);

    return 0;
}

void CFloatWnd::OnDestroy()
{
    ATLVERIFY(SUCCEEDED(m_pDB->CloseDW(0)));
    ATLVERIFY(SUCCEEDED(m_pDB->ShowDW(FALSE)));

    CComPtr<IObjectWithSite> pObjectWithSite;
    ATLVERIFY(SUCCEEDED(m_pDB->QueryInterface(IID_PPV_ARGS(&pObjectWithSite))));
    ATLVERIFY(SUCCEEDED(pObjectWithSite->SetSite(nullptr)));
}

void CFloatWnd::OnSize(UINT nType, CSize size)
{
    RECT rc;
    GetClientRect(&rc);

    if (false)
    {
        ATLVERIFY(SUCCEEDED(m_pDB->ResizeBorderDW(&rc, m_pCom->GetUnknown(), FALSE)));
    }
    else
    {
        HWND hDBWnd;
        ATLVERIFY(SUCCEEDED(m_pDB->GetWindow(&hDBWnd)));

        CWindow dbwnd(hDBWnd);
        dbwnd.MoveWindow(&rc);
    }

    if (!IsRolledUp())
        GetWindowRect(&m_rcWindow);
}

void CFloatWnd::OnGetMinMaxInfo(LPMINMAXINFO lpMMI)
{
    if (m_bRollinUp)
    {
        lpMMI->ptMinTrackSize.y = 0;
    }
    else if (m_bi.dwMask & DBIM_MINSIZE)
    {
        lpMMI->ptMinTrackSize.x = m_bi.ptMinSize.x;
        lpMMI->ptMinTrackSize.y = m_bi.ptMinSize.y;
    }
    if (m_bi.dwMask & DBIM_MAXSIZE)
    {
        if (m_bi.ptMaxSize.x > 0)
            lpMMI->ptMaxTrackSize.x = m_bi.ptMaxSize.x;
        if (m_bi.ptMaxSize.y > 0)
            lpMMI->ptMaxTrackSize.y = m_bi.ptMaxSize.y;
    }
}

void CFloatWnd::OnTimer(UINT_PTR nIDEvent)
{
    if (m_bAutoHide && !IsRolledUp())
    {
        POINT pt;
        GetCursorPos(&pt);
        if (!IsThisOrChild(WindowFromPoint(pt)) && !IsThisOrChild(GetCapture()))
        {
            m_bRollinUp = TRUE;
            GetWindowRect(&m_rcWindow);
            CRect temp(m_rcWindow);
            temp.bottom = temp.top;
            SetWindowPos(NULL, &temp, SWP_NOZORDER);
        }
    }
}

void CFloatWnd::OnContextMenu(CWindow wnd, CPoint point)
{
    CMenu topmenu(AtlLoadMenu(IDR_FLOAT));
    CMenuHandle menu(topmenu.GetSubMenu(0));

    int pos = MenuFind(menu, ID_MAIN_PLACEHOLDER);
    ATLASSERT(pos >= 0);
    menu.DeleteMenu(pos, MF_BYPOSITION);

    if (m_pContextMenu != nullptr || SUCCEEDED(m_pDB->QueryInterface(IID_PPV_ARGS(&m_pContextMenu))))
    {
        m_pContextMenu->QueryContextMenu(menu, pos, ID_CM_FIRST, ID_CM_LAST, CMF_NORMAL);
    }

    ATLVERIFY(menu.TrackPopupMenu(TPM_TOPALIGN | TPM_LEFTALIGN, point.x, point.y, m_hWnd));
}

UINT CFloatWnd::OnNcHitTest(CPoint point)
{
    UINT uHit = static_cast<UINT>(DefWindowProc());
    CRect rc;
    GetWindowRect(&rc);
    switch (uHit)
    {
    case HTTOP: uHit = HTCAPTION; break;
    case HTTOPLEFT: uHit = rc.Height() < 32 ? HTBOTTOMLEFT : HTCAPTION; break;
    case HTTOPRIGHT: uHit = rc.Height() < 32 ? HTBOTTOMRIGHT : HTCAPTION; break;
    }
    return uHit;
}

void CFloatWnd::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
    if (GetKeyState(VK_CONTROL) & 0x8000)
        SendMessage(WM_SYSCOMMAND, SC_MOVE | 0x0002, 0);
    else
        SetMsgHandled(FALSE);
}

void CFloatWnd::OnNcMouseMove(UINT nHitTest, CPoint point)
{
    if (IsRolledUp())
    {
        m_bRollinUp = FALSE;
        SetWindowPos(NULL, &m_rcWindow, SWP_NOZORDER);
    }
    SetMsgHandled(FALSE);
}

void CFloatWnd::OnParentNotify(UINT message, UINT nChildID, LPARAM lParam)
{
    switch (message)
    {
    case WM_LBUTTONDOWN:
    {   // TODO Not working
        if (GetKeyState(VK_CONTROL) & 0x8000)
            SendMessage(WM_SYSCOMMAND, SC_MOVE | 0x0002, 0);
        else
            SetMsgHandled(FALSE);
    }
    break;
    }
}

void CFloatWnd::OnInitMenuPopup(CMenuHandle menuPopup, UINT nIndex, BOOL bSysMenu)
{
    menuPopup.CheckMenuItem(ID_MAIN_ALWAYSONTOP, MF_BYCOMMAND | (IsAlwaysOnTop() ? MF_CHECKED : MF_UNCHECKED));
    menuPopup.CheckMenuItem(ID_MAIN_AUTOHIDE, MF_BYCOMMAND | (m_bAutoHide ? MF_CHECKED : MF_UNCHECKED));
    menuPopup.CheckMenuItem(ID_MAIN_TRANSPARENT, MF_BYCOMMAND | (IsTransparent() ? MF_CHECKED : MF_UNCHECKED));
}

void CFloatWnd::OnUnInitMenuPopup(UINT nID, CMenuHandle menu)
{
    m_pContextMenu.Release();
}

void CFloatWnd::OnCmdAlwaysOnTop(UINT uNotifyCode, int nID, CWindow wndCtl)
{
    RECT rc = {};
    SetWindowPos(IsAlwaysOnTop() ? HWND_NOTOPMOST : HWND_TOPMOST, &rc, SWP_NOSIZE | SWP_NOMOVE);
}

void CFloatWnd::OnCmdAutoHide(UINT uNotifyCode, int nID, CWindow wndCtl)
{
    m_bAutoHide = !m_bAutoHide;

    if (!m_bAutoHide && IsRolledUp())
    {
        m_bRollinUp = FALSE;
        SetWindowPos(NULL, &m_rcWindow, SWP_NOZORDER);
    }
}

void CFloatWnd::OnCmdTransparent(UINT uNotifyCode, int nID, CWindow wndCtl)
{
    DWORD dwExStyle = GetWindowLong(GWL_EXSTYLE);
    dwExStyle ^= WS_EX_LAYERED;
    SetWindowLong(GWL_EXSTYLE, dwExStyle);

    if (IsTransparent())
        SetLayeredWindowAttributes(0, 128, LWA_ALPHA);
}

void CFloatWnd::OnCmdClose(UINT uNotifyCode, int nID, CWindow wndCtl)
{
    SendMessage(WM_CLOSE);
}

void CFloatWnd::OnCmdContextMenu(UINT uNotifyCode, int nID, CWindow wndCtl)
{
    if (m_pContextMenu != nullptr)
    {
        CMINVOKECOMMANDINFO ici = { sizeof(CMINVOKECOMMANDINFO) };
        ici.hwnd = m_hWnd;
        ici.nShow = SW_SHOWNORMAL;
        ici.lpVerb = (LPCSTR) MAKEINTRESOURCE(nID - (int) ID_CM_FIRST);
        m_pContextMenu->InvokeCommand(&ici);
    }
}

void CFloatWnd::OnFinalMessage(_In_ HWND /*hWnd*/)
{
    delete this;
}
