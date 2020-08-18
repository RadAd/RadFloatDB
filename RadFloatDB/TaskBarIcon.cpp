#include "TaskBarIcon.h"

LRESULT CTaskBarIcon::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    SetMsgHandled(FALSE);
    AddIcon();
    return 0;
}

void CTaskBarIcon::OnDestroy()
{
    SetMsgHandled(FALSE);
    DeleteIcon();
}

void CTaskBarIcon::AddIcon()
{
    NOTIFYICONDATA tnid = { sizeof(NOTIFYICONDATA) };
    tnid.hWnd = m_hWnd;
    tnid.uID = m_uID;
    tnid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    tnid.uCallbackMessage = MYWM_NOTIFYICON;
    tnid.hIcon = GetIcon();
    if (tnid.hIcon == NULL)
        tnid.hIcon = (HICON)GetClassLongPtr(m_hWnd, GCLP_HICONSM);
    if (tnid.hIcon == NULL)
        tnid.hIcon = (HICON)GetClassLongPtr(m_hWnd, GCLP_HICON);
    GetWindowText(tnid.szTip, ARRAYSIZE(tnid.szTip));
    Shell_NotifyIcon(NIM_ADD, &tnid);
}

void CTaskBarIcon::DeleteIcon()
{

    NOTIFYICONDATA tnid = { sizeof(NOTIFYICONDATA) };
    tnid.hWnd = m_hWnd;
    tnid.uID = m_uID;
    Shell_NotifyIcon(NIM_DELETE, &tnid);
}

LRESULT CTaskBarIcon::OnNotifyIcon(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UINT uID = (UINT)wParam;
    if (uID == m_uID)
    {
        if (lParam == WM_RBUTTONUP)
        {
            SetForegroundWindow(m_hWnd);
            SendMessage(WM_CONTEXTMENU, (WPARAM)m_hWnd, GetMessagePos());
        }
    }
    else
        SetMsgHandled(FALSE);
    return 0;
}
