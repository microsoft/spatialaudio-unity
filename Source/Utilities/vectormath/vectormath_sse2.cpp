// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
#include "cputype.h"
#if defined(ARCH_X86) || defined(ARCH_X64)
#include "vectormath.h"
#include "vectormath_sse2.h"
#include <cstring>
#include <cmath>
// SSE2 header
#include <xmmintrin.h>
// SSE4 header
#include <smmintrin.h>
#include <algorithm>

namespace VectorMath
{
    namespace Arithmetic_Sse2
    {
        inline void addps(__m128& dst, __m128& src)
        {
            dst = _mm_add_ps(dst, src);
        };
        inline void movaps(__m128& dst, void const* pSrc)
        {
            dst = _mm_load_ps((float*) pSrc);
        };
        inline void movaps(void* pDst, __m128& src)
        {
            _mm_store_ps((float*) pDst, src);
        };
        inline void movlps(__m128& dst, void const* pSrc)
        {
            dst = _mm_loadl_pi(dst, (__m64*) pSrc);
        };
        inline void movlps(void const* pDst, __m128& src)
        {
            _mm_storel_pi((__m64*) pDst, src);
        };
        inline void movups(__m128& dst, void const* pSrc)
        {
            dst = _mm_loadu_ps((float*) pSrc);
        };
        inline void movups(void* pDst, __m128& src)
        {
            _mm_storeu_ps((float*) pDst, src);
        };

        void Add_32f(float* pDst, const float* pSrc1, const float* pSrc2, size_t const length)
        {
            size_t i;

            // aligned branch. check if pointers can be aligned.
            if ((0 == (((ptrdiff_t) pDst) & 0x03)) && ((((ptrdiff_t) pDst) & 0x0f) == (((ptrdiff_t) pSrc1 & 0x0f))) &&
                ((((ptrdiff_t) pDst) & 0x0f) == (((ptrdiff_t) pSrc2 & 0x0f))))
            {
                // align the destination
                i = 0;
                while ((i < length) && ((ptrdiff_t) pDst & 0x0f))
                {
                    *pDst = *pSrc1 + *pSrc2;
                    pSrc1 += 1;
                    pSrc2 += 1;
                    pDst += 1;
                    i += 1;
                }

                // everything is aligned
                for (; i + 16 <= length; i += 16)
                {
                    __m128 xmm0, xmm1, xmm2, xmm3;

                    xmm0 = _mm_load_ps(pSrc1);
                    xmm1 = _mm_load_ps(pSrc1 + 4);
                    xmm2 = _mm_load_ps(pSrc1 + 8);
                    xmm3 = _mm_load_ps(pSrc1 + 12);
                    pSrc1 += 16;
                    xmm0 = _mm_add_ps(xmm0, *((__m128*) (pSrc2)));
                    xmm1 = _mm_add_ps(xmm1, *((__m128*) (pSrc2 + 4)));
                    xmm2 = _mm_add_ps(xmm2, *((__m128*) (pSrc2 + 8)));
                    xmm3 = _mm_add_ps(xmm3, *((__m128*) (pSrc2 + 12)));
                    pSrc2 += 16;
                    _mm_store_ps(pDst, xmm0);
                    _mm_store_ps(pDst + 4, xmm1);
                    _mm_store_ps(pDst + 8, xmm2);
                    _mm_store_ps(pDst + 12, xmm3);
                    pDst += 16;
                }

                for (; i + 4 <= length; i += 4)
                {
                    __m128 xmm0;

                    xmm0 = _mm_load_ps(pSrc1);
                    pSrc1 += 4;
                    xmm0 = _mm_add_ps(xmm0, *((__m128*) (pSrc2)));
                    pSrc2 += 4;
                    _mm_store_ps(pDst, xmm0);
                    pDst += 4;
                }
            }
            // unaligned branch
            else
            {
                for (i = 0; i + 4 <= length; i += 4)
                {
                    __m128 xmm0, xmm1;

                    xmm1 = _mm_loadu_ps(pSrc1);
                    pSrc1 += 4;
                    xmm0 = _mm_loadu_ps(pSrc2);
                    pSrc2 += 4;
                    xmm0 = _mm_add_ps(xmm0, xmm1);
                    _mm_storeu_ps(pDst, xmm0);
                    pDst += 4;
                }
            }

            // remain floats
            for (; i < length; i += 1)
            {
                *pDst = *pSrc1 + *pSrc2;
                pSrc1 += 1;
                pSrc2 += 1;
                pDst += 1;
            }
        }

