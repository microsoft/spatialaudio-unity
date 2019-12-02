// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "vectormath.h"
#include "vectormath_generic.h"
#include "vectormath_sse2.h"
#include "vectormath_neon.h"

namespace VectorMath
{
    namespace Arithmetic
    {
        // Platform abstraction for stateless math functions
        // the following forward calls to the platform-specific optimizations
        void Add_32f(float* pDst, float const* pSrc1, float const* pSrc2, size_t const length)
        {
#if defined(ARCH_X86) || defined(ARCH_X64)
            Arithmetic_Sse2::Add_32f(pDst, pSrc1, pSrc2, length);
#elif defined(ARCH_ARM) || defined(ARCH_ARM64)
            Arithmetic_Neon::Add_32f(pDst, pSrc1, pSrc2, length);
#else
            Arithmetic_Generic::Add_32f(pDst, pSrc1, pSrc2, length);
#endif
        }

        /* Add float array to another */
        void Add_32f_I(float* pSrcDst, float const* pSrc, size_t const length)
        {
#if defined(ARCH_X86) || defined(ARCH_X64)
            Arithmetic_Sse2::Add_32f_I(pSrcDst, pSrc, length);
#elif defined(ARCH_ARM) || defined(ARCH_ARM64)
            Arithmetic_Neon::Add_32f_I(pSrcDst, pSrc, length);
#else
            Arithmetic_Generic::Add_32f_I(pSrcDst, pSrc, length);
#endif
        }

        /* Sum two complex number float vectors and store the result vector into
        the destination vector */
        void Add_32fc(floatFC* pDst, floatFC const* pSrc1, floatFC const* pSrc2, size_t const length)
        {
#if defined(ARCH_X86) || defined(ARCH_X64)
            Arithmetic_Sse2::Add_32fc(pDst, pSrc1, pSrc2, length);
#elif defined(ARCH_ARM) || defined(ARCH_ARM64)
            Arithmetic_Neon::Add_32fc(pDst, pSrc1, pSrc2, length);
#else
            Arithmetic_Generic::Add_32fc(pDst, pSrc1, pSrc2, length);
#endif
        }

        /* Add complex array to another */
        void Add_32fc_I(floatFC* pSrcDst, floatFC const* pSrc, size_t const length)
        {
#if defined(ARCH_X86) || defined(ARCH_X64)
            Arithmetic_Sse2::Add_32f_I(
                reinterpret_cast<float*>(pSrcDst), reinterpret_cast<const float*>(pSrc), length * 2);
#elif defined(ARCH_ARM) || defined(ARCH_ARM64)
            Arithmetic_Neon::Add_32f_I(
                reinterpret_cast<float*>(pSrcDst), reinterpret_cast<const float*>(pSrc), length * 2);
#else
            Arithmetic_Generic::Add_32f_I(
                reinterpret_cast<float*>(pSrcDst), reinterpret_cast<const float*>(pSrc), length * 2);
#endif
        }

        /* multiply 2 arrays of complex floats numbers and save the result into
        the destination vector */
        void Mul_32fc(
            _Out_writes_(length) floatFC* pDst, _In_reads_(length) floatFC const* pSrc1,
            _In_reads_(length) floatFC const* pSrc2, _In_ size_t const length)
        {
#if defined(ARCH_X86) || defined(ARCH_X64)
            Arithmetic_Sse2::Mul_32fc(pDst, pSrc1, pSrc2, length);
#elif defined(ARCH_ARM) || defined(ARCH_ARM64)
            Arithmetic_Neon::Mul_32fc(pDst, pSrc1, pSrc2, length);
#else
            Arithmetic_Generic::Mul_32fc(pDst, pSrc1, pSrc2, length);
#endif
        }

        /* multiply 2 arrays of floats numbers and save the result into
        the destination vector */
        void Mul_32f(
            _Out_writes_(length) float* pDst, _In_reads_(length) float const* pSrc1,
            _In_reads_(length) float const* pSrc2, _In_ size_t const length)
        {
#if defined(ARCH_X86) || defined(ARCH_X64)
            Arithmetic_Sse2::Mul_32f(pDst, pSrc1, pSrc2, length);
#elif defined(ARCH_ARM) || defined(ARCH_ARM64)
            Arithmetic_Neon::Mul_32f(pDst, pSrc1, pSrc2, length);
#else
            Arithmetic_Generic::Mul_32f(pDst, pSrc1, pSrc2, length);
#endif
        }

