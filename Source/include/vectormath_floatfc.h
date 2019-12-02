// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
#pragma once

namespace VectorMath
{
    struct floatFC
    {
        // real part
        float re;
        // imaginary part
        float im;
    };

    // Constants
    constexpr auto c_Pi = 3.141592658f;

    unsigned int Logi2(unsigned int N); // Log base 2
    floatFC ComplexConjugate(floatFC a);
    floatFC operator*(floatFC a, floatFC b);
    floatFC operator+(floatFC a, floatFC b);
    floatFC operator-(floatFC a, floatFC b);

} // namespace VectorMath