        void Add_32f(float* pDst, const float* pSrc1, const float* pSrc2, const float* pSrc3, size_t const length)
        {
            size_t i;

            // aligned branch. check if pointers can be aligned.
            if ((0 == (((ptrdiff_t) pDst) & 0x03)) && ((((ptrdiff_t) pDst) & 0x0f) == (((ptrdiff_t) pSrc1 & 0x0f))) &&
                ((((ptrdiff_t) pDst) & 0x0f) == (((ptrdiff_t) pSrc2 & 0x0f))) &&
                ((((ptrdiff_t) pDst) & 0x0f) == (((ptrdiff_t) pSrc3 & 0x0f))))
            {
                // align the destination
                i = 0;
                while ((i < length) && ((ptrdiff_t) pDst & 0x0f))
                {
                    *pDst = *pSrc1 + *pSrc2 + *pSrc3;
                    pSrc1 += 1;
                    pSrc2 += 1;
                    pSrc3 += 1;
                    pDst += 1;
                    i += 1;
                }

                // everything is aligned
                for (; i + 16 <= length; i += 16)
                {
                    __m128 xmm0, xmm1, xmm2, xmm3;

                    xmm0 = _mm_load_ps(pSrc1);
                    xmm1 = _mm_load_ps(pSrc1 + 4);
                    xmm2 = _mm_load_ps(pSrc1 + 8);
                    xmm3 = _mm_load_ps(pSrc1 + 12);
                    pSrc1 += 16;
                    xmm0 = _mm_add_ps(xmm0, *((__m128*) (pSrc2)));
                    xmm1 = _mm_add_ps(xmm1, *((__m128*) (pSrc2 + 4)));
                    xmm2 = _mm_add_ps(xmm2, *((__m128*) (pSrc2 + 8)));
                    xmm3 = _mm_add_ps(xmm3, *((__m128*) (pSrc2 + 12)));
                    pSrc2 += 16;
                    xmm0 = _mm_add_ps(xmm0, *((__m128*) (pSrc3)));
                    xmm1 = _mm_add_ps(xmm1, *((__m128*) (pSrc3 + 4)));
                    xmm2 = _mm_add_ps(xmm2, *((__m128*) (pSrc3 + 8)));
                    xmm3 = _mm_add_ps(xmm3, *((__m128*) (pSrc3 + 12)));
                    pSrc3 += 16;
                    _mm_store_ps(pDst, xmm0);
                    _mm_store_ps(pDst + 4, xmm1);
                    _mm_store_ps(pDst + 8, xmm2);
                    _mm_store_ps(pDst + 12, xmm3);
                    pDst += 16;
                }

                for (; i + 4 <= length; i += 4)
                {
                    __m128 xmm0;

                    xmm0 = _mm_load_ps(pSrc1);
                    pSrc1 += 4;
                    xmm0 = _mm_add_ps(xmm0, *((__m128*) (pSrc2)));
                    pSrc2 += 4;
                    xmm0 = _mm_add_ps(xmm0, *((__m128*) (pSrc3)));
                    pSrc3 += 4;
                    _mm_store_ps(pDst, xmm0);
                    pDst += 4;
                }
            }
            // unaligned branch
            else
            {
                for (i = 0; i + 4 <= length; i += 4)
                {
                    __m128 xmm0, xmm1;

                    xmm1 = _mm_loadu_ps(pSrc1);
                    pSrc1 += 4;
                    xmm0 = _mm_loadu_ps(pSrc2);
                    pSrc2 += 4;
                    xmm0 = _mm_loadu_ps(pSrc3);
                    pSrc3 += 4;
                    xmm0 = _mm_add_ps(xmm0, xmm1);
                    _mm_storeu_ps(pDst, xmm0);
                    pDst += 4;
                }
            }

            // remain floats
            for (; i < length; i += 1)
            {
                *pDst = *pSrc1 + *pSrc2 + *pSrc3;
                pSrc1 += 1;
                pSrc2 += 1;
                pSrc3 += 1;
                pDst += 1;
            }
        }

        void Add_32f_I(float* pSrcDst, const float* pSrc, size_t length)
        {
            Add_32f(pSrcDst, pSrcDst, pSrc, length);
        }

        void Add_32fc(floatFC* pDst, const floatFC* pSrc1, const floatFC* pSrc2, const size_t length)
        {
            Add_32f((float*) pDst, (float const*) pSrc1, (float const*) pSrc2, length * 2);
        }

        void Sub_32f(float* pDst, const float* pSrc1, const float* pSrc2, size_t const length)
        {
            size_t i;

            // aligned branch. check if pointers can be aligned.
            if ((0 == (((ptrdiff_t) pDst) & 0x03)) && ((((ptrdiff_t) pDst) & 0x0f) == (((ptrdiff_t) pSrc1 & 0x0f))) &&
                ((((ptrdiff_t) pDst) & 0x0f) == (((ptrdiff_t) pSrc2 & 0x0f))))
            {
                // align the destination
                i = 0;
                while ((i < length) && ((ptrdiff_t) pDst & 0x0f))
                {
                    *pDst = *pSrc1 - *pSrc2;
                    pSrc1 += 1;
                    pSrc2 += 1;
                    pDst += 1;
                    i += 1;
                }

                // everything is aligned
                for (; i + 16 <= length; i += 16)
                {
                    __m128 xmm0, xmm1, xmm2, xmm3;

                    xmm0 = _mm_load_ps(pSrc1);
                    xmm1 = _mm_load_ps(pSrc1 + 4);
                    xmm2 = _mm_load_ps(pSrc1 + 8);
                    xmm3 = _mm_load_ps(pSrc1 + 12);
                    pSrc1 += 16;
                    xmm0 = _mm_sub_ps(xmm0, *((__m128*) (pSrc2)));
                    xmm1 = _mm_sub_ps(xmm1, *((__m128*) (pSrc2 + 4)));
                    xmm2 = _mm_sub_ps(xmm2, *((__m128*) (pSrc2 + 8)));
                    xmm3 = _mm_sub_ps(xmm3, *((__m128*) (pSrc2 + 12)));
                    pSrc2 += 16;
                    _mm_store_ps(pDst, xmm0);
                    _mm_store_ps(pDst + 4, xmm1);
                    _mm_store_ps(pDst + 8, xmm2);
                    _mm_store_ps(pDst + 12, xmm3);
                    pDst += 16;
                }

                for (; i + 4 <= length; i += 4)
                {
                    __m128 xmm0;

                    xmm0 = _mm_load_ps(pSrc1);
                    pSrc1 += 4;
                    xmm0 = _mm_sub_ps(xmm0, *((__m128*) (pSrc2)));
                    pSrc2 += 4;
                    _mm_store_ps(pDst, xmm0);
                    pDst += 4;
                }
            }
            // unaligned branch
            else
            {
                for (i = 0; i + 4 <= length; i += 4)
                {
                    __m128 xmm0, xmm1;

                    xmm1 = _mm_loadu_ps(pSrc1);
                    pSrc1 += 4;
                    xmm0 = _mm_loadu_ps(pSrc2);
                    pSrc2 += 4;
                    xmm0 = _mm_sub_ps(xmm0, xmm1);
                    _mm_storeu_ps(pDst, xmm0);
                    pDst += 4;
                }
            }

            // remain floats
            for (; i < length; i += 1)
            {
                *pDst = *pSrc1 - *pSrc2;
                pSrc1 += 1;
                pSrc2 += 1;
                pDst += 1;
            }
        }

        void Sub_32fc(floatFC* pDst, const floatFC* pSrc1, const floatFC* pSrc2, const size_t length)
        {
            Sub_32f((float*) pDst, (float const*) pSrc1, (float const*) pSrc2, length * 2);
        }

