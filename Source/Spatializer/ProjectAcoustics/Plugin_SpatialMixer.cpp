// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
// Please note that this will only work on Unity 5.2 or higher.

#include "AudioPluginUtil.h"
#include "HrtfWrapper.h"
#include <cstring>
#include "vectormath.h"
#include "mathutility.h"

namespace SpatialMixer
{

    enum EffectParams
    {
        AdditionalReverbPower,
        DecayTimeScalar,
        MultichannelPanning,
        Count
    };

    struct EffectData
    {
        float params[EffectParams::Count];

        // History buffer used when the DSP buffer is smaller than the HRTF quantum
        std::unique_ptr<float[]> HrtfHistoryBuffer;

        // Current read offset into the history buffer
        int ReadOffset;
    };

    int InternalRegisterEffectDefinition(UnityAudioEffectDefinition& definition)
    {
        int numparams = EffectParams::Count;
        definition.paramdefs = new UnityAudioParameterDefinition[numparams];
        // Warning: The 'name' value (second argument) below has a strict limit of 15 characters
        RegisterParameter(
            definition,
            "Wetness Adjust",
            "dB",
            -20.0f,
            20.0f,
            0.0f,
            1.0f,
            1.0f,
            EffectParams::AdditionalReverbPower,
            "Additively adjust the calculated reverb wetness in dB for all sources in the scene based on "
            "source-listener distance.");
        RegisterParameter(
            definition,
            "RT60 Scale",
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
            "Use Panning",
            "",
            0.0f,
            1.0f,
            0.0f,
            1.0f,
            1.0f,
            EffectParams::MultichannelPanning,
            "Switch between binaural (0) and panning (1). Values other than 0 and 1 are set to 0 (binaural).");

        return numparams;
    }

    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK CreateCallback(UnityAudioEffectState* state)
    {
        auto effectdata = new EffectData;
        std::memset(effectdata, 0, sizeof(EffectData));
        state->effectdata = effectdata;
        InitParametersFromDefinitions(InternalRegisterEffectDefinition, effectdata->params);
        effectdata->params[EffectParams::AdditionalReverbPower] = 0;
        effectdata->params[EffectParams::DecayTimeScalar] = 1;

        // If the DSP buffer size is smaller than the HRTF quantum, allocate a history buffer.
        // Checking for DSP buffer sizes for PowerOfTwo alignment guarantees integral multiples fit within the HRTF
        // quantum. Unity DSP buffer sizes are PowerOfTwo aligned so this is just extra validation.
        if (state->dspbuffersize < c_HrtfFrameCount && IsPowerOfTwo(state->dspbuffersize))
        {
            effectdata->HrtfHistoryBuffer = std::make_unique<float[]>(2 * c_HrtfFrameCount);
            std::memset(effectdata->HrtfHistoryBuffer.get(), 0, (2 * c_HrtfFrameCount * sizeof(float)));
            effectdata->ReadOffset = 0;
        }

        // Initialize the wrapper so that the initial value of MultichannelPanning gets recorded
        HrtfWrapper::InitWrapper();

        return UNITY_AUDIODSP_OK;
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
        data->params[index] = value;

        // Immediately apply engine type change to HrtfWrapper
        if (index == EffectParams::MultichannelPanning)
        {
            if (value == 1.0f)
            {
                HrtfWrapper::SetActiveEngine(HrtfEngineType_Panner);
            }
            else
            {
                HrtfWrapper::SetActiveEngine(HrtfEngineType_Binaural);
            }
        }

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
            *value = data->params[index];
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

    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK ProcessCallback(
        UnityAudioEffectState* state, float* inbuffer, float* outbuffer, unsigned int length, int inchannels,
        int outchannels)
    {
        // Check that I/O formats are right and that the host API supports this feature
        if (!(state->flags & UnityAudioEffectStateFlags_IsPlaying) || !IsPowerOfTwo(state->dspbuffersize) ||
            state->dspbuffersize > c_HrtfFrameCount || state->dspbuffersize != length)
        {
            std::memcpy(outbuffer, inbuffer, length * outchannels * sizeof(float));
            return UNITY_AUDIODSP_OK;
        }

        auto data = state->GetEffectData<EffectData>();

        // Buffered processing
        if (data->HrtfHistoryBuffer != nullptr)
        {
            // Call HRTF processing if it's time
            auto ticksPerHrtfBuffer = c_HrtfFrameCount / length;
            auto currentTick = (state->currdsptick / length) % ticksPerHrtfBuffer;
            if (currentTick == (ticksPerHrtfBuffer - 1))
            {
                // Reset the read offset to the beginning of the history buffer
                data->ReadOffset = 0;

                // In case of failure, fill with silence
                if (HrtfWrapper::Process(data->HrtfHistoryBuffer.get(), c_HrtfFrameCount, outchannels) == 0)
                {
                    std::memset(data->HrtfHistoryBuffer.get(), 0, c_HrtfFrameCount * outchannels * sizeof(float));
                }
            }

            // Copy from the history buffer
            std::memcpy(
                outbuffer, data->HrtfHistoryBuffer.get() + data->ReadOffset, length * outchannels * sizeof(float));

            // Update the read offset
            data->ReadOffset += outchannels * length;

            // Mix output into the stereo content
            VectorMath::Arithmetic::Add_32f_I(outbuffer, inbuffer, length * inchannels);
        }
        // Non-buffered path
        else
        {
            // Do a mix only if the Process call produces any samples
            if (HrtfWrapper::Process(outbuffer, length, outchannels) > 0)
            {
                // Mix output into the stereo content
                VectorMath::Arithmetic::Add_32f_I(outbuffer, inbuffer, length * inchannels);
            }
            else
            {
                // On failure, just copy input to output
                std::memcpy(outbuffer, inbuffer, length * outchannels * sizeof(float));
            }
        }

        // Update parameters every processing pass
        // This introduces (at worst) one frame of latency for new parameters to kick in
        // However, this guarantees that parameters will always take effect, even if no Spatialized sources were active
        // at the time of the parameters being updated
        HrtfWrapper::SetGlobalReverbPowerAdjustment(data->params[EffectParams::AdditionalReverbPower]);
        HrtfWrapper::SetGlobalReverbTimeAdjustment(data->params[EffectParams::DecayTimeScalar]);

        return UNITY_AUDIODSP_OK;
    }
} // namespace SpatialMixer
