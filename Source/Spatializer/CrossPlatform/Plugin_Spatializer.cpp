// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
// Please note that this will only work on Unity 5.2 or higher.

#include "AudioPluginUtil.h"
#include "HrtfWrapper.h"
#include "VectorMath.h"
#include "mathutility.h"

#include <math.h>
#include <algorithm>
#include <memory>
#include <cstring>

namespace Spatializer
{
    enum EffectParams
    {
        AdditionalReverbPower,
        DecayTimeScalar,
        OcclusionFactor,
        DistanceWarp,
        OutdoornessAdjustment,
        HrtfMode,
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
            "Hrtf Mode",
            "",
            0.0f,
            3.0f,
            1.0f,
            1.0f,
            1.0f,
            EffectParams::HrtfMode,
            "Set FLEX mode. 0: no spatialization, 1: high quality, 2: medium quality, 3: low quality");
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
        effectdata->Params[EffectParams::HrtfMode] = 1;
        effectdata->Params[EffectParams::OcclusionFactor] = 1;
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

    static ATKVectorF ListenerToSourceDirection(const float* sourceMatrix, const float* listenerMatrix)
    {
        const auto* const S = sourceMatrix;
        const auto* const L = listenerMatrix;

        // S[12] = SourcePos.x, S[13] = SourcePos.y, S[14] = SourcePos.z
        auto loc_x = L[0] * S[12] + L[4] * S[13] + L[8] * S[14] + L[12];
        auto loc_y = L[1] * S[12] + L[5] * S[13] + L[9] * S[14] + L[13];
        auto loc_z = -(L[2] * S[12] + L[6] * S[13] + L[10] * S[14] + L[14]);

        return {loc_x, loc_y, loc_z};
    }

    // Update acoustic params with default values
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

            // Not using acoustics, so start with an outdoorness of 0.5 and adjust from there
            auto adjustedOutdoorness = 0.5f + data->Params[EffectParams::OutdoornessAdjustment];
            acousticParams.Outdoorness = Clamp(adjustedOutdoorness, 0, 1);

            unsigned long hrtfMode = 0;
            if (data->Params[EffectParams::HrtfMode] == 1.0f)
            {
                hrtfMode = HrtfDspMode_Quality1;
            }
            else if (data->Params[EffectParams::HrtfMode] == 2.0f)
            {
                hrtfMode = HrtfDspMode_Quality2;
            }
            else if (data->Params[EffectParams::HrtfMode] == 3.0f)
            {
                hrtfMode = HrtfDspMode_Quality3;
            }

            acousticParams.HrtfMode = static_cast<HrtfDspMode>(hrtfMode);

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

        // Stream must be marked IsPlaying, Not Paused, Not Muted, and spatial blend meaningfully > 0
        if (!(state->flags & UnityAudioEffectStateFlags_IsPlaying) ||
            (state->flags & UnityAudioEffectStateFlags_IsPaused) ||
            (state->flags & UnityAudioEffectStateFlags_IsMuted) || state->spatializerdata->spatialblend <= 0.001f)
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

            // If EffectHrtfInfo is still null, that means we're not able to get HRTF resources.
            // Mute this source to prevent unexpectedly loud sounds
            if (data->EffectHrtfInfo == nullptr)
            {
                memset(outbuffer, 0, length * outChannels * sizeof(float));
                return UNITY_AUDIODSP_OK;
            }
        }

        // There's no acoustics support yet, update params using a through-the-wall method
        UpdateAcousticParams(data, state->spatializerdata->spread, ListenerToSourceDirection(S, L));

        // Sometimes, the previous allocation can fail and produce a SourceBuffer with a null array
        // Make sure we have a buffer to use before proceeding
        if (data->EffectHrtfInfo->GetBuffer())
        {
            PrepareAudioData(state, inbuffer, outbuffer, length, outChannels);
        }

        return UNITY_AUDIODSP_OK;
    }
} // namespace Spatializer