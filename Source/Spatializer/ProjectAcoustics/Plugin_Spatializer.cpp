// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
// Please note that this will only work on Unity 5.2 or higher.

#include "AudioPluginUtil.h"
#include "HrtfWrapper.h"
#include "TritonWrapper.h"
#include <math.h>
#include <algorithm>
#include <memory>
#include <cstring>
#include "vectormath.h"
#include "mathutility.h"

namespace Spatializer
{
    enum EffectParams
    {
        AdditionalReverbPower,
        DecayTimeScalar,
        EnableTriton,
        OcclusionFactor,
        DistanceWarp,
        TransmissionDb,
        OutdoornessAdjustment,
        Count
    };

    struct EffectData
    {
        std::unique_ptr<HrtfWrapper::SourceInfo> EffectHrtfInfo;
        float SourceDistance;
        float DryDistanceAttenuation;
        float Params[EffectParams::Count];
    };

    int InternalRegisterEffectDefinition(UnityAudioEffectDefinition& definition)
    {
        definition.flags |= UnityAudioEffectDefinitionFlags_IsSpatializer;

        int numparams = EffectParams::Count;
        definition.paramdefs = new UnityAudioParameterDefinition[numparams];
        // Warning: The 'name' value (second argument) below has a strict limit of 15 characters
        RegisterParameter(
            definition,
            "ReverbAdjust",
            "dB",
            -20.0f,
            20.0f,
            0.0f,
            1.0f,
            1.0f,
            EffectParams::AdditionalReverbPower,
            "Reverb Power Adjustment");
        RegisterParameter(
            definition,
            "RT60Scale",
            "",
            0.0f,
            2.0f,
            1.0f,
            1.0f,
            1.0f,
            EffectParams::DecayTimeScalar,
            "Reverb Time Scale Factor");
        RegisterParameter(
            definition,
            "Use Triton",
            "",
            0.0f,
            1.0f,
            1.0f,
            1.0f,
            1.0f,
            EffectParams::EnableTriton,
            "Use Triton Room Acoustics");
        RegisterParameter(
            definition,
            "OcclusionFactor",
            "",
            0.0f,
            c_MaxOcclusionFactor,
            1.0f,
            1.0f,
            1.0f,
            EffectParams::OcclusionFactor,
            "Occlusion Scaling");
        RegisterParameter(
            definition, "DistanceWarp", "", 0.1f, 2.0f, 1.0f, 1.0f, 1.0f, EffectParams::DistanceWarp, "Distance Warp");
        RegisterParameter(
            definition,
            "Transmission",
            "",
            c_MinTransmissionDb * c_MaxOcclusionFactor,
            0.0f,
            c_MinTransmissionDb,
            1.0f,
            1.0f,
            EffectParams::TransmissionDb,
            "Transmission dB");
        RegisterParameter(
            definition,
            "OutdoorAdjust",
            "",
            -1.0f,
            1.0f,
            0.0f,
            1.0f,
            1.0f,
            EffectParams::OutdoornessAdjustment,
            "Outdoorness Adjustment");

        return numparams;
    }

