// Copyright (c) Microsoft Corporation.  All rights reserved.

#pragma once

#include <memory>
#include "AlignedAllocator.h"
#include <atomic>

class CircularBuffer
{
public:
    CircularBuffer() = default;
    CircularBuffer(uint32_t bufferSizeInFrames, uint32_t channels);
    CircularBuffer(CircularBuffer&& other) = default;
    CircularBuffer(const CircularBuffer& other) = delete;
    CircularBuffer& operator=(CircularBuffer&& other) = default;
    CircularBuffer& operator=(const CircularBuffer& other) = delete;

    void ReadSamples(_Out_writes_(samplesToRead) float* destinationBuffer, uint32_t samplesToRead);
    void WriteSamples(_In_reads_(samplesToWrite) float* sourceBuffer, uint32_t samplesToWrite);
    void WriteSamples(_In_reads_(samplesToWrite) float* sourceBuffer, uint32_t samplesToWrite, uint32_t stride);
    void DropSamples(uint32_t samplesToDrop);
    uint32_t BufferedFrames();
    uint32_t BufferedSamples();

private:
    std::unique_ptr<float[], AlignedStore::AlignedFree> m_AudioData;
    uint32_t m_ReadPos;
    uint32_t m_WritePos;
    uint32_t m_BufferedSamples;
    uint32_t m_BufferSize;
    uint32_t m_Channels;
};
