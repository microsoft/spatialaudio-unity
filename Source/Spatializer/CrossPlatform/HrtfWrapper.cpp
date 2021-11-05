// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "HelperMacros.h"
#include "HrtfWrapper.h"
#include <exception>
#include <cstring>

// Statics
std::unique_ptr<HrtfWrapper> HrtfWrapper::s_HrtfWrapper;

HrtfWrapper::SourceInfo::SourceInfo(uint32_t index, HrtfInputBuffer* const sourceBuffer)
    : m_SourceIndex(index), m_SourceBuffer(sourceBuffer)
{
}

HrtfWrapper::SourceInfo::~SourceInfo()
{
    m_SourceBuffer->Buffer = nullptr;
    m_SourceBuffer->Length = 0;
    if (HrtfWrapper::s_HrtfWrapper)
    {
        s_HrtfWrapper->ReleaseSource(m_SourceIndex);
    }
}

void HrtfWrapper::InitWrapper()
{
    RETURN_RESULT_IF_NULL(HrtfWrapper::s_HrtfWrapper);
    HrtfWrapper::s_HrtfWrapper.reset(new HrtfWrapper());
}

bool HrtfWrapper::SourceInfo::SetParameters(HrtfAcousticParameters* params) const noexcept
{
    RETURN_RESULT_IF_NULL(HrtfWrapper::s_HrtfWrapper, false);
    return HrtfWrapper::s_HrtfWrapper->SetParameters(m_SourceIndex, params);
}

float* HrtfWrapper::SourceInfo::GetBuffer() const noexcept
{
    return m_SourceBuffer->Buffer;
}

uint32_t HrtfWrapper::SourceInfo::GetIndex() const noexcept
{
    return m_SourceIndex;
}

HrtfWrapper::SourceInfo* HrtfWrapper::GetHrtfSource()
{
    RETURN_RESULT_IF_NULL(HrtfWrapper::s_HrtfWrapper, nullptr);
    return HrtfWrapper::s_HrtfWrapper->GetAvailableHrtfSource();
}

uint32_t HrtfWrapper::Process(float* outputBuffer, uint32_t numSamples, uint32_t numChannels) noexcept
{
    RETURN_RESULT_IF_NULL(HrtfWrapper::s_HrtfWrapper, 0);
    return HrtfWrapper::s_HrtfWrapper->ProcessHrtfs(outputBuffer, numSamples, numChannels);
}

HrtfWrapper::HrtfWrapper()
    : m_GlobalReverbPower(0), m_GlobalReverbTime(1), m_SampleBuffers(c_HrtfMaxSources, c_HrtfFrameCount)
{
    for (auto&& buffer : m_HrtfInputBuffers)
    {
        buffer.Buffer = nullptr;
        buffer.Length = 0;
    }

    THROW_IF_FALSE(
        HrtfEngineInitialize(c_HrtfMaxSources, HrtfEngineType_Flex, c_HrtfFrameCount, &m_FlexEngine), std::bad_alloc);

    THROW_IF_FALSE(
        HrtfEngineSetOutputFormat(m_FlexEngine.Get(), HrtfOutputFormat_Stereo),
        std::runtime_error,
        "Failure setting output format");
}

HrtfWrapper::SourceInfo* HrtfWrapper::GetAvailableHrtfSource()
{
    for (auto i = 0u; i < c_HrtfMaxSources; i++)
    {
        // If this buffer is not in use and we can get the control interface, return this index
        // Otherwise, keep trying the next indices
        if (m_HrtfInputBuffers[i].Buffer == nullptr)
        {
            if (HrtfEngineAcquireResourcesForSource(m_FlexEngine.Get(), i))
            {
                std::memset(m_SampleBuffers[i].Data, 0, c_HrtfFrameCount * sizeof(float));
                m_HrtfInputBuffers[i].Buffer = m_SampleBuffers[i].Data;
                m_HrtfInputBuffers[i].Length = c_HrtfFrameCount;
                return new HrtfWrapper::SourceInfo(i, &m_HrtfInputBuffers[i]);
            }
        }
    }

    return nullptr;
}

void HrtfWrapper::ReleaseSource(uint32_t sourceIndex)
{
    HrtfEngineReleaseResourcesForSource(m_FlexEngine.Get(), sourceIndex);
}

uint32_t HrtfWrapper::ProcessHrtfs(float* outputBuffer, uint32_t numSamples, uint32_t numChannels) noexcept
{
    auto retVal = HrtfEngineProcess(
        m_FlexEngine.Get(), m_HrtfInputBuffers, c_HrtfMaxSources, outputBuffer, numSamples * numChannels);

    // We've consumed all the audio data for this pass. Clear out the input buffers
    for (auto i = 0u; i < c_HrtfMaxSources; ++i)
    {
        memset(m_SampleBuffers[i].Data, 0, c_HrtfFrameCount * sizeof(float));
    }

    return retVal;
}

bool HrtfWrapper::SetParameters(uint32_t index, HrtfAcousticParameters* params) noexcept
{
    return HrtfEngineSetParametersForSource(m_FlexEngine.Get(), index, params);
}

void ResetSources(ObjectHandle engine, const HrtfInputBuffer* buffers, uint32_t numBuffers)
{
    for (auto source = 0u; source < numBuffers; ++source)
    {
        if (buffers[source].Buffer)
        {
            HrtfEngineResetSource(engine, source);
        }
    }
}

void HrtfWrapper::SetGlobalReverbPowerAdjustment(float power)
{
    RETURN_RESULT_IF_NULL(HrtfWrapper::s_HrtfWrapper);
    HrtfWrapper::s_HrtfWrapper->m_GlobalReverbPower = power;
}

float HrtfWrapper::GetGlobalReverbPowerAdjustment()
{
    RETURN_RESULT_IF_NULL(HrtfWrapper::s_HrtfWrapper, 0);
    return HrtfWrapper::s_HrtfWrapper->m_GlobalReverbPower;
}

void HrtfWrapper::SetGlobalReverbTimeAdjustment(float time)
{
    RETURN_RESULT_IF_NULL(HrtfWrapper::s_HrtfWrapper);
    HrtfWrapper::s_HrtfWrapper->m_GlobalReverbTime = time;
}

float HrtfWrapper::GetGlobalReverbTimeAdjustment()
{
    RETURN_RESULT_IF_NULL(HrtfWrapper::s_HrtfWrapper, 0);
    return s_HrtfWrapper->m_GlobalReverbTime;
}