    static UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK DistanceAttenuationCallback(
        UnityAudioEffectState* state, float distanceIn, float attenuationIn, float* attenuationOut)
    {
        // Tell Unity not to apply any attenuation, since we will render the specified attenuation on the dry path
        *attenuationOut = 1.0f;
        if (attenuationIn < c_MinAudibleGain)
        {
            // If source is quiet, tell Unity to mute it
            *attenuationOut = 0.0f;
        }

        // Save off this data so we can use it later
        auto data = state->GetEffectData<EffectData>();
        data->SourceDistance = distanceIn;
        data->DryDistanceAttenuation = attenuationIn;
        return UNITY_AUDIODSP_OK;
    }

    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK CreateCallback(UnityAudioEffectState* state)
    {
        // Assign values to the UnityAudioEffectState
        EffectData* effectdata = new EffectData;
        std::memset(effectdata, 0, sizeof(EffectData));
        state->effectdata = effectdata;
        state->spatializerdata->distanceattenuationcallback = DistanceAttenuationCallback;
        InitParametersFromDefinitions(InternalRegisterEffectDefinition, effectdata->Params);
        effectdata->Params[EffectParams::AdditionalReverbPower] = 0;
        effectdata->Params[EffectParams::DecayTimeScalar] = 1;
        effectdata->Params[EffectParams::EnableTriton] = 1;
        effectdata->Params[EffectParams::OcclusionFactor] = 1;
        effectdata->Params[EffectParams::TransmissionDb] = c_MinTransmissionDb;
        effectdata->Params[EffectParams::OutdoornessAdjustment] = 0;

        HrtfWrapper::InitWrapper();

        effectdata->EffectHrtfInfo.reset(HrtfWrapper::GetHrtfSource());

        return effectdata->EffectHrtfInfo ? UNITY_AUDIODSP_OK : UNITY_AUDIODSP_ERR_UNSUPPORTED;
    }

    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK ReleaseCallback(UnityAudioEffectState* state)
    {
        // Cleanup the effect-local data that was created
        auto data = state->GetEffectData<EffectData>();
        if (data)
        {
            delete data;
            state->effectdata = nullptr;
        }
        return UNITY_AUDIODSP_OK;
    }

    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK
    SetFloatParameterCallback(UnityAudioEffectState* state, int index, float value)
    {
        auto data = state->GetEffectData<EffectData>();
        if (index >= EffectParams::Count)
        {
            return UNITY_AUDIODSP_ERR_UNSUPPORTED;
        }
        data->Params[index] = value;
        return UNITY_AUDIODSP_OK;
    }

    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK
    GetFloatParameterCallback(UnityAudioEffectState* state, int index, float* value, char* valuestr)
    {
        auto data = state->GetEffectData<EffectData>();
        if (index >= EffectParams::Count)
        {
            return UNITY_AUDIODSP_ERR_UNSUPPORTED;
        }
        if (value != nullptr)
        {
            *value = data->Params[index];
        }
        if (valuestr != nullptr)
        {
            // it appears that unity isn't currently supporting this parameter
            valuestr[0] = 0;
        }
        return UNITY_AUDIODSP_OK;
    }

    int UNITY_AUDIODSP_CALLBACK GetFloatBufferCallback(UnityAudioEffectState*, const char*, float*, int)
    {
        return UNITY_AUDIODSP_OK;
    }

    static constexpr float DegToRadian = float(3.14159265358979323846) / 180.0f;
    ATKVectorF PolarToCartesian(const float Azimuth, const float Elevation)
    {
        ATKVectorF retVal;
        const auto el = Elevation * DegToRadian;
        const auto az = Azimuth * DegToRadian;
        retVal.z = cos(el);
        const auto horiz = sin(el);
        retVal.x = horiz * cos(az);
        retVal.y = horiz * sin(az);

        return retVal;
    }

    // Helper function to transform a point vector by a 4x4 matrix specified by the application
    // Sometimes, the world mesh is not centered at the origin, but has some extra transform applied
    // Triton assumes the mesh is centered at the origin.
    inline ATKVectorF ApplyAdditionalWorldTransform(ATKVectorF const& point)
    {
        auto tx = TritonWrapper::GetWorldToLocalTransform();
        ATKVectorF retVal;
        retVal.x = point.x * tx.m11 + point.y * tx.m12 + point.z * tx.m13 + tx.m14;
        retVal.y = point.x * tx.m21 + point.y * tx.m22 + point.z * tx.m23 + tx.m24;
        retVal.z = point.x * tx.m31 + point.y * tx.m32 + point.z * tx.m33 + tx.m34;
        return retVal;
    }

    ATKVectorF CalculateListenerPos(const float* listenerMatrix)
    {
        auto L = listenerMatrix;
        // Note that the listener is the inverse matrix to "make it easy to get the direction vector to the source"
        // This makes it harder for us, as we have to undo that inverse
        ATKVectorF listenerPos;
        listenerPos.x = -(L[0] * L[12] + L[1] * L[13] + L[2] * L[14]);
        listenerPos.y = -(L[4] * L[12] + L[5] * L[13] + L[6] * L[14]);
        listenerPos.z = -(L[8] * L[12] + L[9] * L[13] + L[10] * L[14]);

        // shift the emitter position by the initial world transform specified by the app
        listenerPos = ApplyAdditionalWorldTransform(listenerPos);

        // Apply world-to-Triton Transform
        std::swap(listenerPos.y, listenerPos.z);

        return listenerPos;
    }

