// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "IsacAdapter.h"
#include <cstring>
#include "wil/result_macros.h"
#include "mathutility.h"

using namespace Microsoft::WRL;
using namespace winrt::Windows::Media::Devices;

constexpr uint32_t c_RequiredPrerollBuffers = 5;
constexpr uint32_t c_MaximumIsacEventsBetweenUnityTicks = 8;

bool IsacAdapter::AreEnoughSamplesBuffered(uint32_t bufferedSamples, uint32_t requiredSamples, bool prerolled)
{
    if (prerolled)
    {
        return bufferedSamples > requiredSamples;
    }
    else
    {
        // The ISAC pump cadence is 10ms. Unity's audio engine typically runs at 1024 samples (21ms)
        // To prevent glitching, we don't want to start streaming audio data from this source until
        // we have 4x the ISAC cadence buffered up (at least 40ms). This makes sure we don't underrun
        // in the event that the Unity engine is delayed
        return bufferedSamples > requiredSamples * c_RequiredPrerollBuffers;
    }
}

// Warning about the device change lambda not having a virtual dtor is OK to ignore
#pragma warning(disable : 5205)
IsacAdapter::IsacAdapter() : m_IsPlaying(false), m_IsActivated(false), m_IsacEventsSinceLastUnityTick(0)
{
    // Subscribe to device change notifications so we can reactivate later
    auto deviceChangeHandler = [&](winrt::Windows::Foundation::IInspectable const&,
                                   DefaultAudioRenderDeviceChangedEventArgs const& args) {
        return HandleDeviceChange(args.Id());
    };
    m_DeviceChangeToken = MediaDevice::DefaultAudioRenderDeviceChanged(deviceChangeHandler);
}
#pragma warning(default : 5205)

IsacAdapter::~IsacAdapter()
{
    // Unregister from device change notifications
    MediaDevice::DefaultAudioRenderDeviceChanged(m_DeviceChangeToken);
}

// Calls ActivateAudioInterfaceAsync and waits for it to complete
// On success, sets activatedDeviceId to the id of the device that ISAC is activated against
HRESULT IsacAdapter::ActivateIsacInterface(winrt::hstring* activatedDeviceId)
{
    // ActivateAudioInterfaceAsync takes a special activation object.
    // This is our implementation of that object.
    ComPtr<IsacActivator> spCompletionObject = Make<IsacActivator>();
    RETURN_IF_NULL_ALLOC(spCompletionObject);

    ComPtr<IActivateAudioInterfaceCompletionHandler> spCompletionHandler;
    RETURN_IF_FAILED(spCompletionObject.As(&spCompletionHandler));

    auto deviceId = MediaDevice::GetDefaultAudioRenderId(AudioDeviceRole::Default);

    // Activate the ISpatialAudioClient interface and wait for it to complete
    ComPtr<IActivateAudioInterfaceAsyncOperation> spOperation;
    RETURN_IF_FAILED(ActivateAudioInterfaceAsync(
        deviceId.c_str(), __uuidof(ISpatialAudioClient), nullptr, spCompletionHandler.Get(), &spOperation));
    RETURN_IF_FAILED(spCompletionObject->Wait(INFINITE));
    RETURN_IF_FAILED(spCompletionObject->GetActivateResult(&m_Isac));

    *activatedDeviceId = deviceId;
    return S_OK;
}