        /* multiply an array of floats on a constant. store the result into
        the destination array */
        void MulC_32f(
            _Out_writes_(length) float* pDst, _In_reads_(length) float const* pSrc, _In_ float const value,
            _In_ size_t const length)
        {
#if defined(ARCH_X86) || defined(ARCH_X64)
            Arithmetic_Sse2::MulC_32f(pDst, pSrc, value, length);
#elif defined(ARCH_ARM) || defined(ARCH_ARM64)
            Arithmetic_Neon::MulC_32f(pDst, pSrc, value, length);
#else
            Arithmetic_Generic::MulC_32f(pDst, pSrc, value, length);
#endif
        }

        /* multiply an array of complex floats with a constant. store the result into
        the destination array */
        void MulC_32fc(
            _Out_writes_(length) floatFC* pDst, _In_reads_(length) floatFC const* pSrc, _In_ float const value,
            _In_ size_t const length)
        {
#if defined(ARCH_X86) || defined(ARCH_X64)
            Arithmetic_Sse2::MulC_32f(
                reinterpret_cast<float*>(pDst), reinterpret_cast<const float*>(pSrc), value, length * 2);
#elif defined(ARCH_ARM) || defined(ARCH_ARM64)
            Arithmetic_Neon::MulC_32f(
                reinterpret_cast<float*>(pDst), reinterpret_cast<const float*>(pSrc), value, length * 2);
#else
            Arithmetic_Generic::MulC_32f(
                reinterpret_cast<float*>(pDst), reinterpret_cast<const float*>(pSrc), value, length * 2);
#endif
        }

        /* multiply two source vectors element by element and add the result to
        the destination vector */
        void AddProduct_32f(
            _Inout_updates_(length) float* pSrcDst, _In_reads_(length) float const* pSrc1,
            _In_reads_(length) float const* pSrc2, _In_ size_t const length)
        {
#if defined(ARCH_X86) || defined(ARCH_X64)
            Arithmetic_Sse2::AddProduct_32f(pSrcDst, pSrc1, pSrc2, length);
#elif defined(ARCH_ARM) || defined(ARCH_ARM64)
            Arithmetic_Neon::AddProduct_32f(pSrcDst, pSrc1, pSrc2, length);
#else
            Arithmetic_Generic::AddProduct_32f(pSrcDst, pSrc1, pSrc2, length);
#endif
        }

        /* multiply two source vectors element by element and add the result to
        the destination vector */
        void AddProduct_32fc(
            _Inout_updates_(length) floatFC* pSrcDst, _In_reads_(length) floatFC const* pSrc1,
            _In_reads_(length) floatFC const* pSrc2, _In_ size_t const length)
        {
#if defined(ARCH_X86) || defined(ARCH_X64)
            Arithmetic_Sse2::AddProduct_32fc(pSrcDst, pSrc1, pSrc2, length);
#elif defined(ARCH_ARM) || defined(ARCH_ARM64)
            Arithmetic_Neon::AddProduct_32fc(pSrcDst, pSrc1, pSrc2, length);
#else
            Arithmetic_Generic::AddProduct_32fc(pSrcDst, pSrc1, pSrc2, length);
#endif
        }

        /* multiply source vector by scalar and accumulate result to SrcDst */
        void AddProductC_32f(
            _Inout_updates_(length) float* pSrcDst, _In_reads_(length) const float* pSrc, _In_ float scale,
            _In_ size_t length)
        {
#if defined(ARCH_X86) || defined(ARCH_X64)
            Arithmetic_Sse2::AddProductC_32f(pSrcDst, pSrc, scale, length);
#elif defined(ARCH_ARM) || defined(ARCH_ARM64)
            Arithmetic_Neon::AddProductC_32f(pSrcDst, pSrc, scale, length);
#else
            Arithmetic_Generic::AddProductC_32f(pSrcDst, pSrc, scale, length);
#endif
        }

        /* multiply source vectors element by element, sum all result and save */
        void DotProd_32f(
            _Out_writes_(1) float* pDst, _In_reads_(length) float const* pSrc1, _In_reads_(length) float const* pSrc2,
            _In_ size_t const length)
        {
#if defined(ARCH_X86) || defined(ARCH_X64)
            Arithmetic_Sse2::DotProd_32f(pDst, pSrc1, pSrc2, length);
#elif defined(ARCH_ARM) || defined(ARCH_ARM64)
            Arithmetic_Neon::DotProd_32f(pDst, pSrc1, pSrc2, length);
#else
            Arithmetic_Generic::DotProd_32f(pDst, pSrc1, pSrc2, length);
#endif
        }

    } // namespace Arithmetic
} // namespace VectorMath