    // Calls Triton::QueryAcoustics, and returns if QueryAcoustics succeeded or not
    // S - source matrix
    // L - listener matrix
    // sourceIndex - index of the source in the Hrtf pool. This is used to cache triton results for debugging
    // tritonParamsOut - result of the triton call, if successful
    bool QueryTriton(const float* S, const float* L, const int sourceIndex, TritonAcousticParameters* tritonParamsOut)
    {
        auto useTritonParams = false;

        if (TritonWrapper::IsAceFileLoaded())
        {
            // First, shift the emitter position by the initial world transform specified by the app
            auto emitterPos = ApplyAdditionalWorldTransform({S[12], S[13], S[14]});

            // Now, apply Triton Transform
            // Currently we ignore source orientation and only use the position
            // Triton is in Right-Handed, Z-up space. Unity is in Left-Handed, Y-up space
            // If we were using matrix math, we'd use the following transform matrix
            //{ 1, 0, 0, 0,
            //  0, 0, 1, 0,
            //  0, 1, 0, 0,
            //  0, 0, 0, 1 }
            // Instead, we'll just do the transform directly
            std::swap(emitterPos.y, emitterPos.z);

            // Get the listener position
            ATKVectorF listenerPos = CalculateListenerPos(L);

            useTritonParams = TritonWrapper::QueryAcoustics(emitterPos, listenerPos, sourceIndex, tritonParamsOut);
        }

        return useTritonParams;
    }

    bool QueryOutdoorness(const float* listenerMatrix, float* value)
    {
        // Get the listener position
        ATKVectorF listenerPos = CalculateListenerPos(listenerMatrix);

        return TritonWrapper::GetOutdoornessAtListener(listenerPos, value);
    }

    // Helper function to transform a normal vector by a 4x4 matrix, ignoring projection or translation.
    inline ATKVectorF TransformTritonListenerDirection(ATKVectorF const& normal, const float* const& matrix)
    {
        ATKVectorF retVal;
        retVal.x = normal.x * matrix[0] + normal.y * matrix[4] + normal.z * matrix[8];
        retVal.y = normal.x * matrix[1] + normal.y * matrix[5] + normal.z * matrix[9];
        retVal.z = -(normal.x * matrix[2] + normal.y * matrix[6] + normal.z * matrix[10]);
        return retVal;
    }

    static ATKVectorF ListenerToSourceDirection(const float* sourceMatrix, const float* listenerMatrix)
    {
        const auto* const S = sourceMatrix;
        const auto* const L = listenerMatrix;

        // S[12] = SourcePos.x, S[13] = SourcePos.y, S[14] = SourcePos.z
        auto loc_x = L[0] * S[12] + L[4] * S[13] + L[8] * S[14] + L[12];
        auto loc_y = L[1] * S[12] + L[5] * S[13] + L[9] * S[14] + L[13];
        auto loc_z = L[2] * S[12] + L[6] * S[13] + L[10] * S[14] + L[14];

        return {loc_x, loc_y, loc_z};
    }

