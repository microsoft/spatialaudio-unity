// Copyright (c) Microsoft Corp. All rights reserved

#include <cstddef>

#define IS_4BYTE_ALIGNED(p) (0 == (reinterpret_cast<ptrdiff_t>(p) & 0x03))
#define IS_NOT_16BYTE_ALIGNED(p) (reinterpret_cast<ptrdiff_t>(p) & 0x0F)
#define ARE_16BYTE_COALIGNED(pA, pB) (IS_NOT_16BYTE_ALIGNED(pA) == IS_NOT_16BYTE_ALIGNED(pB))
#define ARE_ARRAYS_ALIGNED(pDst, pSrcA, pSrcB)                                                                         \
    (IS_4BYTE_ALIGNED(pDst) && ARE_16BYTE_COALIGNED(pDst, pSrcA) && ARE_16BYTE_COALIGNED(pDst, pSrcB))