        /* The implementations of the below functions were taken from
         * %sdxroot%\avcore\mf\dll\wmperfdll\*.cpp.
         * They are copied here rather than called because that library is not built for ARM. */

#define MS_MUL_1_CMPL_SSE()                                                                                            \
                                                                                                                       \
    {                                                                                                                  \
        __m128 xmm0, xmm1, xmm2, xmm3, xmm4, xmm5;                                                                     \
        xmm4 = _mm_load_ss(((float*) pSrc1) + 0); /* load real part */                                                 \
        xmm5 = _mm_load_ss(((float*) pSrc1) + 1); /* load imaginary part */                                            \
        xmm0 = _mm_load_ss(((float*) pSrc2) + 0); /* load source real part */                                          \
        xmm1 = _mm_load_ss(((float*) pSrc2) + 1); /* load source imaginary part */                                     \
        xmm2 = _mm_mul_ss(xmm4, xmm0);            /* pSrcDst[i].re * pSrc[i].re */                                     \
        xmm3 = _mm_mul_ss(xmm5, xmm1);            /* pSrcDst[i].im * pSrc[i].im */                                     \
        xmm4 = _mm_mul_ss(xmm4, xmm1);            /* pSrcDst[i].re * pSrc[i].im */                                     \
        xmm5 = _mm_mul_ss(xmm5, xmm0);            /* pSrcDst[i].im * pSrc[i].re */                                     \
        xmm2 = _mm_sub_ss(xmm2, xmm3);            /* create new real parts */                                          \
        xmm4 = _mm_add_ss(xmm4, xmm5);            /* create new imaginary parts */                                     \
        xmm2 = _mm_unpacklo_ps(xmm2, xmm4);       /* combine new complex values */                                     \
        _mm_storel_pi((__m64*) pDst, xmm2);       /* store new values */                                               \
    }

        inline __m128 _mm_addsub_ps_sse2(const __m128 a, const __m128 b)
        {
            __m128 xmm0, xmm1, xmm2;
            xmm0 = _mm_add_ps(a, b); /* sum new values */
            xmm1 = _mm_sub_ps(a, b); /* sum new values */
            xmm2 = _mm_shuffle_ps(xmm0, xmm0, _MM_SHUFFLE(3, 1, 3, 1));
            xmm0 = _mm_shuffle_ps(xmm1, xmm1, _MM_SHUFFLE(2, 0, 2, 0));
            xmm1 = _mm_unpacklo_ps(xmm0, xmm2);
            return xmm1;
        }

#define MS_MUL_4_CMPL_SSE2(store_intrinsic)                                                                            \
                                                                                                                       \
    {                                                                                                                  \
        __m128 xmm0, xmm1, xmm2, xmm3, xmm4, xmm5;                                                                     \
        xmm0 = _mm_loadu_ps(((float*) pSrc1) + 0);  /* load 2 complex floats */                                        \
        xmm1 = _mm_loadu_ps(((float*) pSrc1) + 4);  /* load 2 complex floats */                                        \
        xmm4 = _mm_shuffle_ps(xmm0, xmm0, 0x0a0);   /* get 4 real parts */                                             \
        xmm2 = _mm_loadu_ps(((float*) pSrc2) + 0);  /* load 2 source complex floats */                                 \
        xmm5 = _mm_shuffle_ps(xmm0, xmm0, 0x0f5);   /* get 4 imaginary parts */                                        \
        xmm3 = _mm_loadu_ps(((float*) pSrc2) + 4);  /* load 2 source complex floats */                                 \
        xmm0 = _mm_shuffle_ps(xmm1, xmm1, 0x0a0);   /* get 4 source real parts */                                      \
        xmm4 = _mm_mul_ps(xmm4, xmm2);              /* multiply parts */                                               \
        xmm1 = _mm_shuffle_ps(xmm1, xmm1, 0x0f5);   /* get 4 source imaginary parts */                                 \
        xmm5 = _mm_mul_ps(xmm5, xmm2);              /* multiply parts */                                               \
        xmm0 = _mm_mul_ps(xmm0, xmm3);              /* multiply parts */                                               \
        xmm5 = _mm_shuffle_ps(xmm5, xmm5, 0x0b1);   /* change the order of img multiply */                             \
        xmm1 = _mm_mul_ps(xmm1, xmm3);              /* multiply parts */                                               \
        xmm1 = _mm_shuffle_ps(xmm1, xmm1, 0x0b1);   /* change the order of img multiply */                             \
        xmm4 = _mm_addsub_ps_sse2(xmm4, xmm5);      /* sum new values */                                               \
        store_intrinsic(((float*) pDst), xmm4);     /* store new values */                                             \
        xmm0 = _mm_addsub_ps_sse2(xmm0, xmm1);      /* sum new values */                                               \
        store_intrinsic(((float*) pDst) + 4, xmm0); /* store new values */                                             \
    }