    void UpdateAcousticParams(
        const EffectData* data, const float /*spread*/, const float* sourceMatrix, const float* listenerMatrix,
        const TritonAcousticParameters tritonParams)
    {
        if (data->EffectHrtfInfo)
        {
            // Calculate the listener direction from the triton parameters, rotated by the listener orientation
            // Here, we are converting from Triton coordinates to HRTF coordinates
            // Note that triton is Z-up, but HRTF is Y-up, so we need to swap y and z
            auto tritonListenerDirection = PolarToCartesian(tritonParams.DirectAzimuth, tritonParams.DirectElevation);
            std::swap(tritonListenerDirection.y, tritonListenerDirection.z);

            // We now need the inverse transform of the additional world matrix
            auto tx = TritonWrapper::GetLocalToWorldTransform();
            auto x = tritonListenerDirection.x * tx.m11 + tritonListenerDirection.y * tx.m12 +
                     tritonListenerDirection.z * tx.m13;
            auto y = tritonListenerDirection.x * tx.m21 + tritonListenerDirection.y * tx.m22 +
                     tritonListenerDirection.z * tx.m23;
            auto z = tritonListenerDirection.x * tx.m31 + tritonListenerDirection.y * tx.m32 +
                     tritonListenerDirection.z * tx.m33;
            tritonListenerDirection.x = x;
            tritonListenerDirection.y = y;
            tritonListenerDirection.z = z;

            auto listenerDirection = TransformTritonListenerDirection(tritonListenerDirection, listenerMatrix);

            // Unity is left-handed, HRTF is right-handed. Swap here
            listenerDirection.x *= -1;
            listenerDirection.z *= -1;

            // Apply designer control over occlusion dynamics
            float occlusionDbActual = std::max(tritonParams.DirectLoudnessDB, tritonParams.ReflectionsLoudnessDB);
            float obstructionDb = tritonParams.DirectLoudnessDB - occlusionDbActual;
            float wetLevelDb = tritonParams.ReflectionsLoudnessDB - occlusionDbActual;
            float occlusionDb = occlusionDbActual * data->Params[EffectParams::OcclusionFactor];

            // Primary arrival direction
            HrtfAcousticParameters acousticParams = {};
            acousticParams.PrimaryArrivalDirection = listenerDirection;
            acousticParams.PrimaryArrivalGeometryPowerDb = occlusionDb + obstructionDb;
            acousticParams.PrimaryArrivalDistancePowerDb = AmplitudeToDb(data->DryDistanceAttenuation);

            // Secondary arrival models through-the-wall transmission - only enable if not silent
            const auto TransmissionDb = data->Params[EffectParams::TransmissionDb];
            if (TransmissionDb > c_MinTransmissionDb * c_MaxOcclusionFactor)
            {
                acousticParams.SecondaryArrivalDirection = ListenerToSourceDirection(sourceMatrix, listenerMatrix);
                acousticParams.SecondaryArrivalGeometryPowerDb = std::min(
                    TransmissionDb,
                    c_MinTransmissionDb * data->Params[EffectParams::OcclusionFactor] -
                        acousticParams.PrimaryArrivalGeometryPowerDb);
                // Assign secondary arrival same user-designed distance attenuation as primary
                acousticParams.SecondaryArrivalDistancePowerDb = acousticParams.PrimaryArrivalDistancePowerDb;
            }
            else
            {
                // Disable DSP for secondary path
                acousticParams.SecondaryArrivalDirection = {0, 0, 0};
                // Ensure power is floored so overall occlusion dB is correctly computed
                acousticParams.SecondaryArrivalGeometryPowerDb = -120.0f;
                acousticParams.SecondaryArrivalDistancePowerDb = 0;
            }

            // Fill non-directional parameters
            acousticParams.EffectiveSourceDistance = std::max(
                c_MinimumSourceDistance,
                static_cast<float>(pow(data->SourceDistance, data->Params[EffectParams::DistanceWarp])));
            auto drrAdjust =
                acousticParams.PrimaryArrivalDistancePowerDb + AmplitudeToDb(acousticParams.EffectiveSourceDistance);
            acousticParams.EarlyReflectionsPowerDb = occlusionDb + wetLevelDb + drrAdjust +
                                                     data->Params[EffectParams::AdditionalReverbPower] +
                                                     HrtfWrapper::GetGlobalReverbPowerAdjustment();
            acousticParams.EarlyReflections60DbDecaySeconds = tritonParams.EarlyDecayTime *
                                                              data->Params[EffectParams::DecayTimeScalar] *
                                                              HrtfWrapper::GetGlobalReverbTimeAdjustment();
            acousticParams.LateReverb60DbDecaySeconds = tritonParams.ReverbTime *
                                                        data->Params[EffectParams::DecayTimeScalar] *
                                                        HrtfWrapper::GetGlobalReverbTimeAdjustment();
            float tritonOutdoorness = 0;
            QueryOutdoorness(listenerMatrix, &tritonOutdoorness);
            auto adjustedOutdoorness = tritonOutdoorness + data->Params[EffectParams::OutdoornessAdjustment];
            // TODO - Consider mapping values above 0.5 to 1? Task 19193172
            acousticParams.Outdoorness = Clamp(adjustedOutdoorness, 0, 1);

            data->EffectHrtfInfo->SetParameters(&acousticParams);
        }
    }

