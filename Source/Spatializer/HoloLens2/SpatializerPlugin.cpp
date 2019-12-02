// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

// NOTE: This plugin will only work on Unity 5.2 or higher.

#include <math.h>
#include <algorithm>
#include <memory>
#include <cstring>

#include "AudioPluginUtil.h"
#include "HrtfConstants.h"
#include "mathutility.h"
#include "SpatialAudioManager.h"
#include "vectormath.h"

namespace Spatializer
{
    enum EffectParams
    {
        RoomEffectSendPower,
        Count
    };

    struct EffectData
    {
        std::unique_ptr<ISpatialSource> SpatialSource;
        float SourceDistance = 0.0f;
        float DryDistanceAttenuation = 0.0f;
        float Params[EffectParams::Count] = {0};
    };

    int InternalRegisterEffectDefinition(UnityAudioEffectDefinition& definition)
    {
        definition.flags |= UnityAudioEffectDefinitionFlags_IsSpatializer;

        int numparams = EffectParams::Count;
        definition.paramdefs = new UnityAudioParameterDefinition[numparams];
        // Warning: The 'name' value (second argument) below has a strict limit of 15 characters
        RegisterParameter(
            definition,
            "RoomEffectSend",
            "dB",
            -100.0f,
            20.0f,
            0.0f,
            1.0f,
            1.0f,
            EffectParams::RoomEffectSendPower,
            "Room Effect Send Level");
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
        auto effectdata = new EffectData();
        std::memset(effectdata, 0, sizeof(EffectData));
        state->effectdata = effectdata;
        state->spatializerdata->distanceattenuationcallback = DistanceAttenuationCallback;
        InitParametersFromDefinitions(InternalRegisterEffectDefinition, effectdata->Params);
        effectdata->Params[EffectParams::RoomEffectSendPower] = -100.0f;
        SpatialAudioManager::EnsureInitialized();

        effectdata->SpatialSource = SpatialAudioManager::GetSpatialSource();

        return effectdata->SpatialSource ? UNITY_AUDIODSP_OK : UNITY_AUDIODSP_ERR_UNSUPPORTED;
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

    Direction ListenerToSourceDirection(const float* sourceMatrix, const float* listenerMatrix)
    {
        const auto* const S = sourceMatrix;
        const auto* const L = listenerMatrix;

        // S[12] = SourcePos.x, S[13] = SourcePos.y, S[14] = SourcePos.z
        auto loc_x = L[0] * S[12] + L[4] * S[13] + L[8] * S[14] + L[12];
        auto loc_y = L[1] * S[12] + L[5] * S[13] + L[9] * S[14] + L[13];
        auto loc_z = L[2] * S[12] + L[6] * S[13] + L[10] * S[14] + L[14];

        return {loc_x, loc_y, loc_z};
    }

    void UpdateSpatialSourceParams(const EffectData* data, const Direction& direction)
    {
        if (data->SpatialSource)
        {
            SpatialSourceParameters params = {};
            params.PrimaryArrivalDirection = direction;
            params.PrimaryArrivalDistancePowerDb = AmplitudeToDb(data->DryDistanceAttenuation);
            data->SpatialSource->SetParameters(&params);
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
        auto hrtfBuffer = data->SpatialSource->GetBuffer() + offsetIntoHrtfBuffer;
        auto spatialBlend = state->spatializerdata->spatialblend;

        // Unity downmixes multichannel to stereo, or upmixes mono to stereo, before handing
        // to spatializer, but audio buffer may have additional empty channels depending on size
        // of final output device. Ignore these channels and just downmix stereo to mono.
        for (auto i = 0u; i < length; ++i)
        {
            hrtfBuffer[i] = (inbuffer[i * inChannels] + inbuffer[i * inChannels + 1]);
        }
        VectorMath::Arithmetic::MulC_32f(hrtfBuffer, hrtfBuffer, 0.5, length);

        // At this point, we'll get 100% HRTF signal.
        // We want to apply the "Spatial Blend" parameter to this signal.
        // To do this, adjust the amount of signal sent to the hrtfInputBuffer.
        if (spatialBlend < 1.0f)
        {
            VectorMath::Arithmetic::MulC_32f(hrtfBuffer, hrtfBuffer, spatialBlend, length);
        }

        // Attenuation for non-spatialized pass-through signal is a combination of Spatial Blend and room effect send
        // level. If spatial blend == 1, only the room effect level is passed through.
        auto outAttenuation = (1 - spatialBlend) + DbToAmplitude(data->Params[EffectParams::RoomEffectSendPower]);
        VectorMath::Arithmetic::MulC_32f(outbuffer, inbuffer, outAttenuation, static_cast<size_t>(length) * inChannels);

        data->SpatialSource->ReleaseBuffer(length);
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
            // Clearing out the SpatialSource releases the source and prevents spatial processing
            if (data->SpatialSource)
            {
                data->SpatialSource = nullptr;

                // If we're not spatializing because the gain is too low, mute the output
                if (data->DryDistanceAttenuation <= c_MinAudibleGain)
                {
                    memset(outbuffer, 0, static_cast<size_t>(length) * outChannels * sizeof(float));
                    return UNITY_AUDIODSP_OK;
                }
            }

            // In all other cases, do a pass-through
            VectorMath::Arithmetic::MulC_32f(
                outbuffer, inbuffer, data->DryDistanceAttenuation, static_cast<size_t>(length) * inChannels);

            return UNITY_AUDIODSP_OK;
        }

        auto S = state->spatializerdata->sourcematrix;
        auto L = state->spatializerdata->listenermatrix;

        // If we previously released the source, get one back
        if (data->SpatialSource == nullptr)
        {
            data->SpatialSource = SpatialAudioManager::GetSpatialSource();
        }

        // If the source is still null, we can't spatialize anymore. Just mute it.
        if (data->SpatialSource == nullptr)
        {
            memset(outbuffer, 0, static_cast<size_t>(length) * outChannels * sizeof(float));
            return UNITY_AUDIODSP_OK;
        }

        // Update params using a through-the-wall method
        UpdateSpatialSourceParams(data, ListenerToSourceDirection(S, L));

        // Sometimes, the previous allocation can fail and produce a SourceBuffer with a null array
        // Make sure we have a buffer to use before proceeding
        if (data->SpatialSource->GetBuffer())
        {
            PrepareAudioData(state, inbuffer, outbuffer, length, outChannels);
        }

        // Inform ISAC adapter that audio engine is still running
        SpatialAudioManager::Process(outbuffer, length, inChannels);

        return UNITY_AUDIODSP_OK;
    }
} // namespace Spatializer