        _Use_decl_annotations_ void
        Mul_32fc(floatFC* pDst, floatFC const* pSrc1, floatFC const* pSrc2, size_t const length)
        {
            size_t i;

            // aligned branch
            if (0 == (((ptrdiff_t) pDst) & 0x07))
            {
                // align the destination pointer
                i = 0;
                while ((i < length) && (((ptrdiff_t) pDst) & 0x0f))
                {
                    MS_MUL_1_CMPL_SSE();
                    i += 1;

                    // advance pointers
                    pSrc1 += 1;
                    pSrc2 += 1;
                    pDst += 1;
                }

                for (; i + 4 < length; i += 4)
                {
                    MS_MUL_4_CMPL_SSE2(_mm_store_ps);

                    // advance pointers
                    pSrc1 += 4;
                    pSrc2 += 4;
                    pDst += 4;
                }
            }
            // unaligned branch
            else
            {
                for (i = 0; i + 4 < length; i += 4)
                {
                    MS_MUL_4_CMPL_SSE2(_mm_storeu_ps);

                    // advance pointers
                    pSrc1 += 4;
                    pSrc2 += 4;
                    pDst += 4;
                }
            }

            // process remain values
            for (; i < length; i += 1)
            {
                MS_MUL_1_CMPL_SSE();

                // advance pointers
                pSrc1 += 1;
                pSrc2 += 1;
                pDst += 1;
            }
        }

#define MS_MUL_1_32F_SSE2()                                                                                            \
                                                                                                                       \
    {                                                                                                                  \
        __m128 xmm0, xmm1;                                                                                             \
        xmm0 = _mm_load_ss(pSrc1);     /* load float */                                                                \
        xmm1 = _mm_load_ss(pSrc2);     /* load source float */                                                         \
        xmm0 = _mm_mul_ps(xmm0, xmm1); /* multiply float */                                                            \
        _mm_store_ss(pDst, xmm0);                                                                                      \
    }

#define MS_MUL_4_32F_SSE2(store_intrinsic, load_intrinsic)                                                             \
                                                                                                                       \
    {                                                                                                                  \
        __m128 xmm0, xmm1;                                                                                             \
        xmm0 = load_intrinsic(pSrc1);  /* load floats 0-3 */                                                           \
        xmm1 = load_intrinsic(pSrc2);  /* load source floats 0-3 */                                                    \
        xmm0 = _mm_mul_ps(xmm0, xmm1); /* multiply floats 0-3 */                                                       \
        store_intrinsic(pDst, xmm0);                                                                                   \
    }

#define MS_MUL_16_32F_SSE2(store_intrinsic, load_intrinsic)                                                            \
                                                                                                                       \
    {                                                                                                                  \
        __m128 xmm0, xmm1, xmm2, xmm3, xmm4, xmm5;                                                                     \
        xmm0 = load_intrinsic(pSrc1);      /* load floats 0-3 */                                                       \
        xmm4 = load_intrinsic(pSrc2);      /* load source floats 0-3 */                                                \
        xmm1 = load_intrinsic(pSrc1 + 4);  /* load floats 4-7 */                                                       \
        xmm5 = load_intrinsic(pSrc2 + 4);  /* load source floats 4-7 */                                                \
        xmm0 = _mm_mul_ps(xmm0, xmm4);     /* multiply floats 0-3 */                                                   \
        xmm2 = load_intrinsic(pSrc1 + 8);  /* load floats 8-11 */                                                      \
        xmm4 = load_intrinsic(pSrc2 + 8);  /* load source floats 8-11 */                                               \
        xmm1 = _mm_mul_ps(xmm1, xmm5);     /* multiply floats 4-7 */                                                   \
        xmm3 = load_intrinsic(pSrc1 + 12); /* load floats 12-15 */                                                     \
        xmm5 = load_intrinsic(pSrc2 + 12); /* load source floats 12-15 */                                              \
        xmm2 = _mm_mul_ps(xmm2, xmm4);     /* multiply floats 8-11 */                                                  \
        store_intrinsic(pDst, xmm0);                                                                                   \
        xmm3 = _mm_mul_ps(xmm3, xmm5); /* multiply floats 12-15 */                                                     \
        store_intrinsic(pDst + 4, xmm1);                                                                               \
        store_intrinsic(pDst + 8, xmm2);                                                                               \
        store_intrinsic(pDst + 12, xmm3);                                                                              \
    }

