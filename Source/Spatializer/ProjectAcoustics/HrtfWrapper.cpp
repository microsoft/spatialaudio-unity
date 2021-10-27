// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

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
    if (!HrtfWrapper::s_HrtfWrapper)
    {
        HrtfWrapper::s_HrtfWrapper.reset(new HrtfWrapper());
    }
}

bool HrtfWrapper::SourceInfo::SetParameters(HrtfAcousticParameters* params) const noexcept
{
    if (HrtfWrapper::s_HrtfWrapper)
    {
        return HrtfWrapper::s_HrtfWrapper->SetParameters(m_SourceIndex, params);
    }
    return false;
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
    return HrtfWrapper::s_HrtfWrapper->GetAvailableHrtfSource();
}

uint32_t HrtfWrapper::Process(float* outputBuffer, uint32_t numSamples, uint32_t numChannels) noexcept
{
    if (HrtfWrapper::s_HrtfWrapper)
    {
        return HrtfWrapper::s_HrtfWrapper->ProcessHrtfs(outputBuffer, numSamples, numChannels);
    }
    return 0;
}

HrtfWrapper::HrtfWrapper()
    : m_GlobalReverbPower(0), m_GlobalReverbTime(1), m_SampleBuffers(c_HrtfMaxSources, c_HrtfFrameCount)
{
    for (auto&& buffer : m_HrtfInputBuffers)
    {
        buffer.Buffer = nullptr;
        buffer.Length = 0;
    }

    bool result = HrtfEngineInitialize(c_HrtfMaxSources, HrtfEngineType_Binaural, c_HrtfFrameCount, &m_BinauralEngine);
    if (!result)
    {
        throw std::bad_alloc();
    }

    result = HrtfEngineInitialize(c_HrtfMaxSources, HrtfEngineType_Panner, c_HrtfFrameCount, &m_PanningEngine);
    if (!result)
    {
        throw std::bad_alloc();
    }

    result = HrtfEngineInitialize(c_HrtfMaxSources, HrtfEngineType_Flex, c_HrtfFrameCount, &m_FlexEngine);
    if (!result)
    {
        throw std::bad_alloc();
    }

    m_ActiveEngine = m_BinauralEngine.Get();
    m_ActiveEngineType = HrtfEngineType_Binaural;
    m_CurrentFormat = HrtfOutputFormat_Stereo;
    m_CurrentFormatSupported = HrtfEngineSetOutputFormat(m_ActiveEngine, m_CurrentFormat);
}

HrtfWrapper::SourceInfo* HrtfWrapper::GetAvailableHrtfSource()
{
    for (auto i = 0u; i < c_HrtfMaxSources; i++)
    {
        // If this buffer is not in use and we can get the control interface, return this index
        // Otherwise, keep trying the next indices
        if (m_HrtfInputBuffers[i].Buffer == nullptr)
        {
            if (HrtfEngineAcquireResourcesForSource(m_BinauralEngine.Get(), i))
            {
                if (HrtfEngineAcquireResourcesForSource(m_PanningEngine.Get(), i))
                {
                    if (HrtfEngineAcquireResourcesForSource(m_FlexEngine.Get(), i))
                    {
                        std::memset(m_SampleBuffers[i].Data, 0, c_HrtfFrameCount * sizeof(float));
                        m_HrtfInputBuffers[i].Buffer = m_SampleBuffers[i].Data;
                        m_HrtfInputBuffers[i].Length = c_HrtfFrameCount;

                        return new HrtfWrapper::SourceInfo(i, &m_HrtfInputBuffers[i]);
                    }
                    else
                    {
                        // If successfully acquired in one engine but failed in the other, release
                        // the acquired resources
                        HrtfEngineReleaseResourcesForSource(m_BinauralEngine.Get(), i);
                    }
                }
            }
        }
    }

    return nullptr;
}

void HrtfWrapper::ReleaseSource(uint32_t sourceIndex)
{
    HrtfEngineReleaseResourcesForSource(m_BinauralEngine.Get(), sourceIndex);
    HrtfEngineReleaseResourcesForSource(m_PanningEngine.Get(), sourceIndex);
    HrtfEngineReleaseResourcesForSource(m_FlexEngine.Get(), sourceIndex);
}

