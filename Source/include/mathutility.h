// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
#pragma once

#include <cmath>
#include <algorithm>

inline float AmplitudeToDb(float amplitude)
{
    return 20.0f * std::log10(amplitude);
}

inline float DbToAmplitude(float dB)
{
    return std::pow(10.0f, dB / 20.0f);
}

inline float Clamp(float value, float floor, float cap) noexcept
{
    return std::min(std::max(value, floor), cap);
}

inline bool IsPowerOfTwo(int n)
{
    return (n & (n - 1)) == 0;
}

template <typename T>
int Sgn(T val)
{
    return (T(0) < val) - (val < T(0));
}
