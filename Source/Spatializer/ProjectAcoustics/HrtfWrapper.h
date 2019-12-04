// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
#pragma once

#include "HrtfApiTypes.h"
#include "HrtfConstants.h"
#include <memory>
#include "AlignedBuffers.h"
// Delay load headers
#if defined(WINDOWS) || defined(DURANGO)
#include <Windows.h>
#endif

// Holds multiple engines, diverting data to the currently active engine
class HrtfWrapper final
{
public:
    class SourceInfo
    {
    public:
        SourceInfo(uint32_t index, HrtfInputBuffer* const sourceBuffer);
        ~SourceInfo();

        bool SetParameters(HrtfAcousticParameters* params) const noexcept;
        float* GetBuffer() const noexcept;
        uint32_t GetIndex() const noexcept;

    private:
        const uint32_t m_SourceIndex;
        HrtfInputBuffer* const m_SourceBuffer;
    };

    HrtfWrapper();
    ~HrtfWrapper()
    {
    }

    static void InitWrapper();
    static SourceInfo* GetHrtfSource();
    static uint32_t Process(float* outputBuffer, uint32_t numSamples, uint32_t numChannels) noexcept;
    static void SetActiveEngine(HrtfEngineType engineType) noexcept;
    static void SetGlobalReverbPowerAdjustment(float power);
    static float GetGlobalReverbPowerAdjustment();
    static void SetGlobalReverbTimeAdjustment(float time);
    static float GetGlobalReverbTimeAdjustment();

    friend class SourceInfo;

private:
    // Methods
    SourceInfo* GetAvailableHrtfSource();
    void ReleaseSource(uint32_t sourceIndex);
    uint32_t ProcessHrtfs(float* outputBuffer, uint32_t numSamples, uint32_t numChannels) noexcept;
    bool SetParameters(uint32_t index, HrtfAcousticParameters* params) noexcept;
    void SetActiveEngineType(HrtfEngineType engineType) noexcept;
    bool LoadHrtfDll();
    void ResetSources(OBJECT_HANDLE engine, const HrtfInputBuffer* buffers, uint32_t numBuffers);

    // Data
    static std::unique_ptr<HrtfWrapper> s_HrtfWrapper;

    AlignedStore::AlignedBuffers<float> m_SampleBuffers;
    HrtfInputBuffer m_HrtfInputBuffers[c_HrtfMaxSources];

    //! RAII helper for C++: Deletion functor
    struct UniqueHrtfEngineDeleter
    {
        //! Accepts an object handle and uninitializes it
        //! \param p Handle to uninitialize
        void operator()(OBJECT_HANDLE p)
        {
            HrtfWrapper::s_HrtfEngineUninitialize(p);
        }
    };
    typedef UniqueObjectHandle<UniqueHrtfEngineDeleter> UniqueHrtfEngineHandle;

    UniqueHrtfEngineHandle m_BinauralEngine;
    UniqueHrtfEngineHandle m_PanningEngine;
    OBJECT_HANDLE m_ActiveEngine;
    HrtfEngineType m_ActiveEngineType;
    HrtfOutputFormat m_CurrentFormat;
    bool m_CurrentFormatSupported;

    float m_GlobalReverbPower;
    float m_GlobalReverbTime;

    // Delay load helpers
#if defined(WINDOWS) || defined(DURANGO)
    static HMODULE s_HrtfDllHandle;
#elif defined(ANDROID) || defined(LINUX) || defined(APPLE)
    static void* s_HrtfDllHandle;
#endif

    typedef bool (*HrtfEngineInitialize_ptr)(uint32_t, HrtfEngineType, uint32_t, OBJECT_HANDLE);
    static HrtfEngineInitialize_ptr s_HrtfEngineInitialize;

    typedef bool (*HrtfEngineSetOutputFormat_ptr)(OBJECT_HANDLE, HrtfOutputFormat);
    static HrtfEngineSetOutputFormat_ptr s_HrtfEngineSetOutputFormat;

    typedef void (*HrtfEngineUninitialize_ptr)(OBJECT_HANDLE);
    static HrtfEngineUninitialize_ptr s_HrtfEngineUninitialize;

    typedef uint32_t (*HrtfEngineProcess_ptr)(OBJECT_HANDLE, HrtfInputBuffer*, uint32_t, float*, uint32_t);
    static HrtfEngineProcess_ptr s_HrtfEngineProcess;

    typedef bool (*HrtfEngineAcquireResourcesForSource_ptr)(OBJECT_HANDLE, uint32_t);
    static HrtfEngineAcquireResourcesForSource_ptr s_HrtfEngineAcquireResourcesForSource;

    typedef void (*HrtfEngineReleaseResourcesForSource_ptr)(OBJECT_HANDLE, uint32_t);
    static HrtfEngineReleaseResourcesForSource_ptr s_HrtfEngineReleaseResourcesForSource;

    typedef void (*HrtfEngineResetSource_ptr)(OBJECT_HANDLE, uint32_t);
    static HrtfEngineResetSource_ptr s_HrtfEngineResetSource;

    typedef void (*HrtfEngineResetAllSources_ptr)(OBJECT_HANDLE);
    static HrtfEngineResetAllSources_ptr s_HrtfEngineResetAllSources;

    typedef bool (*HrtfEngineSetParametersForSource_ptr)(OBJECT_HANDLE, uint32_t, HrtfAcousticParameters*);
    static HrtfEngineSetParametersForSource_ptr s_HrtfEngineSetParametersForSource;
};