HrtfOutputFormat GetFormatFromChannels(uint32_t numChannels)
{
    if (numChannels == 1)
    {
        return HrtfOutputFormat_Mono;
    }
    else if (numChannels == 2)
    {
        return HrtfOutputFormat_Stereo;
    }
    else if (numChannels == 4)
    {
        return HrtfOutputFormat_Quad;
    }
    else if (numChannels == 5)
    {
        return HrtfOutputFormat_5;
    }
    else if (numChannels == 6)
    {
        return HrtfOutputFormat_5dot1;
    }
    else if (numChannels == 8)
    {
        return HrtfOutputFormat_7dot1;
    }
    return HrtfOutputFormat_Count;
}

uint32_t HrtfWrapper::ProcessHrtfs(float* outputBuffer, uint32_t numSamples, uint32_t numChannels) noexcept
{
    auto thisFormat = GetFormatFromChannels(numChannels);
    if (thisFormat != m_CurrentFormat)
    {
        m_CurrentFormat = thisFormat;
        m_CurrentFormatSupported = HrtfEngineSetOutputFormat(m_ActiveEngine, thisFormat);
    }

    if (!m_CurrentFormatSupported)
    {
        return 0;
    }

    auto retVal =
        HrtfEngineProcess(m_ActiveEngine, m_HrtfInputBuffers, c_HrtfMaxSources, outputBuffer, numSamples * numChannels);

    // We've consumed all the audio data for this pass. Clear out the input buffers
    for (auto i = 0u; i < c_HrtfMaxSources; ++i)
    {
        memset(m_SampleBuffers[i].Data, 0, c_HrtfFrameCount * sizeof(float));
    }

    return retVal;
}

bool HrtfWrapper::SetParameters(uint32_t index, HrtfAcousticParameters* params) noexcept
{
    return HrtfEngineSetParametersForSource(m_ActiveEngine, index, params);
}

void HrtfWrapper::SetActiveEngine(HrtfEngineType engineType) noexcept
{
    if (s_HrtfWrapper)
    {
        s_HrtfWrapper->SetActiveEngineType(engineType);
    }
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

void HrtfWrapper::SetActiveEngineType(HrtfEngineType engineType) noexcept
{
    if (engineType != m_ActiveEngineType)
    {
        if (engineType == HrtfEngineType_Binaural)
        {
            ResetSources(m_BinauralEngine.Get(), m_HrtfInputBuffers, c_HrtfMaxSources);
            m_CurrentFormatSupported = HrtfEngineSetOutputFormat(m_BinauralEngine.Get(), m_CurrentFormat);
            m_ActiveEngine = m_BinauralEngine.Get();
        }
        else if (engineType == HrtfEngineType_Flex)
        {
            ResetSources(m_FlexEngine.Get(), m_HrtfInputBuffers, c_HrtfMaxSources);
            m_CurrentFormatSupported = HrtfEngineSetOutputFormat(m_FlexEngine.Get(), m_CurrentFormat);
            m_ActiveEngine = m_FlexEngine.Get();
        }
        else
        {
            // PanningEngine does not have many per-source resources. It does have per-filter resources,
            // which can only be reset when all sources do.
            HrtfEngineResetAllSources(m_PanningEngine.Get());
            m_CurrentFormatSupported = HrtfEngineSetOutputFormat(m_PanningEngine.Get(), m_CurrentFormat);
            m_ActiveEngine = m_PanningEngine.Get();
        }
        m_ActiveEngineType = engineType;
    }
}

void HrtfWrapper::SetGlobalReverbPowerAdjustment(float power)
{
    if (HrtfWrapper::s_HrtfWrapper)
    {
        s_HrtfWrapper->m_GlobalReverbPower = power;
    }
}
float HrtfWrapper::GetGlobalReverbPowerAdjustment()
{
    if (HrtfWrapper::s_HrtfWrapper)
    {
        return s_HrtfWrapper->m_GlobalReverbPower;
    }
    return 0;
}

void HrtfWrapper::SetGlobalReverbTimeAdjustment(float time)
{
    if (HrtfWrapper::s_HrtfWrapper)
    {
        s_HrtfWrapper->m_GlobalReverbTime = time;
    }
}
float HrtfWrapper::GetGlobalReverbTimeAdjustment()
{
    if (HrtfWrapper::s_HrtfWrapper)
    {
        return s_HrtfWrapper->m_GlobalReverbTime;
    }
    return 0;
}