// Given an activated ISpatialAudioClient interface, finds the supported WAVEFORMATEX that matches our needs
// We only operate in 48kHz float. If that's supported, it is returned. Otherwise, this function fails
HRESULT FindAcceptableWaveformat(ISpatialAudioClient* isac, WAVEFORMATEX* objectFormat)
{
    // Initialize ISAC with its preferred format
    ComPtr<IAudioFormatEnumerator> audioObjectFormatEnumerator;
    RETURN_IF_FAILED(isac->GetSupportedAudioObjectFormatEnumerator(&audioObjectFormatEnumerator));
    UINT32 audioObjectFormatCount = 0;
    RETURN_IF_FAILED(audioObjectFormatEnumerator->GetCount(&audioObjectFormatCount));
    RETURN_HR_IF(E_FAIL, audioObjectFormatCount == 0);

    // Find the first format that's in 48kHz, float. That's what Unity uses
    for (auto i = 0u; i < audioObjectFormatCount; i++)
    {
        wil::unique_cotaskmem_ptr<WAVEFORMATEX> format;
        RETURN_IF_FAILED(audioObjectFormatEnumerator->GetFormat(0, wil::out_param(format)));
        if (format->nSamplesPerSec == c_HrtfSampleRate && format->wFormatTag == WAVE_FORMAT_IEEE_FLOAT)
        {
            *objectFormat = *format;
            break;
        }
    }
    RETURN_HR_IF_NULL(E_NOT_VALID_STATE, objectFormat);

    return S_OK;
}

// Assuming ISAC has been activated, uses that ISAC in conjunction with the passed-in parameters to activate the
// spatial audio stream and associated static bed objects
HRESULT IsacAdapter::ActivateSpatialAudioStream(const WAVEFORMATEX& objectFormat, UINT32 maxObjects)
{
    RETURN_HR_IF_NULL(E_NOT_VALID_STATE, m_Isac);

    // Setup Static Bed Object mask
    AudioObjectType objectMask = AudioObjectType_None;
    SpatialAudioObjectRenderStreamActivationParams activationParams = {
        &objectFormat, objectMask, 0, maxObjects, AudioCategory_GameEffects, m_RtwqInterop->GetEventHandle(), nullptr};

    PROPVARIANT activateParams;
    PropVariantInit(&activateParams);
    activateParams.vt = VT_BLOB;
    activateParams.blob.cbSize = sizeof(activationParams);
    activateParams.blob.pBlobData = reinterpret_cast<BYTE*>(&activationParams);
    RETURN_IF_FAILED(m_Isac->ActivateSpatialAudioStream(&activateParams, IID_PPV_ARGS(&m_SpatialAudioStream)));

    return S_OK;
}

// Registers the ISAC audio thread to run against the real-time work queue
// This elevates the ISAC audio thread to run at priority 21 instead of 11.
HRESULT IsacAdapter::InitializeRtwq()
{
    // We only need to create the RTWQ helper once
    if (m_RtwqInterop == nullptr)
    {
        m_RtwqInterop = Make<RtwqInterop>(this);
        RETURN_IF_NULL_ALLOC(m_RtwqInterop);
    }

    return S_OK;
}

// Fully activates and initializes the ISAC interfaces and all related member variables
HRESULT IsacAdapter::Activate() try
{
    // Nothing to do if already activated
    RETURN_HR_IF(S_OK, m_IsActivated);

    winrt::hstring deviceId;
    RETURN_IF_FAILED(ActivateIsacInterface(&deviceId));

    WAVEFORMATEX objectFormat;
    RETURN_IF_FAILED(FindAcceptableWaveformat(m_Isac.Get(), &objectFormat));

    RETURN_IF_FAILED(InitializeRtwq());

    // Determine how many dynamic objects this platform supports
    // If none, bail out. This will force Unity to fallback to its own panner
    UINT32 maxObjects = 0;
    RETURN_IF_FAILED(m_Isac->GetMaxDynamicObjectCount(&maxObjects));
    RETURN_HR_IF(E_NOT_VALID_STATE, maxObjects == 0);
    RETURN_IF_FAILED(ActivateSpatialAudioStream(objectFormat, maxObjects));

    // Reserve enough spaces in the sources vector for the maximum amount of dynamic sources
    m_Sources.resize(maxObjects);

    // It's important to only set the deviceId after successfully initializing everything
    // Otherwise, we may fail a retry on new device arrival notifications
    m_DeviceIdInUse = deviceId;
    m_IsActivated = true;
    return S_OK;
}
CATCH_RETURN()

