// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "vectormath.h"
#include "vectormath_generic.h"
#include "vectormath_sse2.h"
#include "vectormath_neon.h"

#ifdef VECTORMATH_FFTW
class FftwCleanupHandler final
{
public:
    ~FftwCleanupHandler()
    {
        fftwf_cleanup();
    }
};
// FFTW cleanup needs to happen when all FFTW plans have been guaranteed to be destroyed.
static FftwCleanupHandler g_FftwCleanupHandler;
#endif

namespace VectorMath
{

    std::unique_ptr<IRealFft> CreateRealFft(unsigned int order)
    {
#if defined(VECTORMATH_FFTW)
        return std::unique_ptr<IRealFft>(new FftwWrapper(order));
#else
        return std::unique_ptr<IRealFft>(new RealFft_generic(order));
#endif
    }

    std::shared_ptr<IRealFft> CreateSharedRealFft(unsigned int order)
    {
#if defined(VECTORMATH_FFTW)
        return std::shared_ptr<IRealFft>(new FftwWrapper(order));
#else
        return std::shared_ptr<IRealFft>(new RealFft_generic(order));
#endif
    }

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

        void Add_32f(float* pDst, float const* pSrc1, float const* pSrc2, float const* pSrc3, size_t const length)
        {
#if defined(ARCH_X86) || defined(ARCH_X64)
            Arithmetic_Sse2::Add_32f(pDst, pSrc1, pSrc2, pSrc3, length);
#elif defined(ARCH_ARM) || defined(ARCH_ARM64)
            Arithmetic_Neon::Add_32f(pDst, pSrc1, pSrc2, pSrc3, length);
#else
            Arithmetic_Generic::Add_32f(pDst, pSrc1, pSrc2, pSrc3, length);
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

        void Sub_32f(float* pDst, float const* pSrc1, float const* pSrc2, size_t const length)
        {
#if defined(ARCH_X86) || defined(ARCH_X64)
            Arithmetic_Sse2::Sub_32f(pDst, pSrc1, pSrc2, length);
#elif defined(ARCH_ARM) || defined(ARCH_ARM64)
            Arithmetic_Neon::Sub_32f(pDst, pSrc1, pSrc2, length);
#else
            Arithmetic_Generic::Sub_32f(pDst, pSrc1, pSrc2, length);
#endif
        }

        void Sub_32fc(floatFC* pDst, floatFC const* pSrc1, floatFC const* pSrc2, size_t const length)
        {
#if defined(ARCH_X86) || defined(ARCH_X64)
            Arithmetic_Sse2::Sub_32fc(pDst, pSrc1, pSrc2, length);
#elif defined(ARCH_ARM) || defined(ARCH_ARM64)
            Arithmetic_Neon::Sub_32fc(pDst, pSrc1, pSrc2, length);
#else
            Arithmetic_Generic::Sub_32fc(pDst, pSrc1, pSrc2, length);
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

        /* Multiply source vectors by corresponding constant, sum the results, and save to output vector */
        _Use_decl_annotations_ void DotProdC_32f(
            float* pDst, float const* pSrc1, float const* pSrc2, float const* pSrc3, float const val1, float const val2,
            float const val3, size_t length)
        {
#if defined(ARCH_X86) || defined(ARCH_X64)
            Arithmetic_Sse2::DotProdC_32f(pDst, pSrc1, pSrc2, pSrc3, val1, val2, val3, length);
#elif defined(ARCH_ARM) || defined(ARCH_ARM64)
            Arithmetic_Neon::DotProdC_32f(pDst, pSrc1, pSrc2, pSrc3, val1, val2, val3, length);
#else
            Arithmetic_Generic::DotProdC_32f(pDst, pSrc1, pSrc2, pSrc3, val1, val2, val3, length);
#endif
        }

        /* Find index of max element in vector */
        _Use_decl_annotations_ uint32_t FindMaxIndex_32f(float* pVec, size_t const length)
        {
#if defined(ARCH_X86) || defined(ARCH_X64)
            return Arithmetic_Sse2::FindMaxIndex_32f(pVec, length);
#elif defined(ARCH_ARM) || defined(ARCH_ARM64)
            return Arithmetic_Neon::FindMaxIndex_32f(pVec, length);
#else
            return Arithmetic_Generic::FindMaxIndex_32f(pVec, length);
#endif
        }

    } // namespace Arithmetic
} // namespace VectorMath
