// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
#pragma once
#if defined(ANDROID)
#include <malloc.h>
#endif
#include <vector>
#include "VectorMath.h"

namespace AlignedStore
{
    // free standing aligned alloc and free functions
    inline void* aligned_malloc(size_t size, size_t alignment)
    {
#if (WINDOWS || DURANGO)
        return _aligned_malloc(size, alignment);
#elif (LINUX || APPLE)
        void* ptr = nullptr;
        if (posix_memalign(&ptr, alignment, size) != 0)
        {
            ptr = nullptr;
        }
        return ptr;
#elif (ANDROID)
        return memalign(alignment, size);
#else
        static_assert(false, "Not a valid platform?");
        return nullptr;
#endif
    }

    inline void aligned_free(void* ptr)
    {
#if (WINDOWS || DURANGO)
        _aligned_free(ptr);
#elif (LINUX || ANDROID || APPLE)
        free(ptr);
#endif
    }

    template <typename T, uint32_t alignment = VectorMath::GetMinimumRequiredAlignment()>
    T* AlignedAlloc(size_t size)
    {
        auto ptr = static_cast<T*>(aligned_malloc(size * sizeof(T), alignment));
        if (ptr == nullptr)
        {
            throw std::bad_alloc();
        }
        return ptr;
    }

    static inline float* AllocateFloatBuffer(uint32_t size)
    {
        return AlignedStore::AlignedAlloc<float>(size);
    }

    static inline VectorMath::floatFC* AllocateComplexBuffer(uint32_t size)
    {
        return AlignedStore::AlignedAlloc<VectorMath::floatFC>(size);
    }

    struct AlignedFree
    {
        inline void operator()(void* p)
        {
            aligned_free(p);
        }
    };

    static inline bool IsAligned(const void* address, uint32_t alignment = VectorMath::GetMinimumRequiredAlignment())
    {
        return ((reinterpret_cast<uintptr_t>(address) & (alignment - 1)) == 0);
    }

    template <typename T, uint32_t alignment = VectorMath::GetMinimumRequiredAlignment()>
    uint32_t GetAlignedSize(uint32_t length)
    {
        auto bytes = length * sizeof(T);
        auto unalignedBytes = bytes % alignment;
        auto paddingBytes = unalignedBytes > 0 ? alignment - unalignedBytes : 0;
        auto alignedBytes = paddingBytes + bytes;
        return static_cast<uint32_t>(alignedBytes);
    }

    typedef std::unique_ptr<float[], AlignedStore::AlignedFree> FloatBuffer;
    typedef std::unique_ptr<VectorMath::floatFC[], AlignedStore::AlignedFree> ComplexBuffer;
    typedef std::unique_ptr<uint32_t[]> UIntBuffer;

    // Custom allocator - returns aligned allocations.
    template <typename T, uint32_t alignment = VectorMath::GetMinimumRequiredAlignment()>
    class AlignedAllocator : public std::allocator<T>
    {
    public:
        template <class U>
        struct rebind
        {
            // convert this type to an allocator<U>
            typedef AlignedAllocator<U, alignment> other;
        };

        // Default allocator
        AlignedAllocator() = default;

        // No copy constructor
        AlignedAllocator(const AlignedAllocator<void, alignment>&) = delete;

        template <class U>
        AlignedAllocator(const std::allocator<U>&)
        {
            // Construct from related allocator (do nothing)
        }

        template <class U>
        AlignedAllocator<void, alignment>& operator=(const AlignedAllocator<U, alignment>&)
        {
            // Assign from a related allocator (do nothing)
            return (*this);
        }

        T* allocate(size_t n)
        {
            return AlignedAlloc<T, alignment>(n * sizeof(T));
        }

        void deallocate(T* ptr, size_t)
        {
            if (ptr)
            {
                aligned_free(ptr);
            }
        }
    };

    //
    // Alias for aligned std::vector specialization
    //
    template <typename T, uint32_t alignment = VectorMath::GetMinimumRequiredAlignment()>
    using aligned_vector = std::vector<T, AlignedAllocator<T, alignment>>;

} // namespace  AlignedStore
