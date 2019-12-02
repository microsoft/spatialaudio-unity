// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "SpatialAudioManager.h"
#include "IsacAdapter.h"

// static init
std::unique_ptr<ISpatialAudioAdapter> SpatialAudioManager::s_SpatialAudioAdapter;

void SpatialAudioManager::EnsureInitialized()
{
    if (!s_SpatialAudioAdapter)
    {
        s_SpatialAudioAdapter.reset(new IsacAdapter());
    }
}

std::unique_ptr<ISpatialSource> SpatialAudioManager::GetSpatialSource()
{
    if (s_SpatialAudioAdapter)
    {
        return s_SpatialAudioAdapter->GetSpatialSource();
    }
    return nullptr;
}

uint32_t SpatialAudioManager::Process(float* outputBuffer, uint32_t numSamples, uint32_t numChannels) noexcept
{
    if (s_SpatialAudioAdapter)
    {
        return s_SpatialAudioAdapter->Process(outputBuffer, numSamples, numChannels);
    }
    return 0;
}