// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
#pragma once

#include <atomic>
#include <mfapi.h>
#include <mmdeviceapi.h>
#include <SpatialAudioClient.h>
#include <wil/resource.h>
#include <winrt/Windows.Media.Devices.h>
#include <wrl.h>

#include "CircularBuffer.h"
#include "HrtfConstants.h"
#include "IsacSpatialSource.h"
#include "RtwqInterop.h"
#include "SpatialAudioManager.h"

class IsacAdapter final : public ISpatialAudioAdapter
{
public:
    IsacAdapter();
    virtual ~IsacAdapter();

    // ISpatialAudioAdapter inteface
    virtual std::unique_ptr<ISpatialSource> GetSpatialSource() override;
    virtual uint32_t Process(float* outputBuffer, uint32_t numSamples, uint32_t numChannels) noexcept override;
    HRESULT SpatialAudioClientWorker();

    static bool AreEnoughSamplesBuffered(uint32_t bufferedSamples, uint32_t requiredSamples, bool prerolled);

private:
    HRESULT ActivateIsacInterface(winrt::hstring* activatedDeviceId);
    HRESULT Activate();
    HRESULT ActivateSpatialAudioStream(const WAVEFORMATEX& objectFormat, UINT32 maxObjects);
    HRESULT InitializeRtwq();
    void Reset();
    HRESULT HandleDeviceChange(winrt::hstring newDeviceId);
    void PruneStaleSources();

private:
    std::vector<std::unique_ptr<IsacSpatialSourceInternal>> m_Sources;
    Microsoft::WRL::ComPtr<ISpatialAudioClient> m_Isac;
    Microsoft::WRL::ComPtr<ISpatialAudioObjectRenderStream> m_SpatialAudioStream;
    winrt::hstring m_DeviceIdInUse;
    bool m_IsPlaying;
    bool m_IsActivated;
    uint32_t m_IsacEventsSinceLastUnityTick;
    winrt::event_token m_DeviceChangeToken;
    Microsoft::WRL::ComPtr<IRtwqInterop> m_RtwqInterop;
};

class IsacActivator : public Microsoft::WRL::RuntimeClass<
                          Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::RuntimeClassType::ClassicCom>,
                          Microsoft::WRL::FtmBase, IActivateAudioInterfaceCompletionHandler>
{
public:
    IsacActivator();
    virtual ~IsacActivator() = default;

    // IActivateAudioInterfaceCompletionHandler
    STDMETHOD(ActivateCompleted)(IActivateAudioInterfaceAsyncOperation* operation);

    // Other
    STDMETHOD(Wait)(DWORD Timeout);
    STDMETHOD(GetActivateResult)(ISpatialAudioClient** deviceAccess);

private:
    wil::unique_handle m_CompletedEvent;
    HRESULT m_ActivateResult;
    Microsoft::WRL::ComPtr<ISpatialAudioClient> m_Isac;
};
