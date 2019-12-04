// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "CircularBuffer.h"
#include <cstring>

CircularBuffer::CircularBuffer(uint32_t bufferSizeInFrames, uint32_t channels)
    : m_AudioData(AlignedStore::AlignedAlloc<float>(static_cast<size_t>(bufferSizeInFrames) * channels))
    , m_ReadPos(0)
    , m_WritePos(0)
    , m_BufferedSamples(0)
    , m_BufferSize(bufferSizeInFrames * channels)
    , m_Channels(channels)
{
}

void CircularBuffer::ReadSamples(float* destinationBuffer, uint32_t samplesToRead)
{
    // Determine if we will underflow
    auto bufferedSamplesToRead = samplesToRead;
    auto zerosToRead = 0u;
    if (samplesToRead > BufferedSamples())
    {
        bufferedSamplesToRead = BufferedSamples();
        zerosToRead = samplesToRead - bufferedSamplesToRead;
    }

    // Can we copy memory in one or two parts?
    if (bufferedSamplesToRead > 0)
    {
        if (m_ReadPos + bufferedSamplesToRead < m_BufferSize)
        {
            memcpy(destinationBuffer, &m_AudioData[m_ReadPos], bufferedSamplesToRead * sizeof(float));
            m_ReadPos += bufferedSamplesToRead;
        }
        else
        {
            memcpy(destinationBuffer, &m_AudioData[m_ReadPos], (m_BufferSize - m_ReadPos) * sizeof(float));
            uint32_t remainingSamplesToRead = bufferedSamplesToRead - (m_BufferSize - m_ReadPos);
            memcpy(
                &destinationBuffer[m_BufferSize - m_ReadPos],
                m_AudioData.get(),
                remainingSamplesToRead * sizeof(float));
            m_ReadPos = remainingSamplesToRead;
        }
        m_BufferedSamples -= bufferedSamplesToRead;
    }

    // Write zeros due to underflow
    if (zerosToRead > 0)
    {
        memset(destinationBuffer + bufferedSamplesToRead, 0, zerosToRead * sizeof(float));
    }
}

void CircularBuffer::WriteSamples(float* sourceBuffer, uint32_t samplesToWrite)
{
    this->WriteSamples(sourceBuffer, samplesToWrite, 0u);
}

void CircularBuffer::WriteSamples(float* sourceBuffer, uint32_t samplesToWrite, uint32_t stride)
{
    // Figure out if we're about to buffer overrun
    auto bufferedSamples = m_BufferedSamples;
    if (bufferedSamples + samplesToWrite > m_BufferSize)
    {
        // We are about to overrun. Drop some samples to make room
        DropSamples(samplesToWrite);
    }

    // Can we copy memory in one or two parts?
    if (m_WritePos + samplesToWrite < m_BufferSize)
    {
        if (stride == 0u)
        {
            memcpy(&m_AudioData[m_WritePos], sourceBuffer, samplesToWrite * sizeof(float));
        }
        else
        {
            for (auto sample = 0u; sample < samplesToWrite; ++sample)
            {
                m_AudioData[static_cast<size_t>(m_WritePos) + sample] = sourceBuffer[static_cast<size_t>(sample) * stride];
            }
        }
        m_WritePos += samplesToWrite;
    }
    else
    {
        auto firstBatchLength = m_BufferSize - m_WritePos;
        if (stride == 0u)
        {
            memcpy(&m_AudioData[m_WritePos], sourceBuffer, firstBatchLength * sizeof(float));
        }
        else
        {
            for (auto sample = 0u; sample < firstBatchLength; ++sample)
            {
                m_AudioData[static_cast<size_t>(m_WritePos) + sample] = sourceBuffer[static_cast<size_t>(sample) * stride];
            }
        }
        auto remainingSamplesToStore = samplesToWrite - (m_BufferSize - m_WritePos);
        if (stride == 0u)
        {
            memcpy(
                m_AudioData.get(), &sourceBuffer[m_BufferSize - m_WritePos], remainingSamplesToStore * sizeof(float));
        }
        else
        {
            for (auto sample = 0u; sample < remainingSamplesToStore; ++sample)
            {
                m_AudioData[sample] = sourceBuffer[(m_BufferSize - m_WritePos + sample) * stride];
            }
        }
        m_WritePos = remainingSamplesToStore;
    }
    m_BufferedSamples += samplesToWrite;
}

void CircularBuffer::DropSamples(uint32_t samplesToDrop)
{
    // Can we skip samples in one or two parts?
    if (m_ReadPos + samplesToDrop < m_BufferSize)
    {
        m_ReadPos += samplesToDrop;
    }
    else
    {
        uint32_t remainingSamplesToRead = samplesToDrop - (m_BufferSize - m_ReadPos);
        m_ReadPos = remainingSamplesToRead;
    }

    m_BufferedSamples -= samplesToDrop;
}

uint32_t CircularBuffer::BufferedFrames()
{
    return m_BufferedSamples / m_Channels;
}

uint32_t CircularBuffer::BufferedSamples()
{
    return m_BufferedSamples;
}
