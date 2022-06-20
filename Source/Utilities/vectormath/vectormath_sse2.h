// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
#pragma once
#include "cputype.h"
#if defined(ARCH_X86) || defined(ARCH_X64)

namespace VectorMath
{
    namespace Arithmetic_Sse2
    {
        /* Sum two float vectors and store the result vector into the destination vector */
        void Add_32f(float* pDst, float const* pSrc1, float const* pSrc2, size_t const length);

        /* Sum three float vectors and store the result vector into the destination vector */
        void Add_32f(float* pDst, float const* pSrc1, float const* pSrc2, float const* pSrc3, size_t const length);

        /* Add float array to another */
        void Add_32f_I(float* pSrcDst, float const* pSrc, size_t const length);

        /* Sum two complex number float vectors and store the result vector into
        the destination vector */
        void Add_32fc(floatFC* pDst, floatFC const* pSrc1, floatFC const* pSrc2, size_t const length);

        /* Subtract two float vectors and store the result vector into the destination vector */
        void Sub_32f(float* pDst, float const* pSrc1, float const* pSrc2, size_t const length);

        /* Subtract two complex float vectors and store the result vector into the destination vector */
        void Sub_32fc(floatFC* pDst, floatFC const* pSrc1, floatFC const* pSrc2, size_t const length);

        /* multiply 2 arrays of complex floats numbers and save the result into
        the destination vector */
        void Mul_32fc(
            _Out_writes_(length) floatFC* pDst, _In_reads_(length) floatFC const* pSrc1,
            _In_reads_(length) floatFC const* pSrc2, _In_ size_t const length);

        /* multiply 2 arrays of floats numbers and save the result into
        the destination vector */
        void Mul_32f(
            _Out_writes_(length) float* pDst, _In_reads_(length) float const* pSrc1,
            _In_reads_(length) float const* pSrc2, _In_ size_t const length);

        /* multiply an array of floats on a constant. store the result into
        the destination array */
        void MulC_32f(
            _Out_writes_(length) float* pDst, _In_reads_(length) float const* pSrc, _In_ float const value,
            _In_ size_t const length);

        /* multiply two source vectors element by element and add the result to
        the destination vector */
        void AddProduct_32f(
            _Inout_updates_(length) float* pSrcDst, _In_reads_(length) float const* pSrc1,
            _In_reads_(length) float const* pSrc2, _In_ size_t const length);

        /* multiply two source vectors element by element and add the result to
        the destination vector */
        void AddProduct_32fc(
            _Inout_updates_(length) floatFC* pSrcDst, _In_reads_(length) floatFC const* pSrc1,
            _In_reads_(length) floatFC const* pSrc2, _In_ size_t const length);

        /* multiply source vector by scalar and accumulate result to SrcDst */
        void AddProductC_32f(
            _Inout_updates_(length) float* pSrcDst, _In_reads_(length) const float* pSrc, _In_ float scale,
            _In_ size_t length);

        /* multiply source vectors element by element, sum all result and save */
        void DotProd_32f(
            _Out_writes_(1) float* pDst, _In_reads_(length) float const* pSrc1, _In_reads_(length) float const* pSrc2,
            _In_ size_t const length);

        /* compute dot product of 3 vectors and 3 constants */
        void DotProdC_32f(
            _Out_writes_(length) float* pDst, _In_reads_(length) float const* pSrc1,
            _In_reads_(length) float const* pSrc2, _In_reads_(length) float const* pSrc3, _In_ float const val1,
            _In_ float const val2, _In_ float const val3, _In_ size_t length);

        /* solve the interpolation equation: result = a + (remainder * (b - a))*/
        void Interpolate_32f(
            _Inout_updates_(length) float* pDst, _In_reads_(length) const float* pSrcA,
            _In_reads_(length) float const* pSrcB, _In_reads_(length) const float* pSrcR, size_t length);

        /* Find index of max element in vector */
        uint32_t FindMaxIndex_32f(_In_ float* pVec, _In_ size_t const length);
    } // namespace Arithmetic_Sse2
} // namespace VectorMath
#endif // defined(_M_IX86) || defined(_M_X64)