// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
#pragma once

namespace VectorMath
{
   namespace Arithmetic
    {
        /* Sum two float vectors and store the result vector into the destination vector */
        void Add_32f(float* pDst, float const* pSrc1, float const* pSrc2, size_t const length);

        /* Add float array to another */
        void Add_32f_I(float* pSrcDst, float const* pSrc, size_t const length);

        /* Add two complex arrays in place*/
        void Add_32fc_I(floatFC* pSrcDst, floatFC const* pSrc, size_t const length);

        /* Sum two complex number float vectors and store the result vector into
        the destination vector */
        void Add_32fc(floatFC* pDst, floatFC const* pSrc1, floatFC const* pSrc2, size_t const length);

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

        /* multiply an array of complex numbers on a constant. store the result into
        the destination array */
        void MulC_32fc(
            _Out_writes_(length) floatFC* pDst, _In_reads_(length) floatFC const* pSrc, _In_ float const value,
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
    } // namespace Arithmetic

} // namespace VectorMath