// Stops the ISAC stream and drops all queued samples
void IsacAdapter::Reset()
{
    m_IsacEventsSinceLastUnityTick = 0;
    if (m_IsPlaying)
    {
        m_SpatialAudioStream->Stop();
        m_RtwqInterop->Stop();
        m_IsPlaying = false;
    }
}

// Reacts to an audio endpoint change
HRESULT IsacAdapter::HandleDeviceChange(winrt::hstring newDeviceId)
{
    // Don't process this device change request if we're already playing on the new device
    RETURN_HR_IF(S_OK, newDeviceId == m_DeviceIdInUse);

    Reset();
    m_IsActivated = false;

    RETURN_IF_FAILED(Activate());

    // For any active sources, replace their spatial audio object registration with the new ISAC stream
    // Clear out any old samples from before the stream switch
    for (const auto& source : m_Sources)
    {
        if (source)
        {
            ComPtr<ISpatialAudioObject> object;
            RETURN_IF_FAILED(m_SpatialAudioStream->ActivateSpatialAudioObject(AudioObjectType_Dynamic, &object));
            source->SetSpatialAudioObject(object.Get());
            source->ClearBuffering();
        }
    }

    return S_OK;
}

// Returns the next available spatial source. If none are available, returns null
std::unique_ptr<ISpatialSource> IsacAdapter::GetSpatialSource()
{
    if (FAILED(Activate()))
    {
        return nullptr;
    }

    // Available sources will be null in the objects array
    // Activate and return the first available source. If there aren't any available, return null
    for (auto i = 0u; i < m_Sources.size(); i++)
    {
        if (m_Sources[i] == nullptr)
        {
            ComPtr<ISpatialAudioObject> audioObject;
            if (FAILED(m_SpatialAudioStream->ActivateSpatialAudioObject(AudioObjectType_Dynamic, &audioObject)))
            {
                return nullptr;
            }
            m_Sources[i].reset(new IsacSpatialSourceInternal(i, this, audioObject.Get()));
            return std::make_unique<IsacSpatialSourcePublic>(m_Sources[i].get());
        }
    }
    return nullptr;
}

// This function doesn't actually process, but rather starts playback if it was stopped.
// This is because Unity's audio engine doesn't provide callbacks for us to know when to start our pump
uint32_t IsacAdapter::Process(float*, uint32_t, uint32_t) noexcept
{
    m_IsacEventsSinceLastUnityTick = 0;
    if (!m_IsActivated)
    {
        return 0;
    }
    if (!m_IsPlaying)
    {
        m_SpatialAudioStream->Start();
        m_RtwqInterop->Start();
        m_IsPlaying = true;
    }
    return 0;
}

// Called once per ISAC pump pass. Sends new audio data and spatial parameters to the ISpatialAudioObject
// associated with the IsacSpatialSourceInternal
HRESULT UpdateSpatialAudioObject(IsacSpatialSourceInternal* source)
{
    ComPtr<ISpatialAudioObject> audioObject = source->GetSpatialAudioObject();
    RETURN_HR_IF_NULL(E_NOT_VALID_STATE, audioObject);

    BOOL isActive = FALSE;
    RETURN_IF_FAILED(audioObject->IsActive(&isActive));
    RETURN_HR_IF(E_NOT_VALID_STATE, !isActive);

    BYTE* buffer = nullptr;
    UINT32 byteCount = 0;
    RETURN_IF_FAILED(audioObject->GetBuffer(&buffer, &byteCount));

    if (source->AreEnoughSamplesBuffered(byteCount / sizeof(float)))
    {
        auto params = source->GetParameters();
        RETURN_IF_FAILED(audioObject->SetPosition(
            params.PrimaryArrivalDirection.x, params.PrimaryArrivalDirection.y, params.PrimaryArrivalDirection.z));
        RETURN_IF_FAILED(audioObject->SetVolume(DbToAmplitude(params.PrimaryArrivalDistancePowerDb)));
        source->ReadSamplesFromCircularBuffer(reinterpret_cast<float*>(buffer), byteCount / sizeof(float));
    }
    else
    {
        // If we don't have enough samples buffered to send data down to ISAC, but we have an active audio object,
        // fill the audio object's output with silence. This prevents stuttering in the event of a starved voice,
        // or after Unity stops playing
        memset(buffer, 0, byteCount);
    }

    return S_OK;
}

