// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "vectormath.h"
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

    void FftBitreverse(const floatFC* S, floatFC* X, unsigned int N, const int* bitidx)
    {
        // Reorder according to bit reverse involution
        for (auto i = 0u; i < N; i++)
        {
            X[i].re = S[bitidx[i]].re;
            X[i].im = S[bitidx[i]].im;
        }
    }

    void FftBitreverseReal(const float* S, floatFC* X, unsigned int order, const int* bitidx)
    {
        // Reorder according to bit reverse involution
        for (auto i = 0u; i < order; i++)
        {
            X[i].re = S[bitidx[i]];
            X[i].im = 0.0f;
        }
    }

    // In-place complex FFT
    // Warning: Extremely poor performance. This placeholder implementation in FftCore() is here
    // only as a fallback when porting to new architectures. Targeted instruction set architectures
    // should be provided with an architecture-specific implementation.
    void FftCore(floatFC* X, const floatFC* wn, int orderLog)
    {
        auto order = 1u << orderLog;

        // 1st buterfly - no multiplication
        for (auto k = 0u; k < order - 1; k += 2)
        {
            auto r = X[k + 1];
            X[k + 1] = X[k] - r;
            X[k] = X[k] + r;
        }

        // Next radix 2 butterflies
        for (auto i = 1; i < orderLog; i++)
        {
            auto m = 1 << (orderLog - 1 - i); // number of butterflies for each Wn value
            auto sm = 1 << i;                 // Butterfly width / or number of unique Wn
            for (auto j = 0; j < sm; j++)
            {
                auto i1 = j;
                auto i2 = j + sm;
                auto n = (j * m) % order;
                for (auto k = 0; k < m; k++)
                {
                    auto r = wn[n] * X[i2];
                    X[i2] = X[i1] - r;
                    X[i1] = X[i1] + r;
                    i1 += 2 * sm;
                    i2 += 2 * sm;
                }
            }
        }
    }

    RealFft_generic::RealFft_generic(unsigned int order)
        : m_Order(order)
        , m_OrderLog(0u)
        , m_Wn(order)
        , m_WnInv(order)
        , m_TimeResult(order)
        , m_FreqResult(order)
        , m_Bitidx(order)
    {
        // Order should be positive power of 2
        auto orderLog = Logi2(order);
        m_OrderLog = orderLog;

        // Populate wn
        for (auto i = 0u; i < order; i++)
        {
            auto index = static_cast<float>(i);
            m_Wn[i].re = std::cos(2.0f * c_Pi * index / static_cast<float>(order));
            m_Wn[i].im = -std::sin(2.0f * c_Pi * index / static_cast<float>(order));
            m_WnInv[i].re = m_Wn[i].re;
            m_WnInv[i].im = -m_Wn[i].im;
        }

        // Populate bitreverse indexes
        for (auto i = 0u; i < order; i++)
        {
            auto k = 0u;
            for (auto j = 0u; j < orderLog; j++)
            {
                auto i1 = 1 << j;
                auto i2 = 1 << (orderLog - 1 - j);
                if (i & i1)
                {
                    k |= i2;
                }
                else
                {
                    k &= (~i2);
                }
            }
            m_Bitidx[i] = k;
        }
    }

    RealFft_generic::~RealFft_generic()
    {
    }

    void RealFft_generic::ForwardFft(
        const float* timeDomainBuffer, size_t timeDomainLen, floatFC* freqDomainBuffer, size_t freqDomainLen) const
    {
        if (freqDomainLen != (m_Order / 2 + 1) || timeDomainLen != m_Order)
        {
            throw std::invalid_argument(nullptr);
        }

        // Bit reverse real
        FftBitreverseReal(timeDomainBuffer, m_FreqResult.data(), m_Order, m_Bitidx.data());

        // Butterflies
        FftCore(m_FreqResult.data(), m_Wn.data(), m_OrderLog);

        // Copy non-redundant part of result to output
        memcpy(freqDomainBuffer, m_FreqResult.data(), this->GetFreqDomainBufferLength() * sizeof(floatFC));
    }

    void RealFft_generic::InverseFft(
        const floatFC* freqDomainBuffer, size_t freqDomainLen, float* timeDomainBuffer, size_t timeDomainLen) const
    {
        if (freqDomainLen != (m_Order / 2 + 1) || timeDomainLen != m_Order)
        {
            throw std::invalid_argument(nullptr);
        }

        // Copy to work area and extend redundant frequencies
        memcpy(m_FreqResult.data(), freqDomainBuffer, freqDomainLen * sizeof(floatFC));
        auto j = 2u;
        for (auto i = this->GetFreqDomainBufferLength(); i < m_Order; i++)
        {
            m_FreqResult[i] = ComplexConjugate(m_FreqResult[i - j]);
            j += 2;
        }

        // Bit reverse
        FftBitreverse(m_FreqResult.data(), m_TimeResult.data(), m_Order, m_Bitidx.data());

        // Butterflies
        FftCore(m_TimeResult.data(), m_WnInv.data(), m_OrderLog);

        // Scale and copy to output
        for (auto i = 0u; i < m_Order; i++)
        {
            timeDomainBuffer[i] = m_TimeResult[i].re / m_Order;
        }
    }

    unsigned int RealFft_generic::GetFreqDomainBufferLength() const noexcept
    {
        // Careful about 'simplifying' this, the operation takes advantage of
        // integer rounding in the divide
        return (m_Order / 2 + 1);
    }

    uint32_t RealFft_generic::GetTimeDomainBufferLength() const noexcept
    {
        return m_Order;
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

        void Add_32f(float* pDst, const float* pSrc1, const float* pSrc2, const float* pSrc3, size_t const length)
        {
            size_t i;

            for (i = 0; i < length; i += 1)
            {
                pDst[i] = pSrc1[i] + pSrc2[i] + pSrc3[i];
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

        void Sub_32f(float* pDst, const float* pSrc1, const float* pSrc2, size_t const length)
        {
            size_t i;

            for (i = 0; i < length; i += 1)
            {
                pDst[i] = pSrc1[i] - pSrc2[i];
            }
        }

        void Sub_32fc(floatFC* pDst, const floatFC* pSrc1, const floatFC* pSrc2, size_t const length)
        {
            size_t i;

            for (i = 0; i < length; i += 1)
            {
                pDst[i] = pSrc1[i] - pSrc2[i];
            }
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

        _Use_decl_annotations_ void DotProdC_32f(
            float* pDst, float const* pSrc1, float const* pSrc2, float const* pSrc3, float const val1, float const val2,
            float const val3, size_t length)
        {
            size_t i;

            for (i = 0; i < length; i++)
            {
                pDst[i] = (pSrc1[i] * val1) + (pSrc2[i] * val2) + (pSrc3[i] * val3);
            }
        }

        _Use_decl_annotations_ uint32_t FindMaxIndex_32f(_In_ float* pVec, _In_ size_t const length)
        {
            if (length == 0)
            {
                return 0;
            }

            size_t i;
            float maxValue = pVec[0];
            size_t maxIndex = 0;

            for (i = 1; i < length; i += 1)
            {
                if (pVec[i] >= maxValue)
                {
                    maxValue = pVec[i];
                    maxIndex = i;
                }
            }

            return static_cast<uint32_t>(maxIndex);
        }

        // result = a * (1 - remainder) + b * (remainder);
        // Or factor out remainder: result = a + (remainder * (b - a));
        _Use_decl_annotations_ void
        Interpolate_32f(float* pDst, const float* pSrcA, float const* pSrcB, const float* pSrcR, size_t length)
        {
            size_t i = 0;

            for (i = 0; i < length; i++)
            {
                pDst[i] = pSrcA[i] + (pSrcR[i] * (pSrcB[i] - pSrcA[i]));
            }
        }

        // result = a * (1 - remainder) + b * (remainder);
        // Or factor out remainder: result = a + (remainder * (b - a));
        _Use_decl_annotations_ void
        InterpolateC_32f(float* pDst, const float* pSrcA, float const* pSrcB, const float remainder, size_t length)
        {
            size_t i = 0;

            for (i = 0; i < length; i++)
            {
                pDst[i] = pSrcA[i] + (remainder * (pSrcB[i] - pSrcA[i]));
            }
        }
    } // namespace Arithmetic_Generic
} // namespace VectorMath
