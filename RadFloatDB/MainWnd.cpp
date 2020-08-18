#include "MainWnd.h"

#include "resource.h"
#include "FloatWnd.h"

#include <iterator>
#include <set>

namespace
{
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
}

#define REG_FLOATDB _T("Software\\RadSoft\\RadFloatDB")

TCHAR CMainWnd::s_Title[] = _T("");

LPCTSTR CMainWnd::GetWndCaption()
{
    static int temp = AtlLoadString(IDS_APP_TITLE, s_Title, ARRAYSIZE(s_Title));
    return s_Title;
}

HRESULT CMainWnd::CreateToolbar(REFCLSID clsid)
{
    CFloatWnd* wnd = new CFloatWnd();

    if (!wnd->Create(m_hWnd, clsid))
    {
        delete wnd;
        return AtlHresultFromWin32(GetLastError());
    }

    wnd->ShowWindow(SW_SHOW);

    return S_OK;
}

HRESULT CMainWnd::Load()
{
    CRegKey key;
    if (key.Open(HKEY_CURRENT_USER, REG_FLOATDB, KEY_READ) == ERROR_SUCCESS)
    {
        int i = 0;
        TCHAR name[1024];
        DWORD nlen;
        while (nlen = ARRAYSIZE(name), key.EnumKey(i++, name, &nlen) == ERROR_SUCCESS)
        {
            CFloatWnd* wnd = new CFloatWnd();

            if (!wnd->Create(m_hWnd, key, name))
            {
                delete wnd;
                return AtlHresultFromWin32(GetLastError());
            }

            wnd->ShowWindow(SW_SHOW);
        }
    }

    return S_OK;
}

void CMainWnd::Save()
{
    CRegKey key;
    if (key.Create(HKEY_CURRENT_USER, REG_FLOATDB) == ERROR_SUCCESS)
    {
        std::set<std::wstring> ids;

        for (auto i : CFloatWnd::GetWnds())
        {
            ids.insert(i->GetId());
        }

        int i = 0;
        TCHAR name[1024];
        DWORD nlen;
        while (nlen = ARRAYSIZE(name), key.EnumKey(i++, name, &nlen) == ERROR_SUCCESS)
        {
            if (ids.find(name) == ids.end())
            {
                key.RecurseDeleteKey(name);
            }
        }

        for (auto i : CFloatWnd::GetWnds())
        {
            i->Save(key);
        }
    }
}

LRESULT CMainWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    HRESULT hr = S_OK;

    {
        CRegKey clsid;
        if (clsid.Open(HKEY_CLASSES_ROOT, _T("CLSID"), KEY_READ) == ERROR_SUCCESS)
        {
            int i = 0;
            TCHAR name[1024];
            DWORD nlen;
            while (nlen = ARRAYSIZE(name), clsid.EnumKey(i++, name, &nlen) == ERROR_SUCCESS)
            {
                CRegKey item;
                if (item.Open(clsid, name, KEY_READ) == ERROR_SUCCESS)
                {
                    CRegKey cat;
                    if (cat.Open(item, _T("Implemented Categories\\{00021492-0000-0000-C000-000000000046}"), KEY_READ) == ERROR_SUCCESS) // CATID_DeskBand
                    {
                        TCHAR value[1024];
                        ULONG vlen = ARRAYSIZE(value);
                        item.QueryStringValue(nullptr, value, &vlen);

                        CLSID   clsid;
                        hr = CLSIDFromString(name, &clsid);
                        ATLASSERT(hr == S_OK);

                        m_toolbars.insert(std::map<std::wstring, CLSID>::value_type(value, clsid));
                    }
                }
            }
        }
    }

    Load();

    return 0;
}

void CMainWnd::OnClose()
{
    Save();
    SetMsgHandled(FALSE);
}

void CMainWnd::OnContextMenu(CWindow wnd, CPoint point)
{
    CMenu topmenu(AtlLoadMenu(IDR_MAIN));
    CMenuHandle menu(topmenu.GetSubMenu(0));

    int pos = MenuFind(menu, ID_MAIN_PLACEHOLDER);
    ATLASSERT(pos >= 0);
    menu.DeleteMenu(pos, MF_BYPOSITION);

    UINT j = 0;
    for (const auto& i : m_toolbars)
    {
        ATLVERIFY(menu.InsertMenuW(pos++, MF_BYPOSITION, static_cast<UINT_PTR>(ID_TBFIRST) + j++, i.first.c_str()));
    }

    ATLVERIFY(menu.TrackPopupMenu(TPM_TOPALIGN | TPM_LEFTALIGN, point.x, point.y, m_hWnd));
}

LRESULT CMainWnd::OnToolbar(WORD code, WORD id, HWND lParam)
{
    auto it = std::next(m_toolbars.begin(), id - ID_TBFIRST);
    CreateToolbar(it->second);
    return 0;
}

void CMainWnd::OnCmdClose(UINT uNotifyCode, int nID, CWindow wndCtl)
{
    SendMessage(WM_CLOSE);
}

void CMainWnd::OnFinalMessage(_In_ HWND /*hWnd*/)
{
    PostQuitMessage(0);
    delete this;
}
