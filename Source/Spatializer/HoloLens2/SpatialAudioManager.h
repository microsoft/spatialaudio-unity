// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once
#include <memory>

struct Direction
{
    float x;
    float y;
    float z;
};

struct SpatialSourceParameters
{
    Direction PrimaryArrivalDirection;
    float PrimaryArrivalDistancePowerDb;
};

struct ISpatialSource
{
    virtual ~ISpatialSource() = default;
    virtual bool SetParameters(SpatialSourceParameters* params) noexcept = 0;
    virtual float* GetBuffer() noexcept = 0;
    virtual void ReleaseBuffer(uint32_t samplesWritten) noexcept = 0;
    virtual uint32_t GetIndex() const noexcept = 0;
};

struct ISpatialAudioAdapter
{
    virtual ~ISpatialAudioAdapter() = default;
    virtual std::unique_ptr<ISpatialSource> GetSpatialSource() = 0;
    virtual uint32_t Process(float* outputBuffer, uint32_t numSamples, uint32_t numChannels) noexcept = 0;
};

class SpatialAudioManager final
{
public:
    // Adapter-related methods
    static void EnsureInitialized();
    static std::unique_ptr<ISpatialSource> GetSpatialSource();
    static uint32_t Process(float* outputBuffer, uint32_t numSamples, uint32_t numChannels) noexcept;

private:
    static std::unique_ptr<ISpatialAudioAdapter> s_SpatialAudioAdapter;
};