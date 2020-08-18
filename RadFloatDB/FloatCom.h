#pragma once

#include <ShObjIdl.h>
#include <ShlObj_core.h>

class CFloatCom :
    public CComObjectRootEx<ATL::CComSingleThreadModel>,
    public IDockingWindowSite,
    public IInputObjectSite
{
    BEGIN_COM_MAP(CFloatCom)
        COM_INTERFACE_ENTRY_IID(IID_IUnknown, IOleWindow)
        COM_INTERFACE_ENTRY(IOleWindow)
        COM_INTERFACE_ENTRY(IDockingWindowSite)
        COM_INTERFACE_ENTRY(IInputObjectSite)
    END_COM_MAP()

public:
    void Init(HWND hWnd)
    {
        m_hWnd = hWnd;
    }

    // IOleWindow

    STDMETHOD(GetWindow)(
        /* [out] */ __RPC__deref_out_opt HWND* phwnd)
    {
        if (!phwnd)
            return E_INVALIDARG;

        *phwnd = m_hWnd;
        return S_OK;
    }

    STDMETHOD(ContextSensitiveHelp)(
        /* [in] */ BOOL fEnterMode)
    {
        return E_NOTIMPL;
    }

    // IDockingWindowSite

    STDMETHOD(GetBorderDW)(
        _In_ IUnknown* punkObj, _Out_ RECT* prcBorder)
    {
        return E_NOTIMPL;
    }

    STDMETHOD(RequestBorderSpaceDW)(
        _In_ IUnknown* punkObj, _In_ LPCBORDERWIDTHS pbw)
    {
        return S_OK;
    }

    STDMETHOD(SetBorderSpaceDW)(
        _In_ IUnknown* punkObj, _In_ LPCBORDERWIDTHS pbw)
    {
        return S_OK;
    }

    // IInputObjectSite

    STDMETHOD(OnFocusChangeIS)(
        /* [unique][in] */ __RPC__in_opt IUnknown* punkObj,
        /* [in] */ BOOL fSetFocus)
    {
        return E_NOTIMPL;
    }

private:
    HWND m_hWnd = NULL;
};
