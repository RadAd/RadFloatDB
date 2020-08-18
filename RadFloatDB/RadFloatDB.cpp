#define _ATL_APARTMENT_THREADED

#include "MainWnd.h"

class CModule : public ATL::CAtlExeModuleT< CModule >
{
public:
    HRESULT PreMessageLoop(_In_ int nShowCmd) throw()
    {
        //AtlInitCommonControls(0xFFFF);

        HWND hWnd = FindWindow(CMainWnd::GetWndClassInfo().m_wc.lpszClassName, nullptr);
        if (hWnd != NULL)
        {
            AtlMessageBox(NULL, _T("Rad FLoat DB already running."), IDS_APP_TITLE);
            return S_FALSE;
        }

        CMainWnd* mainwnd = new CMainWnd();
        if (!mainwnd->Create(NULL))
            return AtlHresultFromWin32(GetLastError());
        //mainwnd->ShowWindow(nShowCmd);

        HRESULT hr = S_OK;
        hr = __super::PreMessageLoop(nShowCmd);
        if (FAILED(hr))
            return hr;

        return S_OK;
    }
};

CModule _AtlModule;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    return _AtlModule.WinMain(nCmdShow);
}