        _Use_decl_annotations_ void Mul_32f(float* pDst, float const* pSrc1, float const* pSrc2, size_t const length)
        {

            size_t i;

            // aligned branch
            if (0 == (((ptrdiff_t) pDst) & 0x03))
            {
                // align the destination pointer
                i = 0;
                while ((i < length) && (((ptrdiff_t) pDst) & 0x0f))
                {
                    MS_MUL_1_32F_SSE2();
                    i += 1;

                    // advance pointers
                    pSrc1 += 1;
                    pSrc2 += 1;
                    pDst += 1;
                }

                // the source pointer is aligned as well
                if (0 == (((ptrdiff_t) pSrc1 | (ptrdiff_t) pSrc2) & 0x0f))
                {
                    for (; i + 16 < length; i += 16)
                    {
                        MS_MUL_16_32F_SSE2(_mm_store_ps, _mm_load_ps)

                        // advance pointers
                        pSrc1 += 16;
                        pSrc2 += 16;
                        pDst += 16;
                    }
                    for (; i + 4 < length; i += 4)
                    {
                        MS_MUL_4_32F_SSE2(_mm_store_ps, _mm_load_ps)

                        // advance pointers
                        pSrc1 += 4;
                        pSrc2 += 4;
                        pDst += 4;
                    }
                }
                // the source pointer is not aligned
                else
                {
                    for (; i + 16 < length; i += 16)
                    {
                        MS_MUL_16_32F_SSE2(_mm_store_ps, _mm_loadu_ps)

                        // advance pointers
                        pSrc1 += 16;
                        pSrc2 += 16;
                        pDst += 16;
                    }
                    for (; i + 4 < length; i += 4)
                    {
                        MS_MUL_4_32F_SSE2(_mm_store_ps, _mm_loadu_ps)

                        // advance pointers
                        pSrc1 += 4;
                        pSrc2 += 4;
                        pDst += 4;
                    }
                }
            }
            // unaligned branch
            else
            {
                for (i = 0; i + 16 < length; i += 16)
                {
                    MS_MUL_16_32F_SSE2(_mm_storeu_ps, _mm_loadu_ps)

                    // advance pointers
                    pSrc1 += 16;
                    pSrc2 += 16;
                    pDst += 16;
                }
                for (; i + 4 < length; i += 4)
                {
                    MS_MUL_4_32F_SSE2(_mm_storeu_ps, _mm_loadu_ps)

                    // advance pointers
                    pSrc1 += 4;
                    pSrc2 += 4;
                    pDst += 4;
                }
            }

            // process remain values
            for (; i < length; i += 1)
            {
                MS_MUL_1_32F_SSE2();

                // advance pointers
                pSrc1 += 1;
                pSrc2 += 1;
                pDst += 1;
            }
        }

#define MS_MUL_C_1_32F_SSE2()                                                                                          \
                                                                                                                       \
    {                                                                                                                  \
        __m128 xmm0;                                                                                                   \
        xmm0 = _mm_load_ss(pSrc);          /* load float */                                                            \
        xmm0 = _mm_mul_ps(xmm0, mulConst); /* multiply float */                                                        \
        _mm_store_ss(pDst, xmm0);                                                                                      \
    }

#define MS_MUL_C_4_32F_SSE2(store_intrinsic, load_intrinsic)                                                           \
                                                                                                                       \
    {                                                                                                                  \
        __m128 xmm0;                                                                                                   \
        xmm0 = load_intrinsic(pSrc);       /* load floats 0-3 */                                                       \
        xmm0 = _mm_mul_ps(xmm0, mulConst); /* multiply floats 0-3 */                                                   \
        store_intrinsic(pDst, xmm0);                                                                                   \
    }

#define MS_MUL_C_16_32F_SSE2(store_intrinsic, load_intrinsic)                                                          \
                                                                                                                       \
    {                                                                                                                  \
        __m128 xmm0, xmm1, xmm2, xmm3;                                                                                 \
        xmm0 = load_intrinsic(pSrc);       /* load floats 0-3 */                                                       \
        xmm1 = load_intrinsic(pSrc + 4);   /* load floats 4-7 */                                                       \
        xmm2 = load_intrinsic(pSrc + 8);   /* load floats 8-11 */                                                      \
        xmm0 = _mm_mul_ps(xmm0, mulConst); /* multiply floats 0-3 */                                                   \
        xmm3 = load_intrinsic(pSrc + 12);  /* load floats 12-15 */                                                     \
        xmm1 = _mm_mul_ps(xmm1, mulConst); /* multiply floats 4-7 */                                                   \
        store_intrinsic(pDst, xmm0);                                                                                   \
        xmm2 = _mm_mul_ps(xmm2, mulConst); /* multiply floats 8-11 */                                                  \
        store_intrinsic(pDst + 4, xmm1);                                                                               \
        xmm3 = _mm_mul_ps(xmm3, mulConst); /* multiply floats 12-15 */                                                 \
        store_intrinsic(pDst + 8, xmm2);                                                                               \
        store_intrinsic(pDst + 12, xmm3);                                                                              \
    }
        _Use_decl_annotations_ void MulC_32f(float* pDst, float const* pSrc, float const value, size_t const length)
        {
            __m128 const mulConst = _mm_load_ps1(&value);
            size_t i;

            // aligned branch
            if (0 == (((ptrdiff_t) pDst) & 0x03))
            {
                // align the destination pointer
                i = 0;
                while ((i < length) && (((ptrdiff_t) pDst) & 0x0f))
                {
                    MS_MUL_C_1_32F_SSE2();
                    i += 1;

                    // advance pointers
                    pSrc += 1;
                    pDst += 1;
                }

                // the source pointer is aligned as well
                if (0 == (((ptrdiff_t) pSrc) & 0x0f))
                {
                    for (; i + 16 < length; i += 16)
                    {
                        MS_MUL_C_16_32F_SSE2(_mm_store_ps, _mm_load_ps)

                        // advance pointers
                        pSrc += 16;
                        pDst += 16;
                    }
                    for (; i + 4 < length; i += 4)
                    {
                        MS_MUL_C_4_32F_SSE2(_mm_store_ps, _mm_load_ps)

                        // advance pointers
                        pSrc += 4;
                        pDst += 4;
                    }
                }
                // the source pointer is not aligned
                else
                {
                    for (; i + 16 < length; i += 16)
                    {
                        MS_MUL_C_16_32F_SSE2(_mm_store_ps, _mm_loadu_ps)

                        // advance pointers
                        pSrc += 16;
                        pDst += 16;
                    }
                    for (; i + 4 < length; i += 4)
                    {
                        MS_MUL_C_4_32F_SSE2(_mm_store_ps, _mm_loadu_ps)

                        // advance pointers
                        pSrc += 4;
                        pDst += 4;
                    }
                }
            }
            // unaligned branch
            else
            {
                for (i = 0; i + 16 < length; i += 16)
                {
                    MS_MUL_C_16_32F_SSE2(_mm_storeu_ps, _mm_loadu_ps)

                    // advance pointers
                    pSrc += 16;
                    pDst += 16;
                }
                for (; i + 4 < length; i += 4)
                {
                    MS_MUL_C_4_32F_SSE2(_mm_storeu_ps, _mm_loadu_ps)

                    // advance pointers
                    pSrc += 4;
                    pDst += 4;
                }
            }

            // process remain values
            for (; i < length; i += 1)
            {
                MS_MUL_C_1_32F_SSE2();

                // advance pointers
                pSrc += 1;
                pDst += 1;
            }
        }

#define MS_MUL_ADD_1_FLT_SSE()                                                                                         \
                                                                                                                       \
    {                                                                                                                  \
        __m128 xmm0, xmm1;                                                                                             \
        xmm0 = _mm_load_ss(pSrc1);     /* load source 0 */                                                             \
        xmm1 = _mm_load_ss(pSrc2);     /* load source 1 */                                                             \
        xmm0 = _mm_mul_ss(xmm0, xmm1); /* pSrc1 * pSrc2 */                                                             \
        xmm1 = _mm_load_ss(pSrcDst);   /* load destination */                                                          \
        xmm0 = _mm_add_ss(xmm0, xmm1); /* update destination */                                                        \
        _mm_store_ss(pSrcDst, xmm0);   /* store new value */                                                           \
    }

#define MS_MUL_ADD_4_FLT_SSE(access_instr)                                                                             \
                                                                                                                       \
    {                                                                                                                  \
        __m128 xmm0, xmm1;                                                                                             \
        movups(xmm0, pSrc1);           /* load source 0 */                                                             \
        movups(xmm1, pSrc2);           /* load source 1 */                                                             \
        xmm0 = _mm_mul_ps(xmm0, xmm1); /* pSrc1 * pSrc2 */                                                             \
        access_instr(xmm1, pSrcDst);   /* load destination */                                                          \
        xmm0 = _mm_add_ps(xmm0, xmm1); /* update destination */                                                        \
        access_instr(pSrcDst, xmm0);   /* store new value */                                                           \
    }

