// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "HrtfWrapper.h"
#include <exception>
#include <cstring>

// Statics
std::unique_ptr<HrtfWrapper> HrtfWrapper::s_HrtfWrapper;

// Delay load statics
#if defined(WINDOWS) || defined(DURANGO)
HMODULE HrtfWrapper::s_HrtfDllHandle = nullptr;
#elif defined(LINUX) || defined(ANDROID) || defined(APPLE)
#include <dlfcn.h>
void* HrtfWrapper::s_HrtfDllHandle = nullptr;
#endif

HrtfWrapper::HrtfEngineInitialize_ptr HrtfWrapper::s_HrtfEngineInitialize = nullptr;
HrtfWrapper::HrtfEngineUninitialize_ptr HrtfWrapper::s_HrtfEngineUninitialize = nullptr;
HrtfWrapper::HrtfEngineSetOutputFormat_ptr HrtfWrapper::s_HrtfEngineSetOutputFormat = nullptr;
HrtfWrapper::HrtfEngineProcess_ptr HrtfWrapper::s_HrtfEngineProcess = nullptr;
HrtfWrapper::HrtfEngineAcquireResourcesForSource_ptr HrtfWrapper::s_HrtfEngineAcquireResourcesForSource = nullptr;
HrtfWrapper::HrtfEngineReleaseResourcesForSource_ptr HrtfWrapper::s_HrtfEngineReleaseResourcesForSource = nullptr;
HrtfWrapper::HrtfEngineResetSource_ptr HrtfWrapper::s_HrtfEngineResetSource = nullptr;
HrtfWrapper::HrtfEngineResetAllSources_ptr HrtfWrapper::s_HrtfEngineResetAllSources = nullptr;
HrtfWrapper::HrtfEngineSetParametersForSource_ptr HrtfWrapper::s_HrtfEngineSetParametersForSource = nullptr;

// Helper macro for getting function addresses
#if defined(WINDOWS) || defined(DURANGO)
#define IMPORT_FUNCTION(a, b) reinterpret_cast<a>(GetProcAddress(s_HrtfDllHandle, b))
#else
#define IMPORT_FUNCTION(a, b) reinterpret_cast<a>(dlsym(s_HrtfDllHandle, b))
#endif

HrtfWrapper::SourceInfo::SourceInfo(uint32_t index, HrtfInputBuffer* const sourceBuffer)
    : m_SourceIndex(index), m_SourceBuffer(sourceBuffer)
{
}

HrtfWrapper::SourceInfo::~SourceInfo()
{
    m_SourceBuffer->Buffer = nullptr;
    m_SourceBuffer->Length = 0;
    if (HrtfWrapper::s_HrtfWrapper)
    {
        s_HrtfWrapper->ReleaseSource(m_SourceIndex);
    }
}

void HrtfWrapper::InitWrapper()
{
    if (!HrtfWrapper::s_HrtfWrapper)
    {
        HrtfWrapper::s_HrtfWrapper.reset(new HrtfWrapper());
    }
}

bool HrtfWrapper::SourceInfo::SetParameters(HrtfAcousticParameters* params) const noexcept
{
    if (HrtfWrapper::s_HrtfWrapper)
    {
        return HrtfWrapper::s_HrtfWrapper->SetParameters(m_SourceIndex, params);
    }
    return false;
}

float* HrtfWrapper::SourceInfo::GetBuffer() const noexcept
{
    return m_SourceBuffer->Buffer;
}

uint32_t HrtfWrapper::SourceInfo::GetIndex() const noexcept
{
    return m_SourceIndex;
}

HrtfWrapper::SourceInfo* HrtfWrapper::GetHrtfSource()
{
    return HrtfWrapper::s_HrtfWrapper->GetAvailableHrtfSource();
}

uint32_t HrtfWrapper::Process(float* outputBuffer, uint32_t numSamples, uint32_t numChannels) noexcept
{
    if (HrtfWrapper::s_HrtfWrapper)
    {
        return HrtfWrapper::s_HrtfWrapper->ProcessHrtfs(outputBuffer, numSamples, numChannels);
    }
    return 0;
}

HrtfWrapper::HrtfWrapper()
    : m_GlobalReverbPower(0), m_GlobalReverbTime(1), m_SampleBuffers(c_HrtfMaxSources, c_HrtfFrameCount)
{
    for (auto&& buffer : m_HrtfInputBuffers)
    {
        buffer.Buffer = nullptr;
        buffer.Length = 0;
    }

    bool loadResult = LoadHrtfDll();
    if (!loadResult)
    {
        throw std::runtime_error("Missing HrtfDsp");
    }

    bool result = s_HrtfEngineInitialize(c_HrtfMaxSources, HrtfEngineType_Binaural, c_HrtfFrameCount, &m_BinauralEngine);
    if (!result)
    {
        throw std::bad_alloc();
    }

    result = s_HrtfEngineInitialize(c_HrtfMaxSources, HrtfEngineType_Panner, c_HrtfFrameCount, &m_PanningEngine);
    if (!result)
    {
        throw std::bad_alloc();
    }

    m_ActiveEngine = m_BinauralEngine.Get();
    m_ActiveEngineType = HrtfEngineType_Binaural;
    m_CurrentFormat = HrtfOutputFormat_Stereo;
    m_CurrentFormatSupported = s_HrtfEngineSetOutputFormat(m_ActiveEngine, m_CurrentFormat);
}