// Remove any sources that have been marked for deletion
void IsacAdapter::PruneStaleSources()
{
    // For each source, if it is a valid source and is not active, remove it
    for (auto i = 0u; i < m_Sources.size(); i++)
    {
        if (m_Sources[i] != nullptr && !m_Sources[i]->IsActive())
        {
            m_Sources[i] = nullptr;
        }
    }
}

// This gets triggered on every pump pass from ISAC/AudioDG
// In general, this happens every 10ms (completely separate from Unity's engine tick)
HRESULT IsacAdapter::SpatialAudioClientWorker()
{
    // See if Unity's audio engine is still playing
    // If ISAC gets too far ahead of Unity, Unity is probably stopped.
    m_IsacEventsSinceLastUnityTick += 1;
    if (m_IsacEventsSinceLastUnityTick > c_MaximumIsacEventsBetweenUnityTicks)
    {
        Reset();
        return S_OK;
    }

    // Sources are asynchronously marked for deletion. Check all sources and remove any that have been marked
    PruneStaleSources();

    // Begin the process of sending object data and metadata
    auto updateCleanup = wil::scope_exit([&]() { m_SpatialAudioStream->EndUpdatingAudioObjects(); });

    UINT32 objects = 0;
    UINT32 frameCount = 0;
    RETURN_IF_FAILED(m_SpatialAudioStream->BeginUpdatingAudioObjects(&objects, &frameCount));

    for (const auto& source : m_Sources)
    {
        if (source != nullptr)
        {
            // Intentionally ignore any per-object failures and continue to the next object
            UpdateSpatialAudioObject(source.get());
        }
    }

    return S_OK;
}

IsacActivator::IsacActivator() : m_ActivateResult(E_ILLEGAL_METHOD_CALL)
{
    m_CompletedEvent.reset(CreateEvent(nullptr, FALSE, FALSE, nullptr));
}

STDMETHODIMP IsacActivator::ActivateCompleted(IActivateAudioInterfaceAsyncOperation* operation)
{
    // The completed event must always be set, even in failure cases
    // wil::scope_exit will always run when the setEvent var falls out of scope
    auto setEvent = wil::scope_exit([&]() { SetEvent(m_CompletedEvent.get()); });
    ComPtr<IUnknown> spAudioAsUnknown;
    RETURN_IF_FAILED(operation->GetActivateResult(&m_ActivateResult, &spAudioAsUnknown));
    RETURN_IF_FAILED(m_ActivateResult);
    m_ActivateResult = spAudioAsUnknown.As(&m_Isac);

    return S_OK;
}

STDMETHODIMP IsacActivator::Wait(DWORD Timeout)
{
    auto waitResult = WaitForSingleObject(m_CompletedEvent.get(), Timeout);
    if (waitResult == WAIT_OBJECT_0)
    {
        return S_OK;
    }
    else if (waitResult == WAIT_TIMEOUT)
    {
        return HRESULT_FROM_WIN32(ERROR_TIMEOUT);
    }
    else if (waitResult == WAIT_FAILED)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    return E_FAIL;
}

STDMETHODIMP IsacActivator::GetActivateResult(ISpatialAudioClient** deviceAccess)
{
    *deviceAccess = nullptr;
    RETURN_IF_FAILED(m_ActivateResult);
    RETURN_IF_FAILED(m_Isac.CopyTo(deviceAccess));
    return S_OK;
}