        _Use_decl_annotations_ void
        AddProduct_32f(float* pSrcDst, float const* pSrc1, float const* pSrc2, size_t const length)
        {
            size_t i;

            // aligned branch
            if (0 == (((ptrdiff_t) pSrcDst) & 0x03))
            {
                // align the destination pointer
                i = 0;
                while ((i < length) && (((ptrdiff_t) pSrcDst) & 0x0f))
                {
                    MS_MUL_ADD_1_FLT_SSE();
                    i += 1;

                    // advance pointers
                    pSrc1 += 1;
                    pSrc2 += 1;
                    pSrcDst += 1;
                }

                for (; i + 4 < length; i += 4)
                {
                    MS_MUL_ADD_4_FLT_SSE(movaps);

                    // advance pointers
                    pSrc1 += 4;
                    pSrc2 += 4;
                    pSrcDst += 4;
                }
            }
            // unaligned branch
            else
            {
                for (i = 0; i + 4 < length; i += 4)
                {
                    MS_MUL_ADD_4_FLT_SSE(movups);

                    // advance pointers
                    pSrc1 += 4;
                    pSrc2 += 4;
                    pSrcDst += 4;
                }
            }

            // process remain values
            for (; i < length; i += 1)
            {
                MS_MUL_ADD_1_FLT_SSE();

                // advance pointers
                pSrc1 += 1;
                pSrc2 += 1;
                pSrcDst += 1;
            }
        }

#define MS_MUL_ADD_1_CMPL_SSE()                                                                                        \
                                                                                                                       \
    {                                                                                                                  \
        __m128 xmm0, xmm1, xmm2, xmm3, xmm4, xmm5;                                                                     \
        xmm4 = _mm_load_ss(((float*) pSrc1) + 0); /* load real part */                                                 \
        xmm5 = _mm_load_ss(((float*) pSrc1) + 1); /* load imaginary part */                                            \
        xmm0 = _mm_load_ss(((float*) pSrc2) + 0); /* load source real part */                                          \
        xmm1 = _mm_load_ss(((float*) pSrc2) + 1); /* load source imaginary part */                                     \
        xmm2 = _mm_mul_ss(xmm4, xmm0);            /* pSrcDst[i].re * pSrc[i].re */                                     \
        xmm3 = _mm_mul_ss(xmm5, xmm1);            /* pSrcDst[i].im * pSrc[i].im */                                     \
        xmm4 = _mm_mul_ss(xmm4, xmm1);            /* pSrcDst[i].re * pSrc[i].im */                                     \
        movlps(xmm1, pSrcDst);                    /* load data from the destination array */                           \
        xmm5 = _mm_mul_ss(xmm5, xmm0);            /* pSrcDst[i].im * pSrc[i].re */                                     \
        xmm2 = _mm_sub_ss(xmm2, xmm3);            /* create new real parts */                                          \
        xmm4 = _mm_add_ss(xmm4, xmm5);            /* create new imaginary parts */                                     \
        xmm2 = _mm_unpacklo_ps(xmm2, xmm4);       /* combine new complex values */                                     \
        addps(xmm2, xmm1);                        /* add new value to destination data */                              \
        movlps(pSrcDst, xmm2);                    /* store new values */                                               \
    }

#define MS_MUL_ADD_4_CMPL_SSE(access_instr)                                                                            \
                                                                                                                       \
    {                                                                                                                  \
        __m128 xmm0, xmm1, xmm2, xmm3, xmm4, xmm5;                                                                     \
        xmm0 = _mm_loadu_ps(((float*) pSrc1) + 0); /* load 2 complex floats */                                         \
        xmm1 = _mm_loadu_ps(((float*) pSrc1) + 4); /* load 2 complex floats */                                         \
        xmm4 = _mm_shuffle_ps(xmm0, xmm0, 0x0a0);  /* get 4 real parts */                                              \
        xmm2 = _mm_loadu_ps(((float*) pSrc2) + 0); /* load 2 source complex floats */                                  \
        xmm5 = _mm_shuffle_ps(xmm0, xmm0, 0x0f5);  /* get 4 imaginary parts */                                         \
        xmm3 = _mm_loadu_ps(((float*) pSrc2) + 4); /* load 2 source complex floats */                                  \
        xmm0 = _mm_shuffle_ps(xmm1, xmm1, 0x0a0);  /* get 4 source real parts */                                       \
        xmm4 = _mm_mul_ps(xmm4, xmm2);             /* multiply parts */                                                \
        xmm1 = _mm_shuffle_ps(xmm1, xmm1, 0x0f5);  /* get 4 source imaginary parts */                                  \
        xmm5 = _mm_mul_ps(xmm5, xmm2);             /* multiply parts */                                                \
        access_instr(xmm2, pSrcDst);               /* load data from the destination array */                          \
        xmm0 = _mm_mul_ps(xmm0, xmm3);             /* multiply parts */                                                \
        xmm5 = _mm_shuffle_ps(xmm5, xmm5, 0x0b1);  /* change the order of img multiply */                              \
        xmm1 = _mm_mul_ps(xmm1, xmm3);             /* multiply parts */                                                \
        access_instr(xmm3, pSrcDst + 2);           /* load data from the destination array */                          \
        xmm1 = _mm_shuffle_ps(xmm1, xmm1, 0x0b1);  /* change the order of img multiply */                              \
        xmm4 = _mm_addsub_ps_sse2(xmm4, xmm5);     /* sum new values */                                                \
        xmm0 = _mm_addsub_ps_sse2(xmm0, xmm1);     /* sum new values */                                                \
        addps(xmm4, xmm2);                         /* add new value to destination data */                             \
        access_instr(pSrcDst, xmm4);               /* store new values */                                              \
        addps(xmm0, xmm3);                         /* add new value to destination data */                             \
        access_instr(pSrcDst + 2, xmm0);           /* store new values */                                              \
    }

