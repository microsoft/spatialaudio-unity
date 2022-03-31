// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
#pragma once

namespace VectorMath
{

    // Standard C++ implementation of a Fast Fourier Transform (FFT) for real-valued data
    class RealFft_generic : public IRealFft
    {
    public:
        RealFft_generic(unsigned int order);
        virtual ~RealFft_generic();

        virtual void ForwardFft(
            const float* timeDomainBuffer, size_t timeDomainLen, floatFC* freqDomainBuffer,
            size_t freqDomainLen) const override;

        virtual void InverseFft(
            const floatFC* freqDomainBuffer, size_t freqDomainLen, float* timeDomainBuffer,
            size_t timeDomainLen) const override;

        virtual unsigned int GetFreqDomainBufferLength() const noexcept override;
        virtual uint32_t GetTimeDomainBufferLength() const noexcept override;

    private:
        uint32_t m_Order;
        uint32_t m_OrderLog;
        mutable std::vector<floatFC> m_Wn;
        mutable std::vector<floatFC> m_WnInv;
        mutable std::vector<floatFC> m_TimeResult;
        mutable std::vector<floatFC> m_FreqResult;
        mutable std::vector<int> m_Bitidx;
    };

    // Standard C++ implementation of the vector math operations
    namespace Arithmetic_Generic
    {
        void Add_32f(float* pDst, float const* pSrc1, float const* pSrc2, size_t const length);

        void Add_32f(float* pDst, float const* pSrc1, float const* pSrc2, float const* pSrc3, size_t const length);

        void Add_32f_I(float* pSrcDst, float const* pSrc, size_t const length);

        void Add_32fc(floatFC* pDst, floatFC const* pSrc1, floatFC const* pSrc2, size_t const length);

        void Sub_32f(float* pDst, float const* pSrc1, float const* pSrc2, size_t const length);

        void Sub_32fc(floatFC* pDst, floatFC const* pSrc1, floatFC const* pSrc2, size_t const length);

        void Mul_32fc(
            _Out_writes_(length) floatFC* pDst, _In_reads_(length) floatFC const* pSrc1,
            _In_reads_(length) floatFC const* pSrc2, _In_ size_t const length);

        void Mul_32f(
            _Out_writes_(length) float* pDst, _In_reads_(length) float const* pSrc1,
            _In_reads_(length) float const* pSrc2, _In_ size_t const length);

        void MulC_32f(
            _Out_writes_(length) float* pDst, _In_reads_(length) float const* pSrc, _In_ float const value,
            _In_ size_t const length);

        void AddProduct_32f(
            _Inout_updates_(length) float* pSrcDst, _In_reads_(length) float const* pSrc1,
            _In_reads_(length) float const* pSrc2, _In_ size_t const length);

        void AddProduct_32fc(
            _Inout_updates_(length) floatFC* pSrcDst, _In_reads_(length) floatFC const* pSrc1,
            _In_reads_(length) floatFC const* pSrc2, _In_ size_t const length);

        void AddProductC_32f(
            _Inout_updates_(length) float* pSrcDst, _In_reads_(length) const float* pSrc, _In_ float scale,
            _In_ size_t length);

        void DotProd_32f(
            _Out_writes_(1) float* pDst, _In_reads_(length) float const* pSrc1, _In_reads_(length) float const* pSrc2,
            _In_ size_t const length);

        void DotProdC_32f(
            _Out_writes_(length) float* pDst, _In_reads_(length) float const* pSrc1,
            _In_reads_(length) float const* pSrc2, _In_reads_(length) float const* pSrc3, _In_ float const val1,
            _In_ float const val2, _In_ float const val3, _In_ size_t length);

        uint32_t FindMaxIndex_32f(_In_ float* pVec, _In_ size_t const length);

        void Interpolate_32f(
            _Inout_updates_(length) float* pDst, _In_reads_(length) const float* pSrcA,
            _In_reads_(length) float const* pSrcB, _In_reads_(length) const float* pSrcR, size_t length);

        void InterpolateC_32f(
            _Inout_updates_(length) float* pDst, _In_reads_(length) const float* pSrcA,
            _In_reads_(length) float const* pSrcB, _In_ const float remainder, size_t length);
    } // namespace Arithmetic_Generic

} // namespace VectorMath
