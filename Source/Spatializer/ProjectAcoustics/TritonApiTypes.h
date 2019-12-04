// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
//! \file TritonApiTypes.h
//! \brief Types to be used with Triton

#pragma once
#include "AcousticsSharedTypes.h"

//! An object that holds the parameters returned from QueryAcoustics calls, summarizing
//! the acoustics between dynamic source and listener location.
//! All directional information is located in Triton's canonincal coordinate system.
//! The following image shows Triton's coordinate system, as well as the directions for
//! the ReflLoudnessDB Channels in the TritonAcousticParameters struct.
//! Note that since Triton computes propagation in world coordinates,
//! all its directions are locked to the world, not the listener's head.
//! This means that the user's head rotation must be applied on top of these parameters
//! to reproduce the acoustics. This is the job of the spatializer, assumed to be a separate component.
//! \image html TritonCoordinates.png
typedef struct TritonAcousticParameters
{
    //! Special value that indicates failure to compute a parameter.
    //! Far outside of the normal range of parameter values.
    static constexpr float FailureCode = -1e10f;

    //! The delay, in seconds, that the dry sound from the audio source undergoes to arrive
    //! from source to listener, possibly detouring around intervening scene geometry.
    float DirectDelay;
    //! The loudness of the sound source, in dB, that arrives in the first 10ms to the listener.
    //! Meant to be applied as an additional gain on the dry sound.
    //! By design, this value does not account for distance attenuation, only providing the
    //! addition loss due to obstructions. It layers transparently on top of any
    //! designed distance attenuation model you might be using for a given sound.
    float DirectLoudnessDB;
    //! The azimuth, in degrees, of the direction that the dry sound is propagating towards, ranging from 0 to 360
    //! degrees. Azimuth is zero along the positive X world axis, 90 degrees along positive Y, and so on. Note that the
    //! direction the sound is coming from is the negative of this direction. Will respond to intervening environmental
    //! features like portals or obstructions.
    float DirectAzimuth;
    //! The elevation, in degrees, of the direction that the dry sound is propagating towards, ranging from 0 to 180
    //! degrees. Elevation is zero along the positive Z world axis, 90 degrees on the XY plane, and 180 along negative Z
    //! world axis. Note that the direction the sound is coming from is the negative of this direction. Will respond to
    //! intervening environmental features like portals or obstructions.
    float DirectElevation;

    //! The delay, in seconds, after the dry sound that the first reflection arrives.
    float ReflectionsDelay;
    //! The total loudness, in dB, of the reflections in the first 80ms after reflections start arriving.
    //! Note: This parameter is provided for convenience. Its energy is always the sum of energies in the
    //! directional reflection channels below.
    float ReflectionsLoudnessDB;

    //! Directional reflections loudness, in dB, incoming from the positive Z world direction
    float ReflLoudnessDB_Channel_0;
    //! Directional reflections loudness, in dB,  incoming from the positive X world direction
    float ReflLoudnessDB_Channel_1;
    //! Directional reflections loudness, in dB,  incoming from the positive Y world direction
    float ReflLoudnessDB_Channel_2;
    //! Directional reflections loudness, in dB,  incoming from the negative X world direction
    float ReflLoudnessDB_Channel_3;
    //! Directional reflections loudness, in dB,  incoming from the negative Y world direction
    float ReflLoudnessDB_Channel_4;
    //! Directional reflections loudness, in dB,  incoming from the negative Z world direction
    float ReflLoudnessDB_Channel_5;

    //! The time, in seconds, it would take for the reflections to decay by 60dB.
    //! This is computed by taking the decay rate of reflected energy and extrapolating the time it would take to
    //! achieve 60dB decay.
    float EarlyDecayTime;
    //! The time, in seconds, it would take for the late reverberation following reflections to decay by 60dB
    float ReverbTime;
} TritonAcousticParameters;

//! An object that contains information useful for debugging Acoustics
//! In addition to TritonAcousticParameters, it contains additional metadata about the source
typedef struct TritonAcousticParametersDebug
{
    //! A unique identifier for the sound source
    int SourceId;
    //! The position of the source, in Triton coordinates
    ATKVectorF SourcePosition;
    //! The position of the listener, in Triton coordinates
    ATKVectorF ListenerPosition;
    //! Measure of outdoorness at listener location. 0 being completely indoors, 1 being completely outdoors
    float Outdoorness;
    //! The acoustic parameters returned from the most recent call to QueryAcoustics for the source
    TritonAcousticParameters AcousticParameters;
} TritonAcousticParametersDebug;

//! Internal load status of a probe
enum TritonProbeLoadState
{
    //! The probe loaded successfully
    Loaded,
    //! The probe is not currently loaded
    NotLoaded,
    //! Loading the probe failed
    LoadFailed,
    //! The probe is still being loaded
    LoadInProgress,
    //! There is no probe
    DoesNotExist,
    //! The probe is invalid
    Invalid
};

//! An object containing debug metadata for a probe
typedef struct TritonProbeMetadataDebug
{
    //! Current loading state of this probe
    TritonProbeLoadState State;

    //! World location of this probe
    ATKVectorF Location;

    //! Minimum corner of the cubical region around this probe for which it has data
    ATKVectorF DataMinCorner;
    //! Maximum corner of the cubical region around this probe for which it has data
    ATKVectorF DataMaxCorner;
} TritonProbeMetadataDebug;