        _Use_decl_annotations_ void
        AddProduct_32fc(floatFC* pSrcDst, floatFC const* pSrc1, floatFC const* pSrc2, size_t const length)
        {
            size_t i;

            // aligned branch
            if (0 == (((ptrdiff_t) pSrcDst) & 0x07))
            {
                // align the destination pointer
                i = 0;
                while ((i < length) && (((ptrdiff_t) pSrcDst) & 0x0f))
                {
                    MS_MUL_ADD_1_CMPL_SSE();
                    i += 1;

                    // advance pointers
                    pSrc1 += 1;
                    pSrc2 += 1;
                    pSrcDst += 1;
                }

                for (; i + 4 < length; i += 4)
                {
                    MS_MUL_ADD_4_CMPL_SSE(movaps);

                    // advance pointers
                    pSrc1 += 4;
                    pSrc2 += 4;
                    pSrcDst += 4;
                }
            }
            // unaligned branch
            else
            {
                for (i = 0; i + 4 < length; i += 4)
                {
                    MS_MUL_ADD_4_CMPL_SSE(movups);

                    // advance pointers
                    pSrc1 += 4;
                    pSrc2 += 4;
                    pSrcDst += 4;
                }
            }

            // process remain values
            for (; i < length; i += 1)
            {
                MS_MUL_ADD_1_CMPL_SSE();

                // advance pointers
                pSrc1 += 1;
                pSrc2 += 1;
                pSrcDst += 1;
            }
        }

#define MS_MUL_ADD_C_1_FLT_SSE()                                                                                       \
                                                                                                                       \
    {                                                                                                                  \
        __m128 xmm0, xmm1;                                                                                             \
        xmm0 = _mm_load_ss(pSrc);          /* load source */                                                           \
        xmm0 = _mm_mul_ss(xmm0, xmmMulti); /* pSrc * val */                                                            \
        xmm1 = _mm_load_ss(pSrcDst);       /* load destination */                                                      \
        xmm0 = _mm_add_ss(xmm0, xmm1);     /* update destination */                                                    \
        _mm_store_ss(pSrcDst, xmm0);       /* store new value */                                                       \
    }

#define MS_MUL_ADD_C_4_FLT_SSE(access_instr)                                                                           \
                                                                                                                       \
    {                                                                                                                  \
        __m128 xmm0, xmm1;                                                                                             \
        movups(xmm0, pSrc);                /* load source */                                                           \
        xmm0 = _mm_mul_ps(xmm0, xmmMulti); /* pSrc * val */                                                            \
        access_instr(xmm1, pSrcDst);       /* load destination */                                                      \
        xmm0 = _mm_add_ps(xmm0, xmm1);     /* update destination */                                                    \
        access_instr(pSrcDst, xmm0);       /* store new value */                                                       \
    }

        _Use_decl_annotations_ void AddProductC_32f(float* pSrcDst, const float* pSrc, float scale, size_t length)
        {
            __m128 const xmmMulti = _mm_load1_ps(&scale);
            size_t i;

            // aligned branch
            if (0 == (((ptrdiff_t) pSrcDst) & 0x03))
            {
                // align the destination pointer
                i = 0;
                while ((i < length) && (((ptrdiff_t) pSrcDst) & 0x0f))
                {
                    MS_MUL_ADD_C_1_FLT_SSE();
                    i += 1;

                    // advance pointers
                    pSrc += 1;
                    pSrcDst += 1;
                }

                for (; i + 4 < length; i += 4)
                {
                    MS_MUL_ADD_C_4_FLT_SSE(movaps);

                    // advance pointers
                    pSrc += 4;
                    pSrcDst += 4;
                }
            }
            // unaligned branch
            else
            {
                for (i = 0; i + 4 < length; i += 4)
                {
                    MS_MUL_ADD_C_4_FLT_SSE(movups);

                    // advance pointers
                    pSrc += 4;
                    pSrcDst += 4;
                }
            }

            // process remain values
            for (; i < length; i += 1)
            {
                MS_MUL_ADD_C_1_FLT_SSE();

                // advance pointers
                pSrc += 1;
                pSrcDst += 1;
            }
        }

#define DOT_SINGLE_FLOAT(sum)                                                                                          \
                                                                                                                       \
    {                                                                                                                  \
        __m128 xmm0;                                                                                                   \
        xmm0 = _mm_load_ss(pSrc1);                                                                                     \
        xmm0 = _mm_mul_ss(xmm0, *((__m128*) (pSrc2)));                                                                 \
        sum = _mm_add_ss(sum, xmm0);                                                                                   \
    }

#define DOT_4_FLOATS(load_intrinsic, sum)                                                                              \
                                                                                                                       \
    {                                                                                                                  \
        __m128 xmm0, xmm1;                                                                                             \
        xmm1 = _mm_loadu_ps(pSrc1);                                                                                    \
        xmm0 = _mm_loadu_ps(pSrc2);                                                                                    \
        xmm0 = _mm_mul_ps(xmm0, xmm1);                                                                                 \
        sum = _mm_add_ps(sum, xmm0);                                                                                   \
    }

        static __m128 GetSum(__m128 sum)
        {
            __m128 tmp;

            tmp = sum;
            tmp = _mm_shuffle_ps(tmp, tmp, 0x4e);
            sum = _mm_add_ps(sum, tmp);
            tmp = sum;
            tmp = _mm_shuffle_ps(tmp, tmp, 0xb1);
            sum = _mm_add_ps(sum, tmp);

            return sum;

        } // __m128 GetSum(__m128 sum)

