// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once

#include "TritonApiTypes.h"
#include <vector>

// Delay load headers
#if defined(WINDOWS) || defined(DURANGO)
#include <Windows.h>
#endif

class TritonWrapper
{
public:
    static bool SetTritonHandle(OBJECT_HANDLE handle);

    static void SetAceFileLoaded(bool loaded)
    {
        s_IsTritonAceLoaded = loaded;
        // Any time the ace file changes, we should clear out the old debug info
        s_debugInfo.clear();
    }

    static void SetTransforms(ATKMatrix4x4 worldToLocal, ATKMatrix4x4 localToWorld)
    {
        s_worldToLocal = worldToLocal;
        s_localToWorld = localToWorld;
    }

    static bool IsAceFileLoaded()
    {
        return s_IsTritonAceLoaded;
    }

    static ATKMatrix4x4 GetWorldToLocalTransform()
    {
        return s_worldToLocal;
    }

    static ATKMatrix4x4 GetLocalToWorldTransform()
    {
        return s_localToWorld;
    }

    static bool
    QueryAcoustics(ATKVectorF source, ATKVectorF listener, const int sourceIndex, TritonAcousticParameters* params);

    static bool GetOutdoornessAtListener(ATKVectorF listener, float* value);

    static std::vector<TritonAcousticParametersDebug> GetDebugInfo();

private:
    static bool LoadTritonDll();

    static OBJECT_HANDLE s_TritonHandle;
    static bool s_IsTritonAceLoaded;
    static ATKMatrix4x4 s_worldToLocal;
    static ATKMatrix4x4 s_localToWorld;
    static std::vector<TritonAcousticParametersDebug> s_debugInfo;

    // Delay load helpers
#if defined(WINDOWS) || defined(DURANGO)
    static HMODULE s_TritonDllHandle;
#elif defined(ANDROID) || defined(LINUX) || defined(APPLE)
    static void* s_TritonDllHandle;
#endif

    typedef bool (*Triton_QueryAcoustics_ptr)(OBJECT_HANDLE, ATKVectorF, ATKVectorF, TritonAcousticParameters*);
    static Triton_QueryAcoustics_ptr s_QueryAcoustics;

    typedef bool (*Triton_GetOutdoornessAtListener_ptr)(OBJECT_HANDLE, ATKVectorF, float*);
    static Triton_GetOutdoornessAtListener_ptr s_GetOutdoornessAtListener;
};