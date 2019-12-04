// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
#pragma once

#include <mfapi.h>
#include <wil/resource.h>
#include <winrt/Windows.Media.Devices.h>
#include <wrl.h>

MIDL_INTERFACE("C3855696-201C-4324-A970-BD84B52CEEB9")
IRtwqInterop : public IUnknown
{
public:
    virtual HRESULT Start() noexcept = 0;
    virtual HRESULT Stop() noexcept = 0;
    virtual HANDLE GetEventHandle() noexcept = 0;
};

class IsacAdapter;

template <class T>
class AsyncCallback : public Microsoft::WRL::RuntimeClass<
                          Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::RuntimeClassType::ClassicCom>,
                          Microsoft::WRL::FtmBase, IMFAsyncCallback>
{
public:
    typedef HRESULT (T::*InvokeFn)(IMFAsyncResult* result);

    AsyncCallback(T* parent, InvokeFn fn, DWORD queueId) : m_Parent(parent), m_InvokeFn(fn), m_QueueId(queueId)
    {
    }

    // IMFAsyncCallback methods
    STDMETHODIMP GetParameters(DWORD* flags, DWORD* queueId)
    {
        *flags = 0;
        *queueId = m_QueueId;
        return S_OK;
    }

    STDMETHODIMP Invoke(IMFAsyncResult* result)
    {
        return (m_Parent->*m_InvokeFn)(result);
    }

private:
    T* m_Parent;
    InvokeFn m_InvokeFn;
    DWORD m_QueueId;
};

class RtwqInterop : public Microsoft::WRL::RuntimeClass<
                        Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::RuntimeClassType::ClassicCom>,
                        Microsoft::WRL::FtmBase, IRtwqInterop>
{
public:
    RtwqInterop(IsacAdapter* owner);
    virtual ~RtwqInterop();

    virtual HRESULT Start() noexcept override;
    virtual HRESULT Stop() noexcept override;
    virtual HANDLE GetEventHandle() noexcept override;

private:
    HRESULT OnDoWork(IMFAsyncResult* result);

private:
    Microsoft::WRL::ComPtr<AsyncCallback<RtwqInterop>> m_DoWorkCallback;
    IMFAsyncResult* m_DoWorkResult;
    MFWORKITEM_KEY m_DoWorkKey;
    IsacAdapter* m_Owner;
    wil::unique_event m_BufferCompletionEvent;
    DWORD m_QueueId;
};