        _Use_decl_annotations_ void
        DotProd_32f(float* pDst, float const* pSrc1, float const* pSrc2, size_t const length)
        {
            __m128 sum;
            size_t i;

            // reset the sum
            *pDst = 0.0f;
            sum = _mm_setzero_ps();

            // aligned branch. check if pointers can be aligned.
            if ((0 == (((ptrdiff_t) pSrc1) & 0x03)) && ((((ptrdiff_t) pSrc1) & 0x0f) == (((ptrdiff_t) pSrc2 & 0x0f))))
            {
                // align the destination
                i = 0;
                while ((i < length) && (((ptrdiff_t)(pSrc1)) & 0x0f))
                {
                    DOT_SINGLE_FLOAT(sum);
                    i += 1;

                    // advance pointers
                    pSrc1 += 1;
                    pSrc2 += 1;
                }

                // everything is aligned
                for (; i + 16 <= length; i += 16)
                {
                    __m128 xmm0, xmm1, xmm2, xmm3;

                    xmm0 = _mm_load_ps(pSrc1);
                    xmm1 = _mm_load_ps(pSrc1 + 4);
                    xmm2 = _mm_load_ps(pSrc1 + 8);
                    xmm3 = _mm_load_ps(pSrc1 + 12);
                    xmm0 = _mm_mul_ps(xmm0, *((__m128*) (pSrc2)));
                    xmm1 = _mm_mul_ps(xmm1, *((__m128*) (pSrc2 + 4)));
                    xmm2 = _mm_mul_ps(xmm2, *((__m128*) (pSrc2 + 8)));
                    xmm3 = _mm_mul_ps(xmm3, *((__m128*) (pSrc2 + 12)));
                    xmm0 = _mm_add_ps(xmm0, xmm1);
                    xmm2 = _mm_add_ps(xmm2, xmm3);
                    xmm0 = _mm_add_ps(xmm0, xmm2);
                    sum = _mm_add_ps(sum, xmm0);

                    // advance pointers
                    pSrc1 += 16;
                    pSrc2 += 16;
                }

                for (; i + 4 <= length; i += 4)
                {
                    DOT_4_FLOATS(_mm_load_ps, sum);

                    // advance pointers
                    pSrc1 += 4;
                    pSrc2 += 4;
                }
            }
            // unaligned branch
            else
            {
                for (i = 0; i + 4 <= length; i += 4)
                {
                    DOT_4_FLOATS(_mm_loadu_ps, sum);

                    // advance pointers
                    pSrc1 += 4;
                    pSrc2 += 4;
                }
            }

            // remain floats
            for (; i < length; i += 1)
            {
                DOT_SINGLE_FLOAT(sum);

                // advance pointers
                pSrc1 += 1;
                pSrc2 += 1;
            }

            // store the result
            _mm_store_ss(pDst, GetSum(sum));
        }

        // dst = (src1 * val1) + (src2 * val2) + (src3 * val3)
        _Use_decl_annotations_ void DotProdC_32f(
            float* pDst, float const* pSrc1, float const* pSrc2, float const* pSrc3, float const val1, float const val2,
            float const val3, size_t length)
        {
            auto idxMax = length / 4;
            auto const val1Vec = _mm_load_ps1(&val1);
            auto const val2Vec = _mm_load_ps1(&val2);
            auto const val3Vec = _mm_load_ps1(&val3);

            for (size_t i = 0; i < idxMax; i++)
            {
                auto src1 = _mm_loadu_ps(pSrc1);
                auto src2 = _mm_loadu_ps(pSrc2);
                auto src3 = _mm_loadu_ps(pSrc3);
                pSrc1 += 4;
                pSrc2 += 4;
                pSrc3 += 4;

                auto res1 = _mm_mul_ps(src1, val1Vec);
                auto res2 = _mm_mul_ps(src2, val2Vec);
                auto res3 = _mm_mul_ps(src3, val3Vec);

                auto res4 = _mm_add_ps(res3, _mm_add_ps(res1, res2));
                _mm_store_ps(pDst, res4);
                pDst += 4;
            }

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
                auto a = _mm_loadu_ps(pSrcA + i * 4);
                auto b = _mm_loadu_ps(pSrcB + i * 4);
                auto remainder = _mm_loadu_ps(pSrcR + i * 4);
                auto curDst = _mm_loadu_ps(pDst + i * 4);

                auto result = _mm_add_ps(a, _mm_mul_ps(remainder, _mm_sub_ps(b, a)));
                _mm_store_ps(pDst + i * 4, result);
            }

            // Finish remainder
            for (size_t i = idxMax * 4; i < length; i++)
            {
                pDst[i] = pSrcA[i] + (pSrcR[i] * (pSrcB[i] - pSrcA[i]));
            }
        }

        /* Find index of max element in vector */
        _Use_decl_annotations_ uint32_t FindMaxIndex_32f(_In_ float* pSrc, _In_ size_t const length)
        {
            if (length == 0)
            {
                return 0;
            }
            float maxValues[4];
            float maxIndices[4];

            auto incVec = _mm_set1_ps(4);
            auto indicesVec = _mm_setr_ps(0, 1, 2, 3);
            auto maxIndicesVec = indicesVec;
            auto maxValuesVec = _mm_loadu_ps(pSrc); // Start with 0

            for (size_t i = 4; i < length; i += 4)
            {
                // Increment indices
                indicesVec = _mm_add_ps(indicesVec, incVec);

                auto src = _mm_loadu_ps(pSrc + i);

                // See if we have any new max values
                auto gt = _mm_cmpgt_ps(src, maxValuesVec);

                // Copy over the indices of new max values
#ifdef __SSE4_1__ // SSE4 implementation
                maxIndicesVec = _mm_blendv_ps(maxIndicesVec, indicesVec, gt);
#else // pre-SSE4 implementation
                auto maxIndicesMask = _mm_andnot_ps(gt, maxIndicesVec);
                auto indicesMask = _mm_and_ps(gt, indicesVec);
                maxIndicesVec = _mm_or_ps(maxIndicesMask, indicesMask);
#endif
                // Save the new max values
                maxValuesVec = _mm_max_ps(src, maxValuesVec);
            }

            // Find the max in the vectors
            _mm_storeu_ps(maxValues, maxValuesVec);
            _mm_storeu_ps(maxIndices, maxIndicesVec);

            // Start with 0
            auto maxIndex = maxIndices[0];
            auto maxValue = maxValues[0];
            for (int i = 1; i < 4; i++)
            {
                if (maxValues[i] >= maxValue)
                {
                    maxValue = maxValues[i];
                    maxIndex = maxIndices[i];
                }
            }

            // Get the uint version of the float
            uint32_t finalResult = static_cast<uint32_t>(maxIndex);

            // Deal with residual
            const uint32_t residualStart = (uint32_t)(length / 4) * 4;
            for (auto i = residualStart; i < length; i++)
            {
                if (pSrc[i] > maxValue)
                {
                    maxValue = pSrc[i];
                    finalResult = i;
                }
            }

            return finalResult;
        }
    } // namespace Arithmetic_Sse2
} // namespace VectorMath
#endif // defined(ARCH_X86) || defined(ARCH_X64)