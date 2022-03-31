// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
#pragma once

namespace VectorMath
{

    // Interface to a Fast Fourier Transform (FFT) for real-valued data.
    class IRealFft
    {
    public:
        // Forward FFT. Use GetFreqDomainBufferLength() to determine how many complex
        // numbers to allocate for freqDomainBuffer.
        virtual void ForwardFft(
            const float* timeDomainBuffer, size_t timeDomainLen, floatFC* freqDomainBuffer,
            size_t freqDomainLen) const = 0;

        // Inverse FFT. Use GetFreqDomainBufferLength() to determine how many complex
        // numbers to allocate for freqDomainBuffer.
        virtual void InverseFft(
            const floatFC* freqDomainBuffer, size_t freqDomainLen, float* timeDomainBuffer,
            size_t timeDomainLen) const = 0;

        // Return the number of complex numbers comprising one frequency domain vector.
        virtual unsigned int GetFreqDomainBufferLength() const noexcept = 0;
        virtual uint32_t GetTimeDomainBufferLength() const noexcept = 0;

        virtual ~IRealFft(){};

        /* On the topic of frequency domain data storage:
         * Frequency domain data used by objects implementing this interface shall be stored
         * in Intel's CCS data format. This format stores the first non-redundant complex numbers
         * from the theoretical FFT result. For example, for a 1024-point FFT, the frequency
         * domain output is 513 complex numbers, i.e. 1026 floats.
         *
         * The imaginary parts of the first and last elements are therefore always zero.
         */
    };

    namespace Arithmetic
    {
        /* Sum two float vectors and store the result vector into the destination vector */
        void Add_32f(float* pDst, float const* pSrc1, float const* pSrc2, size_t const length);

        /* Sum three float vectors and store the result vector into the destination vector */
        void Add_32f(float* pDst, float const* pSrc1, float const* pSrc2, float const* pSrc3, size_t const length);

        /* Add float array to another */
        void Add_32f_I(float* pSrcDst, float const* pSrc, size_t const length);

        /* Add two complex arrays in place*/
        void Add_32fc_I(floatFC* pSrcDst, floatFC const* pSrc, size_t const length);

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

        /* Multiply source vectors by corresponding constant, sum the results, and save to output vector */
        void DotProdC_32f(
            _Out_ float* pDst, _In_reads_(length) float const* pSrc1, _In_reads_(length) float const* pSrc2,
            _In_reads_(length) float const* pSrc3, _In_ float const val1, _In_ float const val2, _In_ float const val3,
            _In_ size_t length);

        /* Find index of max element in vector */
        uint32_t FindMaxIndex_32f(_In_ float* pVec, _In_ size_t const length);

        /* Solve the modified interpolation equation: a + (remainder * (b - a)) */
        void Interpolate_32f(
            _Inout_updates_(length) float* pDst, _In_reads_(length) const float* pSrcA,
            _In_reads_(length) float const* pSrcB, _In_reads_(length) const float* pSrcR, size_t length);

        /* Solve the modified interpolation equation: a + (remainder * (b - a)) */
        void InterpolateC_32f(
            _Inout_updates_(length) float* pDst, _In_reads_(length) const float* pSrcA,
            _In_reads_(length) float const* pSrcB, _In_ float const remainder, size_t length);
    } // namespace Arithmetic

} // namespace VectorMath
