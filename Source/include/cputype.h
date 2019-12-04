// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
#pragma once

#if _MSC_VER
#if defined(_M_X86)
#define ARCH_X86
#elif defined(_M_X64)
#define ARCH_X64
#elif defined(_M_ARM)
#define ARCH_ARM
#elif defined(_M_ARM64)
#define ARCH_ARM64
#endif
#endif

#if __GNUC__
#if defined(__i386__)
#define ARCH_X86
#elif defined(__ia64__) || defined(__x86_64__)
#define ARCH_X64
// Check for ARM64 first because the __arm__ flag for ARM32 is also defined for ARM64
#elif defined(__aarch64__)
#define ARCH_ARM64
#elif defined(__arm__)
#define ARCH_ARM
#endif
#endif