HrtfWrapper::SourceInfo* HrtfWrapper::GetAvailableHrtfSource()
{
    for (auto i = 0u; i < c_HrtfMaxSources; i++)
    {
        // If this buffer is not in use and we can get the control interface, return this index
        // Otherwise, keep trying the next indices
        if (m_HrtfInputBuffers[i].Buffer == nullptr)
        {
            if (s_HrtfEngineAcquireResourcesForSource(m_BinauralEngine.Get(), i))
            {
                if (s_HrtfEngineAcquireResourcesForSource(m_PanningEngine.Get(), i))
                {
                    std::memset(m_SampleBuffers[i].Data, 0, c_HrtfFrameCount * sizeof(float));
                    m_HrtfInputBuffers[i].Buffer = m_SampleBuffers[i].Data;
                    m_HrtfInputBuffers[i].Length = c_HrtfFrameCount;

                    return new HrtfWrapper::SourceInfo(i, &m_HrtfInputBuffers[i]);
                }
                else
                {
                    // If successfully acquired in one engine but failed in the other, release
                    // the acquired resources
                    s_HrtfEngineReleaseResourcesForSource(m_BinauralEngine.Get(), i);
                }
            }
        }
    }

    return nullptr;
}

void HrtfWrapper::ReleaseSource(uint32_t sourceIndex)
{
    s_HrtfEngineReleaseResourcesForSource(m_BinauralEngine.Get(), sourceIndex);
    s_HrtfEngineReleaseResourcesForSource(m_PanningEngine.Get(), sourceIndex);
}

HrtfOutputFormat GetFormatFromChannels(uint32_t numChannels)
{
    if (numChannels == 1)
    {
        return HrtfOutputFormat_Mono;
    }
    else if (numChannels == 2)
    {
        return HrtfOutputFormat_Stereo;
    }
    else if (numChannels == 4)
    {
        return HrtfOutputFormat_Quad;
    }
    else if (numChannels == 5)
    {
        return HrtfOutputFormat_5;
    }
    else if (numChannels == 6)
    {
        return HrtfOutputFormat_5dot1;
    }
    else if (numChannels == 8)
    {
        return HrtfOutputFormat_7dot1;
    }
    return HrtfOutputFormat_Count;
}

uint32_t HrtfWrapper::ProcessHrtfs(float* outputBuffer, uint32_t numSamples, uint32_t numChannels) noexcept
{
    auto thisFormat = GetFormatFromChannels(numChannels);
    if (thisFormat != m_CurrentFormat)
    {
        m_CurrentFormat = thisFormat;
        m_CurrentFormatSupported = s_HrtfEngineSetOutputFormat(m_ActiveEngine, thisFormat);
    }

    if (!m_CurrentFormatSupported)
    {
        return 0;
    }

    auto retVal =
        s_HrtfEngineProcess(m_ActiveEngine, m_HrtfInputBuffers, c_HrtfMaxSources, outputBuffer, numSamples * numChannels);

    // We've consumed all the audio data for this pass. Clear out the input buffers
    for (auto i = 0u; i < c_HrtfMaxSources; ++i)
    {
        memset(m_SampleBuffers[i].Data, 0, c_HrtfFrameCount * sizeof(float));
    }

    return retVal;
}

bool HrtfWrapper::SetParameters(uint32_t index, HrtfAcousticParameters* params) noexcept
{
    return s_HrtfEngineSetParametersForSource(m_ActiveEngine, index, params);
}

void HrtfWrapper::SetActiveEngine(HrtfEngineType engineType) noexcept
{
    if (s_HrtfWrapper)
    {
        s_HrtfWrapper->SetActiveEngineType(engineType);
    }
}

void HrtfWrapper::ResetSources(OBJECT_HANDLE engine, const HrtfInputBuffer* buffers, uint32_t numBuffers)
{
    for (auto source = 0u; source < numBuffers; ++source)
    {
        if (buffers[source].Buffer)
        {
            s_HrtfEngineResetSource(engine, source);
        }
    }
}

