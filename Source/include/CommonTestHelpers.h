// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
#pragma once

#include <cmath>
#include "gtest.h"

#define HRTF_2PI 6.28318530f

inline bool AreFloatsTooFarApart(float one, float two, float zeroTolerance = 10E-6, float relativeTolerance = 10E-4)
{
    if (one == 0)
    {
        if (std::fabs(two) > zeroTolerance)
        {
            return true;
        }
    }
    else if (std::fabs((one - two) / one) > relativeTolerance)
    {
        return true;
    }
    return false;
}

// Checks absolute difference between one and two
inline bool CheckEqual(float one, float two, float tolerance)
{
    return std::fabs(one - two) < tolerance;
}