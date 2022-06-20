// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
// Please note that this will only work on Unity 5.2 or higher.

#include "AudioPluginUtil.h"
#include "HrtfWrapper.h"
#include "vectormath.h"
#include "mathutility.h"

#include <cstring>

namespace SpatializerMixer
{
    struct EffectData
    {
        // History buffer used when the DSP buffer is smaller than the HRTF quantum
        std::unique_ptr<float[]> HrtfHistoryBuffer;

        // Current read offset into the history buffer
        int ReadOffset;
    };

    int InternalRegisterEffectDefinition(UnityAudioEffectDefinition&)
    {
        return 0;
    }

    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK CreateCallback(UnityAudioEffectState* state)
    {
        auto effectdata = new EffectData;
        std::memset(effectdata, 0, sizeof(EffectData));
        state->effectdata = effectdata;

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

    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK SetFloatParameterCallback(UnityAudioEffectState*, int, float)
    {
        return UNITY_AUDIODSP_OK;
    }

    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK GetFloatParameterCallback(UnityAudioEffectState*, int, float*, char*)
    {
        return UNITY_AUDIODSP_OK;
    }

    int UNITY_AUDIODSP_CALLBACK GetFloatBufferCallback(UnityAudioEffectState*, const char*, float*, int)
    {
        return UNITY_AUDIODSP_OK;
    }

    UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK ProcessCallback(
        UnityAudioEffectState* state, float* inBuffer, float* outBuffer, unsigned int length, int inChannels,
        int outChannels)
    {
        // Check that I/O formats are right and that the host API supports this feature
        if (!(state->flags & UnityAudioEffectStateFlags_IsPlaying) ||
            (state->flags & UnityAudioEffectStateFlags_IsPaused) ||
            (state->flags & UnityAudioEffectStateFlags_IsMuted) || !IsPowerOfTwo(state->dspbuffersize) ||
            state->dspbuffersize > c_HrtfFrameCount || state->dspbuffersize != length)
        {
            std::memcpy(outBuffer, inBuffer, length * outChannels * sizeof(float));
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
                if (HrtfWrapper::Process(data->HrtfHistoryBuffer.get(), c_HrtfFrameCount, outChannels) == 0)
                {
                    std::memset(data->HrtfHistoryBuffer.get(), 0, c_HrtfFrameCount * outChannels * sizeof(float));
                }
            }

            // Copy from the history buffer
            std::memcpy(
                outBuffer, data->HrtfHistoryBuffer.get() + data->ReadOffset, length * outChannels * sizeof(float));

            // Update the read offset
            data->ReadOffset += outChannels * length;

            // Mix output into the stereo content
            VectorMath::Arithmetic::Add_32f_I(outBuffer, inBuffer, length * inChannels);
        }
        // Non-buffered path
        else
        {
            // Do a mix only if the Process call produces any samples
            if (HrtfWrapper::Process(outBuffer, length, outChannels) > 0)
            {
                // Mix output into the stereo content
                VectorMath::Arithmetic::Add_32f_I(outBuffer, inBuffer, length * inChannels);
            }
            else
            {
                // On failure, just copy input to output
                std::memcpy(outBuffer, inBuffer, length * outChannels * sizeof(float));
            }
        }

        return UNITY_AUDIODSP_OK;
    }
} // namespace SpatialMixer
