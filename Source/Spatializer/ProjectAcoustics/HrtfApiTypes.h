// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
//! \file HrtfApiTypes.h
//! \brief Types to be used with the HrtfEngine

#pragma once
#include <stdint.h>
#include "AcousticsSharedTypes.h"

//! A list of gain values for each frequency band
typedef struct
{
    //! Gain in dB for band centered at 250Hz
    float g_250HzDb;
    //! Gain in dB for band centered at 500Hz
    float g_500HzDb;
    //! Gain in dB for band centered at 1kHz
    float g_1kHzDb;
    //! Gain in dB for band centered at 2kHz
    float g_2kHzDb;
    //! Gain in dB for band centered at 4kHz
    float g_4kHzDb;
    //! Gain in dB for band centered at 8kHz
    float g_8kHzDb;
    //! Gain in dB for band centered at 16kHz
    float g_16kHzDb;
} FrequencyBandGainsDb;
//! Define the number of bands
#define HRTF_NUM_FREQUENCY_BANDS (sizeof(FrequencyBandGainsDb) / sizeof(float))

//! A container for audio data that will be processed by the HrtfEngine
typedef struct HrtfInputBuffer
{
    //! Pointer to the input audio buffer. Audio must be 32bit float, PCM, mono, 48KHz
    float* Buffer;
    //! Length of the audio buffer. Must be >= 1024 samples
    uint32_t Length;
} HrtfInputBuffer;

//! Perceptual description of the listener's experience of a single audio source
typedef struct
{
    //! The effective source-to-listener distance, potentially including both physics and user input
    float EffectiveSourceDistance;

    //! The direction that a sound source should be perceived as coming from
    VectorF PrimaryArrivalDirection;
    //! The gain on the primary arrival direction caused by scene geometry
    float PrimaryArrivalGeometryPowerDb;
    //! The gain on the primary arrival direction caused by propagation distance
    float PrimaryArrivalDistancePowerDb;

    //! The direction of the fully occluded sound source. If not desired, set to 0,0,0
    VectorF SecondaryArrivalDirection;
    //! The gain on the secondary arrival direction caused by scene geometry
    float SecondaryArrivalGeometryPowerDb;
    //! The gain on the secondary arrival direction caused by propagation distance
    float SecondaryArrivalDistancePowerDb;

    //! Loudness of early room reflections
    float EarlyReflectionsPowerDb;
    //! Time required for the early room reflections to decay by 60dB
    float EarlyReflections60DbDecaySeconds;
    //! Time required for the late reverberation to decay by 60dB
    float LateReverb60DbDecaySeconds;
    //! A measure of the extent to which the current listener is outdoors. [0,1] 0 meaning indoors, 1 meaning outdoors
    float Outdoorness;
} HrtfAcousticParameters;

//! Method of spatialization
enum HrtfEngineType
{
    //! Use HRTF-based binaural processing for spatialization
    HrtfEngineType_Binaural = 0,
    //! Use VBAP-panning for multi-channel spatialization
    HrtfEngineType_Panner,
    //! Only do reverb - does not render direct path at all
    HrtfEngineType_ReverbOnly,
    //! Only do panning - no reverb at all
    HrtfEngineType_PannerOnly
};

//! Output channel format for spatialization
enum HrtfOutputFormat
{
    //! Single-channel mixdown
    HrtfOutputFormat_Mono = 0,
    //! Stereo mix-down
    HrtfOutputFormat_Stereo,
    //! Quadraphonic 4.0 loudspeaker locations
    HrtfOutputFormat_Quad,
    //! Standard 5.0 loudspeaker locations (no LFE)
    HrtfOutputFormat_5,
    //! Dolby standard 5.1 loudspeaker locations
    HrtfOutputFormat_5dot1,
    //! Dolby standard 7.1 loudspeaker locations
    HrtfOutputFormat_7dot1,
    //! Total number of formats; can be used to represent 'unknown' or 'unsupported' format
    HrtfOutputFormat_Count
};
