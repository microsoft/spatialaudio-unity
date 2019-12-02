// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
#pragma once

#include <atomic>
#include <mfapi.h>
#include <mmdeviceapi.h>
#include <SpatialAudioClient.h>
#include <wrl.h>
#include <winrt/Windows.Media.Devices.h>

#include "CircularBuffer.h"
#include "HrtfConstants.h"
#include "RtwqInterop.h"
#include "SpatialAudioManager.h"

class IsacAdapter;

// ISAC needs to know a bunch of internal state about the source object while doing processing
// Because of this, we can't directly return the source object directly to the user of the IsacAdapter
// Instead, ISAC holds onto all the real logic inside this Internal class, and returns a shadow object
// back to the caller. The shadow object calls directly into this object to service all API calls
// When the shadow object is deleted, it calls into this object via MarkForDeletion. On the next ISAC
// processing pass, the ISAC worker will see that this object is ready for deletion and will destroy it
//
// Note that this class does implement ISpatialSource, but is intentionally not inheriting from ISpatialSource
// This prevents accidentally returning this class outside the scope of IsacAdapter
class IsacSpatialSourceInternal final : private ISpatialSource
{
public:
    IsacSpatialSourceInternal(uint32_t index, IsacAdapter* owner, ISpatialAudioObject* audioObject);
    virtual ~IsacSpatialSourceInternal() = default;

    // ISpatialSource interface
    bool SetParameters(SpatialSourceParameters* params) noexcept override;
    float* GetBuffer() noexcept override;
    void ReleaseBuffer(uint32_t samplesWritten) noexcept override;
    uint32_t GetIndex() const noexcept override;

    ISpatialAudioObject* GetSpatialAudioObject();
    void SetSpatialAudioObject(ISpatialAudioObject* object);
    SpatialSourceParameters GetParameters();
    void ReadSamplesFromCircularBuffer(float* buffer, uint32_t numSamples);
    bool AreEnoughSamplesBuffered(uint32_t requiredSamples);
    void ClearBuffering();
    void MarkForDeletion();
    bool IsActive() const;

private:
    SpatialSourceParameters m_Params;
    CircularBuffer m_Buffer;
    float m_frameBuffer[c_HrtfFrameCount];
    uint32_t m_Index;
    IsacAdapter* m_Owner;
    Microsoft::WRL::ComPtr<ISpatialAudioObject> m_AudioObject;
    bool m_PreRolled;
    std::atomic<bool> m_IsActive;
};

// This is the shadow object returned to the user of IsacAdapter, mentioned above
class IsacSpatialSourcePublic final : public ISpatialSource
{
public:
    IsacSpatialSourcePublic(IsacSpatialSourceInternal* impl);
    virtual ~IsacSpatialSourcePublic();

    // ISpatialSource interface
    virtual bool SetParameters(SpatialSourceParameters* params) noexcept override;
    virtual float* GetBuffer() noexcept override;
    virtual void ReleaseBuffer(uint32_t samplesWritten) noexcept override;
    virtual uint32_t GetIndex() const noexcept override;

private:
    IsacSpatialSourceInternal* m_SpatialSource;
};