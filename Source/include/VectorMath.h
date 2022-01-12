// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
#pragma once

#include <stdint.h>
#include <vector>
#include <cstddef>
#include <memory>

#include "UndefSAL.h"
#include "vectormath_floatfc.h"
#include "vectormath_interfaces.h"

namespace VectorMath
{
    // Buffers used with VectorMath functions must be aligned to at least the byte width returned here
    constexpr inline uint32_t GetMinimumRequiredAlignment()
    {
        return 16;
    }
} // namespace VectorMath