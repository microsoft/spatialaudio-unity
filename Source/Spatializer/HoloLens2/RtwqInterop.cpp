// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "RtwqInterop.h"
#include "IsacAdapter.h"

using namespace Microsoft::WRL;

RtwqInterop::RtwqInterop(IsacAdapter* owner) : m_Owner(owner), m_DoWorkKey(0), m_DoWorkResult(nullptr)
{
    // Setup MFWorkQueue
    // Throwing here causes Unity to unload our plugin, and get some debug spew if a debugger is attached
    THROW_IF_FAILED(MFStartup(MF_VERSION));
    m_BufferCompletionEvent.create();
    DWORD taskId = 0;
    THROW_IF_FAILED(MFLockSharedWorkQueue(L"Audio", 0, &taskId, &m_QueueId));
    m_DoWorkCallback = Make<AsyncCallback<RtwqInterop>>(this, &RtwqInterop::OnDoWork, m_QueueId);
    THROW_IF_FAILED(MFCreateAsyncResult(nullptr, m_DoWorkCallback.Get(), nullptr, &m_DoWorkResult));
}

RtwqInterop::~RtwqInterop()
{
    MFUnlockWorkQueue(m_QueueId);
    MFShutdown();
}

HRESULT RtwqInterop::OnDoWork(IMFAsyncResult* result)
{
    RETURN_IF_FAILED_EXPECTED(result->GetStatus());
    LOG_IF_FAILED(m_Owner->SpatialAudioClientWorker());
    RETURN_IF_FAILED(MFPutWaitingWorkItem(m_BufferCompletionEvent.get(), 0, m_DoWorkResult, &m_DoWorkKey));
    return S_OK;
}

HRESULT RtwqInterop::Start() noexcept
{
    RETURN_IF_FAILED(MFPutWaitingWorkItem(m_BufferCompletionEvent.get(), 0, m_DoWorkResult, &m_DoWorkKey));
    return S_OK;
}

HRESULT RtwqInterop::Stop() noexcept
{
    RETURN_HR_IF(S_OK, m_DoWorkKey == 0);

    RETURN_IF_FAILED(MFCancelWorkItem(m_DoWorkKey));
    RETURN_IF_FAILED(m_DoWorkResult->SetStatus(E_ABORT));
    m_DoWorkKey = 0;
    return S_OK;
}

HANDLE RtwqInterop::GetEventHandle() noexcept
{
    return m_BufferCompletionEvent.get();
}