void HrtfWrapper::SetActiveEngineType(HrtfEngineType engineType) noexcept
{
    if (engineType != m_ActiveEngineType)
    {
        if (engineType == HrtfEngineType_Binaural)
        {
            ResetSources(m_BinauralEngine.Get(), m_HrtfInputBuffers, c_HrtfMaxSources);
            m_CurrentFormatSupported = s_HrtfEngineSetOutputFormat(m_BinauralEngine.Get(), m_CurrentFormat);
            m_ActiveEngine = m_BinauralEngine.Get();
        }
        else
        {
            // PanningEngine does not have many per-source resources. It does have per-filter resources,
            // which can only be reset when all sources do.
            s_HrtfEngineResetAllSources(m_PanningEngine.Get());
            m_CurrentFormatSupported = s_HrtfEngineSetOutputFormat(m_PanningEngine.Get(), m_CurrentFormat);
            m_ActiveEngine = m_PanningEngine.Get();
        }
        m_ActiveEngineType = engineType;
    }
}

void HrtfWrapper::SetGlobalReverbPowerAdjustment(float power)
{
    if (HrtfWrapper::s_HrtfWrapper)
    {
        s_HrtfWrapper->m_GlobalReverbPower = power;
    }
}
float HrtfWrapper::GetGlobalReverbPowerAdjustment()
{
    if (HrtfWrapper::s_HrtfWrapper)
    {
        return s_HrtfWrapper->m_GlobalReverbPower;
    }
    return 0;
}

void HrtfWrapper::SetGlobalReverbTimeAdjustment(float time)
{
    if (HrtfWrapper::s_HrtfWrapper)
    {
        s_HrtfWrapper->m_GlobalReverbTime = time;
    }
}
float HrtfWrapper::GetGlobalReverbTimeAdjustment()
{
    if (HrtfWrapper::s_HrtfWrapper)
    {
        return s_HrtfWrapper->m_GlobalReverbTime;
    }
    return 0;
}


bool HrtfWrapper::LoadHrtfDll()
{
#if defined(WINDOWS) || defined(DURANGO)
#if defined(WINDOWSSTORE)
    s_HrtfDllHandle = LoadPackagedLibrary(L"HrtfDsp.dll", 0);
#else
    s_HrtfDllHandle = LoadLibraryW(L"HrtfDsp.dll");
#endif
#elif defined(LINUX) || defined(ANDROID)
    s_TritonDllHandle = dlopen("libHrtfDsp.so", RTLD_LAZY);
#elif defined(APPLE)
#define STRINGIFY(v) #v
#define MAKE_STR(m) STRINGIFY(m)
    const auto c_TritonLibName = "@rpath/libHrtfDsp." MAKE_STR(PRODUCT_VERSION) ".dylib";
    s_TritonDllHandle = dlopen(c_TritonLibName, RTLD_LAZY);
#endif

    if (!s_HrtfDllHandle)
    {
        return false;
    }

    s_HrtfEngineInitialize = IMPORT_FUNCTION(HrtfEngineInitialize_ptr, "HrtfEngineInitialize");
    s_HrtfEngineUninitialize = IMPORT_FUNCTION(HrtfEngineUninitialize_ptr, "HrtfEngineUninitialize");
    s_HrtfEngineSetOutputFormat =
        IMPORT_FUNCTION(HrtfEngineSetOutputFormat_ptr, "HrtfEngineSetOutputFormat");

    s_HrtfEngineProcess = IMPORT_FUNCTION(HrtfEngineProcess_ptr, "HrtfEngineProcess");
    s_HrtfEngineAcquireResourcesForSource = IMPORT_FUNCTION(HrtfEngineAcquireResourcesForSource_ptr, "HrtfEngineAcquireResourcesForSource");
    s_HrtfEngineReleaseResourcesForSource = IMPORT_FUNCTION(HrtfEngineReleaseResourcesForSource_ptr, "HrtfEngineReleaseResourcesForSource");
    s_HrtfEngineResetSource = IMPORT_FUNCTION(HrtfEngineResetSource_ptr, "HrtfEngineResetSource");
    s_HrtfEngineResetAllSources = IMPORT_FUNCTION(HrtfEngineResetAllSources_ptr, "HrtfEngineResetAllSources");
    s_HrtfEngineSetParametersForSource = IMPORT_FUNCTION(HrtfEngineSetParametersForSource_ptr, "HrtfEngineSetParametersForSource");

    if (s_HrtfEngineInitialize == nullptr || 
        s_HrtfEngineSetOutputFormat == nullptr ||
        s_HrtfEngineUninitialize == nullptr ||
        s_HrtfEngineProcess == nullptr || 
        s_HrtfEngineAcquireResourcesForSource == nullptr ||
        s_HrtfEngineReleaseResourcesForSource == nullptr ||
        s_HrtfEngineResetSource == nullptr ||
        s_HrtfEngineResetAllSources == nullptr ||
        s_HrtfEngineSetParametersForSource == nullptr)
    {
        return false;
    }

    return true;
}
