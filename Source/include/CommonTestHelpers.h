// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
#pragma once

#include <cmath>
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include "mmreg.h"

#define HRTF_2PI 6.28318530f

inline bool AreFloatsTooFarApart(float one, float two, float zeroTolerance = 10E-6, float relativeTolerance = 10E-4)
{
    if (one == 0)
    {
        if (std::fabs(two) > zeroTolerance)
        {
            return true;
        }
    }
    else if (std::fabs((one - two) / one) > relativeTolerance)
    {
        return true;
    }
    return false;
}

// Checks absolute difference between one and two
inline bool CheckEqual(float one, float two, float tolerance)
{
    return std::fabs(one - two) < tolerance;
}

inline void CheckBufferDifference(
    const float* reference, const float* result, uint32_t length, float threshold, uint32_t allowedCount,
    std::ostream* output = nullptr)
{
    auto errorCount = 0u;
    for (auto sample = 0u; sample < length; ++sample)
    {
        if (abs(reference[sample] - result[sample]) > threshold)
        {
            std::cout << std::scientific << "Exceeded threshold of " << threshold << " at buffer sample " << sample
                      << " with reference " << reference[sample] << " and result " << result[sample] << std::endl;
            errorCount++;
        }
        if (output)
        {
            std::ostream& out = *output;
            out << std::scientific << result[sample] << " " << reference[sample] << std::endl;
        }
    }
    ASSERT_LE(errorCount, allowedCount);
}

inline void FillPcmFormat(WAVEFORMATEX* pFormat, uint16_t wChannels, int nSampleRate, uint16_t wBits)
{
    pFormat->wFormatTag = wBits == 32 ? WAVE_FORMAT_IEEE_FLOAT : WAVE_FORMAT_PCM;
    pFormat->nChannels = wChannels;
    pFormat->nSamplesPerSec = nSampleRate;
    pFormat->wBitsPerSample = wBits;
    pFormat->nBlockAlign = pFormat->nChannels * (pFormat->wBitsPerSample / 8);
    pFormat->nAvgBytesPerSec = pFormat->nSamplesPerSec * pFormat->nBlockAlign;
    pFormat->cbSize = 0;
}

inline bool GenerateSineWave(uint8_t* pbBuffer, size_t cbSize, const WAVEFORMATEX& format, float flFrequency)
{
    if (!pbBuffer || (format.wFormatTag != WAVE_FORMAT_PCM && format.wFormatTag != WAVE_FORMAT_IEEE_FLOAT) ||
        (format.wBitsPerSample != 8 && format.wBitsPerSample != 16 && format.wBitsPerSample != 32) ||
        (format.nChannels != 1 && format.nChannels != 2))
    {
        return false;
    }

    if (format.wFormatTag == WAVE_FORMAT_PCM)
    {
        int nBytesPerSample = format.wBitsPerSample >> 3;
        int amplitude = (nBytesPerSample == 2) ? 0x7FFF : 0x7F;

        // The "angle" used in the function, adjusted for the number of channels and sample rate.
        // The argument to sine is 2*pi*frequency/samplingRate
        auto t = static_cast<float>((HRTF_2PI * flFrequency) / (format.nSamplesPerSec * format.nBlockAlign));
        for (auto i = 0u; i <= cbSize - format.nBlockAlign; i += format.nBlockAlign)
        {
            // Fill with a simple sine wave at max amplitude
            for (int channel = 0; channel < format.nChannels; ++channel)
            {
                int j = i + (channel * nBytesPerSample);
                if (nBytesPerSample == 2)
                {
                    short sample = short(amplitude * sin(t * i));
                    *(pbBuffer + j) = static_cast<uint8_t>(sample & 0x00FF);
                    *(pbBuffer + j + 1) = static_cast<uint8_t>((sample >> 8) & 0x00FF);
                }
                else
                {
                    *(pbBuffer + j) = static_cast<uint8_t>(amplitude * sin(t * i));
                }
            }
        }
    }
    else
    {
        float t = static_cast<float>((HRTF_2PI * flFrequency) / (format.nSamplesPerSec * format.nChannels));
        float* pFloatBuffer = reinterpret_cast<float*>(pbBuffer);
        size_t bufferSize = cbSize / sizeof(float);
        for (auto i = 0u; i < bufferSize; i += format.nChannels)
        {
            // Fill with a simple sine wave at -6dB amplitude
            float sample = 0.57f * sin(t * i);
            for (int channel = 0; channel < format.nChannels; ++channel)
            {
                pFloatBuffer[i + channel] = sample;
            }
        }
    }

    return true;
}

inline float CheckSineWaveFreq(uint32_t sampleRate, uint32_t channels, uint32_t samples, const float* buffer)
{
    // To make sure the result of processing didn't introduce glitches or distortion
    // To do this, we'll look at the time domain signal, and make sure it's mostly smooth
    auto counter = 0;
    uint32_t i = channels * 2;

    // Figure out if the signal is currently increasing or decreasing
    bool increasing = (buffer[i - channels] >= buffer[i - channels * 2]);

    // Now, go through to make sure the sine wave is smooth
    // Typically, at 1000 Hz, the signal changes direction every 22 samples
    // However, it's not perfect. Sometimes the SRC introduces some aliasing
    // This checks roughly what the frequency is, without doing a full FFT
    bool firstTime = true;
    auto peaks = 0u;
    for (; i < samples * channels; i += channels)
    {
        if (increasing)
        {
            if (buffer[i] >= buffer[i - channels])
            {
                counter += 1;
            }
            else
            {
                // If there's some noise in the signal, filter it out. This code checks SRCs, which sometimes alias
                if (counter > 3)
                {
                    peaks += 1;
                }
                firstTime = false;
                counter = 0;
                increasing = false;
            }
        }
        else
        {
            if (buffer[i] <= buffer[i - channels])
            {
                counter += 1;
            }
            else
            {
                // If there's some noise in the signal, filter it out. This code checks SRCs, which sometimes alias
                if (counter > 3)
                {
                    peaks += 1;
                }
                firstTime = false;
                counter = 0;
                increasing = true;
            }
        }
    }

    return (float) sampleRate / (float) samples * (float) peaks / 2;
}

inline bool VerifySineWave(
    uint32_t sampleRate, uint32_t channels, uint32_t samples, const float* buffer, float expectedSineWaveFrequency,
    float* sineWaveFrequency)
{
    *sineWaveFrequency = CheckSineWaveFreq(sampleRate, channels, samples, buffer);

    // Do a basic check of the frequency -- callers of this function can be more strict
    return (abs(expectedSineWaveFrequency - *sineWaveFrequency) < expectedSineWaveFrequency / 10);
}
