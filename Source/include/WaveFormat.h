// Copyright (c) Microsoft Corporation.  All rights reserved.
// Licensed under the MIT License.
#pragma once

#include <cstdint>
#define INITGUID
#include "guid.h"
#include "UndefSAL.h"

#if defined(WINDOWS)

#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include "mmreg.h"

#else

// Supply WAVEFORMAT structures and consts on non-Windows platforms
#pragma pack(push, 1)
#define WAVE_FORMAT_PCM 1
#define WAVE_FORMAT_IEEE_FLOAT 3

typedef struct waveformat_tag
{
    unsigned short wFormatTag;     /* format type */
    unsigned short nChannels;      /* number of channels (i.e. mono, stereo, etc.) */
    unsigned long nSamplesPerSec;  /* sample rate */
    unsigned long nAvgBytesPerSec; /* for buffer estimation */
    unsigned short nBlockAlign;    /* block size of data */
} WAVEFORMAT;

/* flags for wFormatTag field of WAVEFORMAT */
#define WAVE_FORMAT_PCM 1

/* specific waveform format structure for PCM data */
typedef struct pcmwaveformat_tag
{
    WAVEFORMAT wf;
    unsigned short wBitsPerSample;
} PCMWAVEFORMAT;

#ifndef _WAVEFORMATEX_
#define _WAVEFORMATEX_

typedef struct tWAVEFORMATEX
{
    unsigned short wFormatTag;     /* format type */
    unsigned short nChannels;      /* number of channels (i.e. mono, stereo...) */
    unsigned long nSamplesPerSec;  /* sample rate */
    unsigned long nAvgBytesPerSec; /* for buffer estimation */
    unsigned short nBlockAlign;    /* block size of data */
    unsigned short wBitsPerSample; /* Number of bits per sample of mono data */
    unsigned short cbSize;         /* The count in bytes of the size of
                                extra information (after cbSize) */
} WAVEFORMATEX;
#endif

typedef struct
{
    WAVEFORMATEX Format;
    union {
        unsigned short wValidBitsPerSample; /* bits of precision  */
        unsigned short wSamplesPerBlock;    /* valid if wBitsPerSample==0 */
        unsigned short wReserved;           /* If neither applies, set to zero. */
    } Samples;
    unsigned long dwChannelMask; /* which channels are */
                                 /* present in stream  */
    GUID SubFormat;
} WAVEFORMATEXTENSIBLE;

#if !defined(WAVE_FORMAT_EXTENSIBLE)
#define WAVE_FORMAT_EXTENSIBLE 0xFFFE /* Microsoft */
#endif                                // !defined(WAVE_FORMAT_EXTENSIBLE)

#pragma pack(pop)

#endif // !defined(WINDOWS)
