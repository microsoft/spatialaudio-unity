// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
#pragma once

#define RETURN_RESULT_IF_NULL(ptr, ...) \
    if (ptr == nullptr) \
    { \
        return __VA_ARGS__; \
    }

#define RETURN_RESULT_IF_TRUE(condition, result) \
    if (condition) \
    { \
        return result; \
    }

#define RETURN_FALSE_IF_TRUE(condition) \
    RETURN_RESULT_IF_TRUE(condition, false)


#define THROW_IF_FALSE(condition, exception, ...) \
    if (!(condition)) \
    { \
        throw exception(__VA_ARGS__); \
    }

// Clang formatting causes Android build warnings: need a newline at end of file
// clang-format off

