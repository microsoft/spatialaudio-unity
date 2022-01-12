// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include <stdint.h>

constexpr float c_DefaultPrimaryArrivalGeometryPowerDb = 0.0f;
constexpr float c_DefaultEarlyReflectionsPowerDb = -100.0f;
constexpr float c_DefaultEarlyReflections60DbDecaySeconds = 0.0f;
constexpr float c_DefaultLateReverb60DbDecaySeconds = 0.0f;
constexpr float c_DefaultOutdoorness = 0.0f;
constexpr uint32_t c_HrtfFrameCount = 1024;
constexpr uint32_t c_HrtfSampleRate = 48000;
constexpr uint32_t c_HrtfMaxSources = 128;
constexpr float c_MinAudibleGain = 0.00002f;   // -94dB
constexpr auto c_MinimumSourceDistance = 0.1f; // In meters