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
    if (!HrtfWrapper::s_HrtfWrapper)
    {
        return false;
    }
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
    if (!HrtfWrapper::s_HrtfWrapper)
    {
        return nullptr;
    }
    return HrtfWrapper::s_HrtfWrapper->GetAvailableHrtfSource();
}

uint32_t HrtfWrapper::Process(float* outputBuffer, uint32_t numSamples, uint32_t numChannels) noexcept
{
    if (!HrtfWrapper::s_HrtfWrapper)
    {
        return 0;
    }
    return HrtfWrapper::s_HrtfWrapper->ProcessHrtfs(outputBuffer, numSamples, numChannels);
}

HrtfWrapper::HrtfWrapper()
    : m_SampleBuffers(c_HrtfMaxSources, c_HrtfFrameCount)
{

    for (uint32_t i = 0; i < c_HrtfMaxSources; ++i)
    {
        m_HrtfInputBuffers[i].Buffer = nullptr;
        m_HrtfInputBuffers[i].Length = 0;
        // Push onto available slots stack in reverse order, so that index 0 is on top of the stack
        // This doesn't matter for functionality, but will make debugging easier if the active sources
        // start at index 0.
        m_AvailableProcessingSlots.push(static_cast<unsigned char>(c_HrtfMaxSources - i - 1));
    }

    if (!HrtfEngineInitialize(c_HrtfMaxSources, HrtfEngineType_FlexBinaural_High, c_HrtfFrameCount, &m_FlexEngine))
    {
        throw std::bad_alloc();
    } 
}

HrtfWrapper::SourceInfo* HrtfWrapper::GetAvailableHrtfSource()
{
    // Are there any sources available?
    if (!m_AvailableProcessingSlots.empty())
    {
        auto sourceIndex = m_AvailableProcessingSlots.top();
        if (HrtfEngineAcquireResourcesForSource(m_FlexEngine.Get(), sourceIndex))
        {
            m_AvailableProcessingSlots.pop();
            std::memset(m_SampleBuffers[sourceIndex].Data, 0, c_HrtfFrameCount * sizeof(float));
            m_HrtfInputBuffers[sourceIndex].Buffer = m_SampleBuffers[sourceIndex].Data;
            m_HrtfInputBuffers[sourceIndex].Length = c_HrtfFrameCount;
            return new HrtfWrapper::SourceInfo(sourceIndex, &m_HrtfInputBuffers[sourceIndex]);
        }
    }

    return nullptr;
}

void HrtfWrapper::ReleaseSource(uint32_t sourceIndex)
{
    HrtfEngineReleaseResourcesForSource(m_FlexEngine.Get(), sourceIndex);
    m_AvailableProcessingSlots.push(static_cast<unsigned char>(sourceIndex));   
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