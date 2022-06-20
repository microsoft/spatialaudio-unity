// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "cputype.h"
#if defined(ARCH_ARM) || defined(ARCH_ARM64)

#include "vectormath.h"
#include "vectormath_neon.h"
// For the unaligned portions, some functions fall back to the generic impl
#include "vectormath_generic.h"
#if defined(ARCH_ARM64) && !defined(__clang__)
#include <arm64_neon.h>
#else
#include <arm_neon.h>
#endif

namespace VectorMath
{
    namespace Arithmetic_Neon
    {
        // helper functions
        inline float32x4x2_t NeonComplexAdd(float32x4x2_t src1, float32x4x2_t src2)
        {
            float32x4x2_t result;
            result.val[0] = vaddq_f32(src1.val[0], src2.val[0]);
            result.val[1] = vaddq_f32(src1.val[1], src2.val[1]);
            return result;
        }

        inline float32x4x2_t NeonComplexMultiply(float32x4x2_t src1, float32x4x2_t src2)
        {
            float32x4x2_t result;
            result.val[0] = vmulq_f32(src1.val[0], src2.val[0]);
            result.val[0] = vmlsq_f32(result.val[0], src1.val[1], src2.val[1]);
            result.val[1] = vmulq_f32(src1.val[0], src2.val[1]);
            result.val[1] = vmlaq_f32(result.val[1], src1.val[1], src2.val[0]);
            return result;
        }

        inline float32x4x2_t NeonComplexMultiplyAdd(float32x4x2_t src1, float32x4x2_t src2, float32x4x2_t src3)
        {
            float32x4x2_t addend = NeonComplexMultiply(src1, src2);
            return NeonComplexAdd(addend, src3);
        }

        void Add_32f(float* pDst, const float* pSrc1, const float* pSrc2, size_t const length)
        {
            auto i = 0u;
            for (; i + 4 <= length; i += 4)
            {
                auto src1 = vld1q_f32(pSrc1 + i);
                auto src2 = vld1q_f32(pSrc2 + i);
                auto dst = vaddq_f32(src1, src2);
                vst1q_f32(pDst + i, dst);
            }

            if (i < length)
            {
                Arithmetic_Generic::Add_32f(pDst + i, pSrc1 + i, pSrc2 + i, length - i);
            }
        }

