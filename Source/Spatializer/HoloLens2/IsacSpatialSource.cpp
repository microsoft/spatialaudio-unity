// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "IsacSpatialSource.h"
#include "IsacAdapter.h"

constexpr uint32_t c_MaximumSourceBuffers = 4;

IsacSpatialSourceInternal::IsacSpatialSourceInternal(
    uint32_t index, IsacAdapter* owner, ISpatialAudioObject* audioObject)
    : m_Index(index)
    , m_Owner(owner)
    , m_AudioObject(audioObject)
    , m_IsActive(true)
    , m_PreRolled(false)
    // To prevent glitching, we buffer 40ms of data before starting audio playback. To prevent circular buffer overflow,
    // We need to have space for that much data, plus one more source buffer
    // Unity typically runs at c_HrtfFrameCount buffer sizes (21ms), so the circular buffer needs to be at least
    // 3072 samples large to hold enough data without dropping any
    , m_Buffer(c_HrtfFrameCount * c_MaximumSourceBuffers, 1)
{
    // Fail fast if passed a null owner object. This is a bug, and is never valid
    FAIL_FAST_IF(owner == nullptr);

    memset(&m_Params, 0, sizeof(SpatialSourceParameters));
    memset(m_frameBuffer, 0, c_HrtfFrameCount * sizeof(float));
}

void IsacSpatialSourceInternal::MarkForDeletion()
{
    m_IsActive = false;
}

bool IsacSpatialSourceInternal::IsActive() const
{
    return m_IsActive;
}

bool IsacSpatialSourceInternal::SetParameters(SpatialSourceParameters* params) noexcept
{
    m_Params = *params;
    return true;
}

float* IsacSpatialSourceInternal::GetBuffer() noexcept
{
    // A call to GetBuffer means we are processing
    // Since there's no events that trigger processing start/stop,
    // Use this method to tell ISAC that we are still processing data
    m_Owner->Process(nullptr, 0, 0);
    return m_frameBuffer;
}

void IsacSpatialSourceInternal::ReleaseBuffer(uint32_t samplesWritten) noexcept
{
    m_Buffer.WriteSamples(m_frameBuffer, samplesWritten);
}

uint32_t IsacSpatialSourceInternal::GetIndex() const noexcept
{
    return m_Index;
}

ISpatialAudioObject* IsacSpatialSourceInternal::GetSpatialAudioObject()
{
    return m_AudioObject.Get();
}

void IsacSpatialSourceInternal::SetSpatialAudioObject(ISpatialAudioObject* object)
{
    m_AudioObject = object;
}

void IsacSpatialSourceInternal::ClearBuffering()
{
    m_Buffer.DropSamples(m_Buffer.BufferedSamples());
}

void IsacSpatialSourceInternal::ReadSamplesFromCircularBuffer(float* buffer, uint32_t numSamples)
{
    // Check for buffer under-run
    auto bufferedSamps = m_Buffer.BufferedSamples();
    if (numSamples > bufferedSamps)
    {
        numSamples = bufferedSamps;
        m_PreRolled = false;
    }

    m_Buffer.ReadSamples(buffer, numSamples);
}

SpatialSourceParameters IsacSpatialSourceInternal::GetParameters()
{
    return m_Params;
}

bool IsacSpatialSourceInternal::AreEnoughSamplesBuffered(uint32_t requiredSamples)
{
    auto bufferedSamples = m_Buffer.BufferedSamples();
    auto enoughBuffered = IsacAdapter::AreEnoughSamplesBuffered(bufferedSamples, requiredSamples, m_PreRolled);
    // We have enough samples buffered if:
    // - IsacAdapter::AreEnoughSamplesBuffered returned true, which means we definitely have enough samples to do glitch-free
    // playback
    // - It returned false, but we have some samples left. This will glitch, but we should still play out what we have
    if (enoughBuffered || (!enoughBuffered && m_PreRolled && bufferedSamples > 0))
    {
        m_PreRolled = true;
        return true;
    }
    else
    {
        // If we don't have any samples at all, we are completely starved and need to do a full preroll again before
        // continuing
        m_PreRolled = false;
        return false;
    }
}

IsacSpatialSourcePublic::IsacSpatialSourcePublic(IsacSpatialSourceInternal* impl) : m_SpatialSource(impl)
{
    // fail fast if passed a null backing object. This is a bug, and is never valid
    FAIL_FAST_IF(impl == nullptr);
}

IsacSpatialSourcePublic::~IsacSpatialSourcePublic()
{
    m_SpatialSource->MarkForDeletion();
}

bool IsacSpatialSourcePublic::SetParameters(SpatialSourceParameters* params) noexcept
{
    return m_SpatialSource->SetParameters(params);
}

float* IsacSpatialSourcePublic::GetBuffer() noexcept
{
    return m_SpatialSource->GetBuffer();
}

void IsacSpatialSourcePublic::ReleaseBuffer(uint32_t samplesWritten) noexcept
{
    m_SpatialSource->ReleaseBuffer(samplesWritten);
}

uint32_t IsacSpatialSourcePublic::GetIndex() const noexcept
{
    return m_SpatialSource->GetIndex();
}