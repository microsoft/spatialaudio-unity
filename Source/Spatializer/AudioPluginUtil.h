#pragma once

#include "AudioPluginInterface.h"

#ifdef WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <pthread.h>
#define strcpy_s strcpy
#endif

typedef int (*InternalEffectDefinitionRegistrationCallback)(UnityAudioEffectDefinition& desc);

void RegisterParameter(
    UnityAudioEffectDefinition& desc, const char* name, const char* unit, float minval, float maxval, float defaultval,
    float displayscale, float displayexponent, int enumvalue, const char* description = NULL);

void InitParametersFromDefinitions(
    InternalEffectDefinitionRegistrationCallback registereffectdefcallback, float* params);

void DeclareEffect(
    UnityAudioEffectDefinition& desc, const char* name, UnityAudioEffect_CreateCallback createcallback,
    UnityAudioEffect_ReleaseCallback releasecallback, UnityAudioEffect_ProcessCallback processcallback,
    UnityAudioEffect_SetFloatParameterCallback setfloatparametercallback,
    UnityAudioEffect_GetFloatParameterCallback getfloatparametercallback,
    UnityAudioEffect_GetFloatBufferCallback getfloatbuffercallback,
    InternalEffectDefinitionRegistrationCallback registereffectdefcallback);
