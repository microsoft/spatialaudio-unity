// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
#include "TritonWrapper.h"
#include <vector>
#include <cstring>

// Static init
OBJECT_HANDLE TritonWrapper::s_TritonHandle = nullptr;
bool TritonWrapper::s_IsTritonAceLoaded = false;
ATKMatrix4x4 TritonWrapper::s_worldToLocal = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
ATKMatrix4x4 TritonWrapper::s_localToWorld = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
std::vector<TritonAcousticParametersDebug> TritonWrapper::s_debugInfo;
float s_lastOutdoorness = 0.0f;

#if defined(WINDOWS) || defined(DURANGO)
HMODULE TritonWrapper::s_TritonDllHandle = nullptr;
#elif defined(LINUX) || defined(ANDROID) || defined(APPLE)
#include <dlfcn.h>
void* TritonWrapper::s_TritonDllHandle = nullptr;
#endif
TritonWrapper::Triton_QueryAcoustics_ptr TritonWrapper::s_QueryAcoustics = nullptr;
TritonWrapper::Triton_GetOutdoornessAtListener_ptr TritonWrapper::s_GetOutdoornessAtListener = nullptr;

// Put any exported functions here
#ifdef __cplusplus
extern "C"
#endif
{
    bool EXPORT_API Spatializer_SetTritonHandle(OBJECT_HANDLE handle)
    {
        return TritonWrapper::SetTritonHandle(handle);
    }

    void EXPORT_API Spatializer_SetAceFileLoaded(bool loaded)
    {
        TritonWrapper::SetAceFileLoaded(loaded);
    }

    void EXPORT_API Spatializer_SetTransforms(ATKMatrix4x4 worldToLocal, ATKMatrix4x4 localToWorld)
    {
        TritonWrapper::SetTransforms(worldToLocal, localToWorld);
    }

    bool EXPORT_API Spatializer_GetDebugInfo(TritonAcousticParametersDebug** debugArray, int* size)
    {
        // Take a snapshot of the debug info
        auto debugCopy = TritonWrapper::GetDebugInfo();

        // If the vector is empty, don't allocate any memory
        if (debugCopy.size() == 0)
        {
            *size = 0;
            *debugArray = nullptr;
            return true;
        }

        // If the vector is not empty, allocate and copy memory
        *debugArray = new (std::nothrow) TritonAcousticParametersDebug[debugCopy.size()];
        // If out of memory, just return 0
        if (*debugArray == nullptr)
        {
            *size = 0;
            return false;
        }

        memcpy(*debugArray, debugCopy.data(), debugCopy.size() * sizeof(TritonAcousticParametersDebug));
        *size = static_cast<int>(debugCopy.size());

        return true;
    }

    void EXPORT_API Spatializer_FreeDebugInfo(TritonAcousticParametersDebug* debugArray)
    {
        if (debugArray != nullptr)
        {
            delete[] debugArray;
        }
    }
}

bool TritonWrapper::SetTritonHandle(OBJECT_HANDLE handle)
{
    if (s_TritonDllHandle == nullptr)
    {
        auto succeeded = LoadTritonDll();
        if (!succeeded)
        {
            return false;
        }
    }
    s_TritonHandle = handle;
    if (handle == nullptr)
    {
        s_IsTritonAceLoaded = false;
    }

    return true;
}

// Helper macro for getting function addresses
#if defined(WINDOWS) || defined(DURANGO)
#define IMPORT_FUNCTION(a, b) reinterpret_cast<a>(GetProcAddress(s_TritonDllHandle, b))
#else
#define IMPORT_FUNCTION(a, b) reinterpret_cast<a>(dlsym(s_TritonDllHandle, b))
#endif

bool TritonWrapper::LoadTritonDll()
{
#if defined(WINDOWS) || defined(DURANGO)
#if defined(WINDOWSSTORE)
    s_TritonDllHandle = LoadPackagedLibrary(L"Triton.dll", 0);
#else
    s_TritonDllHandle = LoadLibraryW(L"Triton.dll");
#endif
#elif defined(LINUX) || defined(ANDROID)
    s_TritonDllHandle = dlopen("libTriton.so", RTLD_LAZY);
#elif defined(APPLE)
#define STRINGIFY(v) #v
#define MAKE_STR(m) STRINGIFY(m)
    const auto c_TritonLibName = "@rpath/libTriton." MAKE_STR(PRODUCT_VERSION) ".dylib";
    s_TritonDllHandle = dlopen(c_TritonLibName, RTLD_LAZY);
#endif

    if (!s_TritonDllHandle)
    {
        return false;
    }

    s_QueryAcoustics = IMPORT_FUNCTION(Triton_QueryAcoustics_ptr, "Triton_QueryAcoustics");
    s_GetOutdoornessAtListener =
        IMPORT_FUNCTION(Triton_GetOutdoornessAtListener_ptr, "Triton_GetOutdoornessAtListener");

    if (s_QueryAcoustics == nullptr || s_GetOutdoornessAtListener == nullptr)
    {
        return false;
    }

    return true;
}

bool TritonWrapper::QueryAcoustics(
    ATKVectorF source, ATKVectorF listener, const int sourceIndex, TritonAcousticParameters* params)
{
    if (!IsAceFileLoaded())
    {
        return false;
    }
    auto result = s_QueryAcoustics(s_TritonHandle, source, listener, params);

    // Cache params in the debugInfo vector before returning so that Unity can query it later
    TritonAcousticParametersDebug debugParams = {};
    debugParams.SourceId = sourceIndex;
    debugParams.SourcePosition = source;
    debugParams.ListenerPosition = listener;
    debugParams.AcousticParameters = *params;
    debugParams.Outdoorness = s_lastOutdoorness;

    bool foundIt = false;
    for (auto&& info : s_debugInfo)
    {
        if (info.SourceId == sourceIndex)
        {
            info = debugParams;
            foundIt = true;
        }
    }
    if (!foundIt)
    {
        s_debugInfo.push_back(debugParams);
    }

    return result;
}

bool TritonWrapper::GetOutdoornessAtListener(ATKVectorF listener, float* value)
{
    if (!IsAceFileLoaded())
    {
        return false;
    }

    auto result = s_GetOutdoornessAtListener(s_TritonHandle, listener, value);
    if (result)
    {
        s_lastOutdoorness = *value;
    }
    return result;
}

std::vector<TritonAcousticParametersDebug> TritonWrapper::GetDebugInfo()
{
    // Take a snapshot of the debug data, clearing out the old info
    return std::move(s_debugInfo);
}