        void Add_32f(float* pDst, const float* pSrc1, const float* pSrc2, const float* pSrc3, size_t const length)
        {
            auto i = 0u;
            for (; i + 4 <= length; i += 4)
            {
                auto src1 = vld1q_f32(pSrc1 + i);
                auto src2 = vld1q_f32(pSrc2 + i);
                auto src3 = vld1q_f32(pSrc3 + i);
                auto temp = vaddq_f32(src1, src2);
                auto dst = vaddq_f32(temp, src3);
                vst1q_f32(pDst + i, dst);
            }

            if (i < length)
            {
                Arithmetic_Generic::Add_32f(pDst + i, pSrc1 + i, pSrc2 + i, pSrc3 + i, length - i);
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
            auto i = 0u;
            for (; i + 4 <= length; i += 4)
            {
                auto src1 = vld1q_f32(pSrc1 + i);
                auto src2 = vld1q_f32(pSrc2 + i);
                auto dst = vsubq_f32(src1, src2);
                vst1q_f32(pDst + i, dst);
            }

            if (i < length)
            {
                Arithmetic_Generic::Sub_32f(pDst + i, pSrc1 + i, pSrc2 + i, length - i);
            }
        }

        void Sub_32fc(floatFC* pDst, const floatFC* pSrc1, const floatFC* pSrc2, const size_t length)
        {
            Sub_32f((float*) pDst, (float const*) pSrc1, (float const*) pSrc2, length * 2);
        }

        _Use_decl_annotations_ void
        Mul_32fc(floatFC* pDst, floatFC const* pSrc1, floatFC const* pSrc2, size_t const length)
        {
            auto i = 0u;
            for (; i + 4 <= length; i += 4)
            {
                auto src1 = vld2q_f32(reinterpret_cast<const float*>(pSrc1 + i));
                auto src2 = vld2q_f32(reinterpret_cast<const float*>(pSrc2 + i));
                auto dst = NeonComplexMultiply(src1, src2);
                vst2q_f32(reinterpret_cast<float*>(pDst + i), dst);
            }

            if (i < length)
            {
                Arithmetic_Generic::Mul_32fc(pDst + i, pSrc1 + i, pSrc2 + i, length - i);
            }
        }

        _Use_decl_annotations_ void Mul_32f(float* pDst, float const* pSrc1, float const* pSrc2, size_t const length)
        {
            auto i = 0u;

            for (; i + 4 <= length; i += 4)
            {
                auto src1 = vld1q_f32(pSrc1 + i);
                auto src2 = vld1q_f32(pSrc2 + i);
                auto dst = vmulq_f32(src1, src2);
                vst1q_f32(pDst + i, dst);
            }

            if (i < length)
            {
                Arithmetic_Generic::Mul_32f(pDst + i, pSrc1 + i, pSrc2 + i, length - i);
            }
        }

        _Use_decl_annotations_ void MulC_32f(float* pDst, float const* pSrc, float const value, size_t const length)
        {
            auto i = 0u;
            if ((reinterpret_cast<ptrdiff_t>(pDst) & 0x0F) == 0 &&
                (reinterpret_cast<ptrdiff_t>(pSrc) & 0x0F) == 0) // If possible to align
            {
                auto j = 4u - reinterpret_cast<ptrdiff_t>(pDst) % (4 * sizeof(float)) / sizeof(float);
                while (j-- && i < length) // If beginning of vectors are unaligned
                {
                    pDst[i] = pSrc[i] * value;
                    ++i;
                }

                for (; i + 4 <= length; i += 4) // Aligned part
                {
                    auto src = vld1q_f32(pSrc + i);
                    auto dst = vmulq_n_f32(src, value);
                    vst1q_f32(pDst + i, dst);
                }
            }

            if (i < length) // Remainder
            {
                Arithmetic_Generic::MulC_32f(pDst + i, pSrc + i, value, length - i);
            }
        }

        _Use_decl_annotations_ void
        AddProduct_32f(float* pSrcDst, float const* pSrc1, float const* pSrc2, size_t const length)
        {
            auto i = 0u;
            for (; i + 4 <= length; i += 4)
            {
                auto src1 = vld1q_f32(pSrc1 + i);
                auto src2 = vld1q_f32(pSrc2 + i);
                auto srcDst = vld1q_f32(pSrcDst + i);
                srcDst = vmlaq_f32(srcDst, src1, src2);
                vst1q_f32(pSrcDst + i, srcDst);
            }

            if (i < length)
            {
                Arithmetic_Generic::AddProduct_32f(pSrcDst + i, pSrc1 + i, pSrc2 + i, length - i);
            }
        }

        _Use_decl_annotations_ void
        AddProduct_32fc(floatFC* pSrcDst, floatFC const* pSrc1, floatFC const* pSrc2, size_t const length)
        {
            auto i = 0u;
            for (; i + 4 <= length; i += 4)
            {
                auto src1 = vld2q_f32(reinterpret_cast<const float*>(pSrc1 + i));
                auto src2 = vld2q_f32(reinterpret_cast<const float*>(pSrc2 + i));
                auto srcDst = vld2q_f32(reinterpret_cast<const float*>(pSrcDst + i));
                srcDst = NeonComplexMultiplyAdd(src1, src2, srcDst);
                vst2q_f32(reinterpret_cast<float*>(pSrcDst + i), srcDst);
            }

            if (i < length)
            {
                Arithmetic_Generic::AddProduct_32fc(pSrcDst + i, pSrc1 + i, pSrc2 + i, length - i);
            }
        }

        _Use_decl_annotations_ void AddProductC_32f(float* pSrcDst, const float* pSrc, float scale, size_t const length)
        {
            auto i = 0u;
            for (; i + 4 <= length; i += 4)
            {
                auto srcDst = vld1q_f32(pSrcDst + i);
                auto src = vld1q_f32(pSrc + i);
                auto multiply = vmulq_n_f32(src, scale);
                srcDst = vaddq_f32(srcDst, multiply);
                vst1q_f32(pSrcDst + i, srcDst);
            }

            if (i < length)
            {
                Arithmetic_Generic::AddProductC_32f(pSrcDst + i, pSrc + i, scale, length - i);
            }
        }

        _Use_decl_annotations_ void
        DotProd_32f(float* pDst, float const* pSrc1, float const* pSrc2, _In_ size_t const length)
        {
            auto i = 0u;
            auto sum = vmovq_n_f32(0);

            // reset the sum
            *pDst = 0.0f;

            for (; i + 4 <= length; i += 4)
            {
                auto src1 = vld1q_f32(pSrc1 + i);
                auto src2 = vld1q_f32(pSrc2 + i);
                auto multiply = vmulq_f32(src1, src2);
                sum = vaddq_f32(sum, multiply);
            }

            // Do remaining elements
            for (; i < length; i++)
            {
                *pDst += pSrc1[i] * pSrc2[i];
            }

            // Horizontal sum
            auto resultPair = vadd_f32(vget_high_f32(sum), vget_low_f32(sum));
            *pDst += vget_lane_f32(resultPair, 0) + vget_lane_f32(resultPair, 1);
        }

        // dst = (src1 * val1) + (src2 * val2) + (src3 * val3)
        _Use_decl_annotations_ void DotProdC_32f(
            float* pDst, float const* pSrc1, float const* pSrc2, float const* pSrc3, float const val1, float const val2,
            float const val3, size_t length)
        {
            auto idxMax = length / 4;
            auto const val1Vec = vmovq_n_f32(val1);
            auto const val2Vec = vmovq_n_f32(val2);
            auto const val3Vec = vmovq_n_f32(val3);

            for (size_t i = 0; i < idxMax; i++)
            {
                auto src1 = vld1q_f32(pSrc1);
                auto src2 = vld1q_f32(pSrc2);
                auto src3 = vld1q_f32(pSrc3);
                pSrc1 += 4;
                pSrc2 += 4;
                pSrc3 += 4;

                auto res1 = vmulq_f32(src1, val1Vec);
                auto res2 = vmulq_f32(src2, val2Vec);
                auto res3 = vmulq_f32(src3, val3Vec);

                auto res4 = vaddq_f32(res3, vaddq_f32(res1, res2));
                vst1q_f32(pDst, res4);
                pDst += 4;
            }

            // Deal with remainder
            for (size_t i = idxMax * 4; i < length; i++)
            {
                pDst[i] = (pSrc1[i] * val1) + (pSrc2[i] * val2) + (pSrc3[i] * val3);
            }
        }

        // float result = a * (1 - remainder) + b * (remainder);
        // Or: a + (remainder * (b - a)); // Factor out remainder
        _Use_decl_annotations_ void Interpolate_32f(
            _Inout_updates_(length) float* pDst, _In_reads_(length) const float* pSrcA,
            _In_reads_(length) float const* pSrcB, _In_reads_(length) const float* pSrcR, size_t length)
        {
            auto idxMax = length / 4;

            for (size_t i = 0; i < idxMax; i++)
            {
                auto a = vld1q_f32(pSrcA);
                auto b = vld1q_f32(pSrcB);
                auto remainder = vld1q_f32(pSrcR);
                auto curDst = vld1q_f32(pDst);

                auto result = vaddq_f32(a, vmulq_f32(remainder, vsubq_f32(b, a)));
                vst1q_f32(pDst, result);

                pSrcA += 4;
                pSrcB += 4;
                pSrcR += 4;
                pDst += 4;
            }

            // Finish remainder
            for (size_t i = idxMax * 4; i < length; i++)
            {
                pDst[i] = pSrcA[i] + (pSrcR[i] * (pSrcB[i] - pSrcA[i]));
            }
        }

        _Use_decl_annotations_ uint32_t FindMaxIndex_32f(_In_ float* pVec, _In_ size_t const length)
        {
            if (length == 0)
            {
                return 0;
            }

            auto idxMax = length / 4;

            uint32_t values[4] = {0, 1, 2, 3};
            auto indicesVec = vld1q_u32(values);
            auto maxIndicesVec = indicesVec;
            auto maxValuesVec = vld1q_f32(pVec);
            auto incVec = vdupq_n_u32(4);

            for (size_t i = 4; i < length; i += 4)
            {
                // Increment the indices
                indicesVec = vaddq_u32(indicesVec, incVec);

                auto src = vld1q_f32(pVec + i);

                // Get bit result as to which vector had the higher value for each element
                auto IdxMask = vcgtq_f32(src, maxValuesVec);

                // Using the bitmask, save the index that belongs to the highest value
                maxIndicesVec = vbslq_u32(IdxMask, indicesVec, maxIndicesVec);

                // Find max values
                maxValuesVec = vmaxq_f32(maxValuesVec, src);
            }

            // Get max value/index horizontally
            auto maxValue = vgetq_lane_f32(maxValuesVec, 0);
            auto maxIndex = vgetq_lane_u32(maxIndicesVec, 0);
            if (maxValue < vgetq_lane_f32(maxValuesVec, 1))
            {
                maxValue = vgetq_lane_f32(maxValuesVec, 1);
                maxIndex = vgetq_lane_u32(maxIndicesVec, 1);
            }
            if (maxValue < vgetq_lane_f32(maxValuesVec, 2))
            {
                maxValue = vgetq_lane_f32(maxValuesVec, 2);
                maxIndex = vgetq_lane_u32(maxIndicesVec, 2);
            }
            if (maxValue < vgetq_lane_f32(maxValuesVec, 3))
            {
                maxValue = vgetq_lane_f32(maxValuesVec, 3);
                maxIndex = vgetq_lane_u32(maxIndicesVec, 3);
            }

            // Deal with residual
            for (size_t i = idxMax * 4; i < length; i++)
            {
                if (pVec[i] > maxValue)
                {
                    maxValue = pVec[i];
                    maxIndex = static_cast<uint32_t>(i);
                }
            }

            return maxIndex;
        }
    } // namespace Arithmetic_Neon
} // namespace VectorMath

#endif // defined(ARCH_ARM)
