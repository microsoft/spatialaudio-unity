// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "VectorMath.h"
#include "vectormath_generic.h"
#include <cstring>
#include <cmath>
#include <stdexcept>

namespace VectorMath
{

    floatFC ComplexConjugate(floatFC a)
    {
        auto b = a;
        b.im = -b.im;
        return b;
    }

    floatFC operator*(floatFC a, floatFC b)
    {
        auto real = a.re * b.re - a.im * b.im;
        auto imag = a.re * b.im + a.im * b.re;
        return floatFC{real, imag};
    }

    floatFC operator+(floatFC a, floatFC b)
    {
        auto real = a.re + b.re;
        auto imag = a.im + b.im;
        return floatFC{real, imag};
    }

    floatFC operator-(floatFC a, floatFC b)
    {
        auto real = a.re - b.re;
        auto imag = a.im - b.im;
        return floatFC{real, imag};
    }

    unsigned int Logi2(unsigned int N)
    {
        auto Nlog = 0u;
        while ((1u << Nlog) < N)
        {
            Nlog++;
        }
        return Nlog;
    }

    namespace Arithmetic_Generic
    {
        void Add_32f(float* pDst, const float* pSrc1, const float* pSrc2, size_t const length)
        {
            size_t i;

            for (i = 0; i < length; i += 1)
            {
                pDst[i] = pSrc1[i] + pSrc2[i];
            }
        }

        void Add_32f_I(float* pSrcDst, const float* pSrc, size_t length)
        {
            Add_32f(pSrcDst, pSrcDst, pSrc, length);
        }

        void Add_32fc(floatFC* pDst, const floatFC* pSrc1, const floatFC* pSrc2, const size_t length)
        {
            Add_32f(
                reinterpret_cast<float*>(pDst),
                reinterpret_cast<const float*>(pSrc1),
                reinterpret_cast<const float*>(pSrc2),
                length * 2);
        }

        /* The implementations of the below functions were taken from
         * %sdxroot%\avcore\mf\dll\wmperfdll\*.cpp.
         * They are copied here rather than called because that library is not built for ARM. */
        _Use_decl_annotations_ void
        Mul_32fc(floatFC* pDst, floatFC const* pSrc1, floatFC const* pSrc2, size_t const length)
        {
            size_t i;

            for (i = 0; i < length; i += 1)
            {
                floatFC tmp;

                tmp.re = pSrc1[i].re * pSrc2[i].re - pSrc1[i].im * pSrc2[i].im;
                tmp.im = pSrc1[i].re * pSrc2[i].im + pSrc1[i].im * pSrc2[i].re;

                pDst[i] = tmp;
            }
        }

        _Use_decl_annotations_ void Mul_32f(float* pDst, float const* pSrc1, float const* pSrc2, size_t const length)
        {
            size_t i;

            for (i = 0; i < length; i += 1)
            {
                pDst[i] = pSrc1[i] * pSrc2[i];
            }
        }

        _Use_decl_annotations_ void MulC_32f(float* pDst, float const* pSrc, float const value, size_t const length)
        {
            if (1.0f == value)
            {
                memcpy(pDst, pSrc, length * sizeof(float));
                return;
            }
            else if (0.0f == value)
            {
                memset(pDst, 0, length * sizeof(float));
                return;
            }

            size_t i;

            for (i = 0; i < length; i += 1)
            {
                pDst[i] = pSrc[i] * value;
            }
        }

        _Use_decl_annotations_ void
        AddProduct_32f(float* pSrcDst, float const* pSrc1, float const* pSrc2, size_t const length)
        {
            size_t i;

            for (i = 0; i < length; i += 1)
            {
                pSrcDst[i] = pSrcDst[i] + pSrc1[i] * pSrc2[i];
            }
        }

        _Use_decl_annotations_ void
        AddProduct_32fc(floatFC* pSrcDst, floatFC const* pSrc1, floatFC const* pSrc2, size_t const length)
        {
            size_t i;

            for (i = 0; i < length; i += 1)
            {
                floatFC tmp;

                tmp.re = pSrc1[i].re * pSrc2[i].re - pSrc1[i].im * pSrc2[i].im;
                tmp.im = pSrc1[i].re * pSrc2[i].im + pSrc1[i].im * pSrc2[i].re;

                pSrcDst[i].re += tmp.re;
                pSrcDst[i].im += tmp.im;
            }
        }

        _Use_decl_annotations_ void AddProductC_32f(float* pSrcDst, const float* pSrc, float scale, size_t length)
        {
            size_t i;

            for (i = 0; i < length; i += 1)
            {
                pSrcDst[i] = pSrcDst[i] + pSrc[i] * scale;
            }
        }

        _Use_decl_annotations_ void
        DotProd_32f(float* pDst, float const* pSrc1, float const* pSrc2, size_t const length)
        {
            size_t i;
            float dst = 0.0f;

            for (i = 0; i < length; i += 1)
            {
                dst += pSrc1[i] * pSrc2[i];
            }

            *pDst = dst;
        }
    } // namespace Arithmetic_Generic
} // namespace VectorMath
