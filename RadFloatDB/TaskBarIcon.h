#pragma once

#include <AtlBase.h>

#include <atlapp.h>
#include <atlcrack.h>

#define MYWM_NOTIFYICON 2000

class CTaskBarIcon : public CWindow
{
private:
    static UINT s_uTaskbarRestart;

public:
    BOOL ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, DWORD dwMsgMapID = 0)
    {
        HWND hOldWnd = m_hWnd;
        m_hWnd = hWnd;
        BOOL bRet = __ProcessWindowMessage(hWnd, uMsg, wParam, lParam, lResult, dwMsgMapID);
        m_hWnd = hOldWnd;
        return bRet;
    }

#define ProcessWindowMessage __ProcessWindowMessage
    BEGIN_MSG_MAP_EX(CTaskBarIcon)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_DESTROY(OnDestroy)
        MESSAGE_HANDLER_EX(MYWM_NOTIFYICON, OnNotifyIcon)
        MESSAGE_HANDLER(s_uTaskbarRestart, OnTaskbarRestart)
    END_MSG_MAP()
#undef ProcessWindowMessage

    void AddIcon();
    void DeleteIcon();

private:
    LRESULT OnCreate(LPCREATESTRUCT lpCreateStruct);
    void OnDestroy();
    LRESULT OnNotifyIcon(UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnTaskbarRestart(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    UINT m_uID = 0;
};