    // When not using Triton, update acoustic params with default values
    void UpdateAcousticParams(const EffectData* data, const float /*spread*/, const ATKVectorF direction)
    {
        if (data->EffectHrtfInfo)
        {
            HrtfAcousticParameters acousticParams = {};
            acousticParams.PrimaryArrivalDirection = direction;
            acousticParams.PrimaryArrivalGeometryPowerDb = 0.0f;
            acousticParams.PrimaryArrivalDistancePowerDb = AmplitudeToDb(data->DryDistanceAttenuation);
            // Disable DSP for secondary arrival
            acousticParams.SecondaryArrivalDirection = {0, 0, 0};

            acousticParams.EffectiveSourceDistance = data->SourceDistance;

            // Start with default reverb power, then scale by distance and user parameters
            acousticParams.EarlyReflectionsPowerDb =
                c_DefaultEarlyReflectionsPowerDb + AmplitudeToDb(data->DryDistanceAttenuation) +
                data->Params[EffectParams::AdditionalReverbPower] + HrtfWrapper::GetGlobalReverbPowerAdjustment();
            acousticParams.EarlyReflections60DbDecaySeconds = c_DefaultEarlyReflections60DbDecaySeconds *
                                                              data->Params[EffectParams::DecayTimeScalar] *
                                                              HrtfWrapper::GetGlobalReverbTimeAdjustment();
            acousticParams.LateReverb60DbDecaySeconds = c_DefaultLateReverb60DbDecaySeconds *
                                                        data->Params[EffectParams::DecayTimeScalar] *
                                                        HrtfWrapper::GetGlobalReverbTimeAdjustment();

            // Not using Triton, so start with an outdoorness of 0.5 and adjust from there
            auto adjustedOutdoorness = 0.5f + data->Params[EffectParams::OutdoornessAdjustment];
            acousticParams.Outdoorness = Clamp(adjustedOutdoorness, 0, 1);

            data->EffectHrtfInfo->SetParameters(&acousticParams);
        }
    }

    // Both inbuffer and outbuffer are assumed to be stereo, and the same length
    void PrepareAudioData(
        const UnityAudioEffectState* state, const float* inbuffer, float* outbuffer, const unsigned int length,
        int inChannels)
    {
        auto ticksPerHrtfBuffer = c_HrtfFrameCount / state->dspbuffersize;
        auto currentTick = (state->currdsptick / state->dspbuffersize) % ticksPerHrtfBuffer;
        auto offsetIntoHrtfBuffer = currentTick * state->dspbuffersize;

        auto data = state->GetEffectData<EffectData>();
        auto hrtfBuffer = data->EffectHrtfInfo->GetBuffer() + offsetIntoHrtfBuffer;
        auto spatialBlend = state->spatializerdata->spatialblend;

        // Unity downmixes multichannel to stereo, or upmixes mono to stereo, before handing
        // to spatializer, but audio buffer may have additional empty channels depending on size
        // of final output device. Ignore these channels and just downmix stereo to mono.
        for (auto i = 0u; i < length; ++i)
        {
            hrtfBuffer[i] = (inbuffer[i * inChannels] + inbuffer[i * inChannels + 1]);
        }
        VectorMath::Arithmetic::MulC_32f(hrtfBuffer, hrtfBuffer, 0.5, length);

        // At this point, we'll get 100% HRTF signal. We want to apply the "spatialblend" param
        // To do this, adjust the amount of signal sent to the hrtfInputBuffer, and send some stereo
        // to the output buffer
        if (spatialBlend < 1.0f)
        {
            VectorMath::Arithmetic::MulC_32f(hrtfBuffer, hrtfBuffer, spatialBlend, length);
            VectorMath::Arithmetic::MulC_32f(outbuffer, inbuffer, 1 - spatialBlend, length * inChannels);
        }
        else
        {
            // If spatial blend == 1, we don't want any stereo signal bleeding through.
            std::memset(outbuffer, 0, length * inChannels * sizeof(float));
        }
    }

