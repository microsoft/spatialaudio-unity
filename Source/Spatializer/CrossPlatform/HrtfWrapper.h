// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
#pragma once

#include "AlignedBuffers.h"
#include "HrtfApi.h"
#include "HrtfConstants.h"
#include <memory>

class HrtfWrapper final
{
public:
    class SourceInfo final
    {
    public:
        SourceInfo(uint32_t index, HrtfInputBuffer* const sourceBuffer);
        ~SourceInfo();

        bool SetParameters(HrtfAcousticParameters* params) const noexcept;
        float* GetBuffer() const noexcept;
        uint32_t GetIndex() const noexcept;

    private:
        const uint32_t m_SourceIndex;
        HrtfInputBuffer* const m_SourceBuffer;
    };

    HrtfWrapper();
    ~HrtfWrapper() = default;

    static void InitWrapper();
    static SourceInfo* GetHrtfSource();
    static uint32_t Process(float* outputBuffer, uint32_t numSamples, uint32_t numChannels) noexcept;
    static void SetGlobalReverbPowerAdjustment(float power);
    static float GetGlobalReverbPowerAdjustment();
    static void SetGlobalReverbTimeAdjustment(float time);
    static float GetGlobalReverbTimeAdjustment();

    friend class SourceInfo;

private:
    // Methods
    SourceInfo* GetAvailableHrtfSource();
    void ReleaseSource(uint32_t sourceIndex);
    uint32_t ProcessHrtfs(float* outputBuffer, uint32_t numSamples, uint32_t numChannels) noexcept;
    bool SetParameters(uint32_t index, HrtfAcousticParameters* params) noexcept;

    // Data
    static std::unique_ptr<HrtfWrapper> s_HrtfWrapper;

    AlignedStore::AlignedBuffers<float> m_SampleBuffers;
    HrtfInputBuffer m_HrtfInputBuffers[c_HrtfMaxSources];
    HrtfEngineHandle m_FlexEngine;

    float m_GlobalReverbPower;
    float m_GlobalReverbTime;
};
