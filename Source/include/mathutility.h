// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
#pragma once
#include "MathConstants.h"
#include <cmath>
#include <algorithm>

constexpr float RadianToDeg = 180.0f / static_cast<float>(M_PI);
constexpr float DegToRadian = static_cast<float>(M_PI) / 180.0f;

inline float AmplitudeToDb(float amplitude)
{
    return 20.0f * std::log10(amplitude);
}

inline float DbToAmplitude(float dB)
{
    return std::pow(10.0f, dB / 20.0f);
}

inline float EnergyToDB(float Energy)
{
    return 10.0f * std::log10(Energy);
}

inline float DBToEnergy(float DB)
{
    return std::pow(10.0f, (DB / 10.0f));
}

inline float Clamp(float value, float floor, float cap) noexcept
{
    return std::min(std::max(value, floor), cap);
}

template <class T>
inline T Clamp(T val, T minval, T maxval)
{
    return val < minval ? minval : (val > maxval ? maxval : val);
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

template <typename T>
T FloorTowardsZero(T val)
{
    if (val < 0)
    {
        return -std::floor(-val);
    }
    else
    {
        return std::floor(val);
    }
}

inline double Square(double x)
{
    return x * x;
}

inline float Square(float x)
{
    return x * x;
}

// Conversion from 3D vector to spherical azimuth & elevation coordinates (in degrees).
// Note this uses the Windows coordinate system (x+ right, y+ up, z- forward)
//
// Returns a pair of spherical coordinates (in degrees), with the following ranges:
//   Azimuth . . . . . .   0 to 360* (not containing 360 itself),
//   Elevation . . . . . -90 to  90,
//
inline std::pair<float, float> VectorToSpherical(const VectorF& vec) noexcept
{
    constexpr float eps = 1e-4f;
    const float horizontalLength = sqrtf(vec.x * vec.x + vec.z * vec.z);
    const float azimuthRadians = horizontalLength > eps ? atan2f(-vec.x, -vec.z) : 0.0f;
    const float elevationRadians = (horizontalLength + fabsf(vec.y)) > eps ? atan2f(vec.y, horizontalLength) : 0.0f;

    // 0-360 degrees
    float azimuthDegrees = fmodf(fmodf(azimuthRadians * RadianToDeg, 360.0f) + 360.0f, 360.0f);

    // -90-90 degrees
    const float elevationDegrees = std::clamp(elevationRadians * RadianToDeg, -90.0f, 90.0f);

    return std::make_pair(azimuthDegrees, elevationDegrees);
}

// Conversion from spherical coordinates (in degrees) to 3D vector.
// Note this uses the Windows coordinate system (x+ right, y+ up, z- forward).
inline VectorF SphericalToVector(float azimuthDegrees, float elevationDegrees) noexcept
{
    const float azimuthRadians = azimuthDegrees * DegToRadian;
    const float elevationRadians = elevationDegrees * DegToRadian;

    const float cosEle = cosf(elevationRadians);
    return VectorF({-sinf(azimuthRadians) * cosEle, sinf(elevationRadians), -cosf(azimuthRadians) * cosEle});
}