    // There's a lot of conditions in which the Spatializer should disable itself and operate in passthrough mode
    // This function helps clarify when that is
    bool ShouldSpatialize(UnityAudioEffectState* state)
    {
        // state data and SpatializerData are required.
        if (state == nullptr || state->spatializerdata == nullptr)
        {
            return false;
        }

        // DSP buffer size must be a power of two aligned and smaller than HRTF quantum.
        // This ensures even multiples fit inside a single HRTF pass for buffering.
        if (!IsPowerOfTwo(state->dspbuffersize) || state->dspbuffersize > c_HrtfFrameCount)
        {
            return false;
        }

        // Stream must be marked IsPlaying and spatial blend meaningfully > 0
        if (!(state->flags & UnityAudioEffectStateFlags_IsPlaying) || state->spatializerdata->spatialblend <= 0.001f)
        {
            return false;
        }

        // Do not spatialize if the EffectData is missing, or if the source is too quiet
        auto data = state->GetEffectData<EffectData>();
        if (data == nullptr || data->DryDistanceAttenuation <= c_MinAudibleGain)
        {
            return false;
        }

        // For all other cases, we should spatialize this stream
        return true;
    }

    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK ProcessCallback(
        UnityAudioEffectState* state, float* inbuffer, float* outbuffer, unsigned int length, int inChannels,
        int outChannels)
    {
        if (inChannels != outChannels)
        {
            // Don't need to support this because it doesn't seem this scenario exists in the Unity audio engine
            return UNITY_AUDIODSP_ERR_UNSUPPORTED;
        }

        auto data = state->GetEffectData<EffectData>();

        if (!ShouldSpatialize(state))
        {
            // Clearing out the SourceInfo releases the source and prevents hrtf processing
            if (data)
            {
                data->EffectHrtfInfo = nullptr;

                // If we're not spatializing because the gain is too low, mute the output
                if (data->DryDistanceAttenuation <= c_MinAudibleGain)
                {
                    memset(outbuffer, 0, length * outChannels * sizeof(float));
                    return UNITY_AUDIODSP_OK;
                }
            }

            // In all other cases, do a pass-through
            std::memcpy(outbuffer, inbuffer, length * outChannels * sizeof(float));
            return UNITY_AUDIODSP_OK;
        }

        auto S = state->spatializerdata->sourcematrix;
        auto L = state->spatializerdata->listenermatrix;

        // If we previously released the source, get one back
        if (data->EffectHrtfInfo == nullptr)
        {
            data->EffectHrtfInfo.reset(HrtfWrapper::GetHrtfSource());
        }

        // check local triton usage settings, then make sure triton is initialized
        if (data->Params[EffectParams::EnableTriton] && TritonWrapper::IsAceFileLoaded())
        {
            TritonAcousticParameters tritonParams = {};
            auto useTritonParams = QueryTriton(S, L, data->EffectHrtfInfo->GetIndex(), &tritonParams);

            // Only update the acoustic params if we were able to get a successful query acoustics
            if (useTritonParams)
            {
                UpdateAcousticParams(data, state->spatializerdata->spread, S, L, tritonParams);
            }
        }
        else
        {
            // If there's no ace file, or the user bypasses triton, update params using a through-the-wall method
            UpdateAcousticParams(data, state->spatializerdata->spread, ListenerToSourceDirection(S, L));
        }

        // Sometimes, the previous allocation can fail and produce a SourceBuffer with a null array
        // Make sure we have a buffer to use before proceeding
        if (data->EffectHrtfInfo->GetBuffer())
        {
            PrepareAudioData(state, inbuffer, outbuffer, length, outChannels);
        }

        return UNITY_AUDIODSP_OK;
    }
} // namespace Spatializer
