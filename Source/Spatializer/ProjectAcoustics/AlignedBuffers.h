// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
#pragma once
#include "AlignedAllocator.h"
#include "ConstVector.h"
#include <stdexcept>

namespace AlignedStore
{
    template <typename T>
    struct AlignedBufferConst;

    template <typename T>
    struct AlignedBuffer
    {
        AlignedBuffer()
        {
        }
        AlignedBuffer(T* data, uint32_t length);

        AlignedBufferConst<T> ConstBuffer;
        T* Data;
    };

    template <typename T>
    struct AlignedBufferConst
    {
        AlignedBufferConst()
        {
        }
        inline AlignedBufferConst(const AlignedBuffer<T>& t)
            : ConstData(t.ConstBuffer.ConstData), Length(t.ConstBuffer.Length)
        {
        }
        inline AlignedBufferConst(const T* data, uint32_t length) : ConstData(data), Length(length)
        {
        }
        const T* ConstData;
        uint32_t Length;
    };

    template <typename T>
    inline AlignedBuffer<T>::AlignedBuffer(T* data, uint32_t length) : ConstBuffer(data, length), Data(data)
    {
    }

    template <typename T, uint32_t alignment = VectorMath::GetMinimumRequiredAlignment()>
    class AlignedBuffersConst
    {
    public:
        // Expects data to be correctly aligned.
        void Initialize(const uint8_t* data, uint32_t dataSize, uint32_t numBuffers, uint32_t bufferLength)
        {
            auto bytesPerBuffer = GetAlignedSize<T, alignment>(bufferLength);
            if (!IsAligned(data, alignment) || dataSize < (bytesPerBuffer * numBuffers))
            {
                throw std::invalid_argument("");
            }

            m_BytesPerBuffer = bytesPerBuffer;
            m_BufferLength = bufferLength;
            m_NumBuffers = numBuffers;
            m_Data = data;
            m_DataSize = dataSize;
        }

        uint32_t GetSize() const
        {
            return m_DataSize;
        }

        template <typename U>
        const U* GetData() const
        {
            return reinterpret_cast<const U*>(m_Data);
        }

        const T& GetAt(uint32_t element) const
        {
            return *GetPtrAt(element);
        }

        uint32_t GetNumBuffers() const
        {
            return m_NumBuffers;
        }

        uint32_t GetBufferLength() const
        {
            return m_BufferLength;
        }

        const AlignedBufferConst<T> operator[](uint32_t element) const
        {
            return AlignedBufferConst<T>(GetPtrAt(element), m_BufferLength);
        }

    protected:
        const T* GetPtrAt(uint32_t element) const
        {
            return reinterpret_cast<const T*>(m_Data + element * m_BytesPerBuffer);
        }

    protected:
        const uint8_t* m_Data = nullptr;
        uint32_t m_BytesPerBuffer = 0;
        uint32_t m_BufferLength = 0;
        uint32_t m_NumBuffers = 0;
        uint32_t m_DataSize = 0;
    };

    template <typename T, uint32_t alignment = VectorMath::GetMinimumRequiredAlignment()>
    class AlignedBuffers : public AlignedBuffersConst<T, alignment>
    {
    public:
        AlignedBuffers() = default;

        AlignedBuffers(AlignedBuffers&& other)
            : AlignedBuffersConst<T, alignment>(other), m_Data(std::move(other.m_Data))
        {
        }

        AlignedBuffers& operator=(AlignedBuffers&& other)
        {
            m_Data = std::move(other.m_Data);
            AlignedBuffersConst<T, alignment>::m_Data = other.AlignedBuffersConst<T, alignment>::m_Data;
            AlignedBuffersConst<T, alignment>::m_BytesPerBuffer = other.m_BytesPerBuffer;
            AlignedBuffersConst<T, alignment>::m_BufferLength = other.m_BufferLength;
            AlignedBuffersConst<T, alignment>::m_NumBuffers = other.m_NumBuffers;
            AlignedBuffersConst<T, alignment>::m_DataSize = other.m_DataSize;
            return *this;
        }

        AlignedBuffers(uint32_t numBuffers, uint32_t bufferLength)
        {
            auto size = GetAlignedSize<T, alignment>(bufferLength) * numBuffers;
            m_Data.reset(AlignedStore::AlignedAlloc<uint8_t, alignment>(size));
            AlignedBuffersConst<T, alignment>::Initialize(m_Data.get(), size, numBuffers, bufferLength);
        }

        void Clear()
        {
            memset(m_Data.get(), 0, AlignedBuffersConst<T, alignment>::GetSize());
        }

        T& GetAt(uint32_t element)
        {
            return *GetPtrAt(element);
        }

        const AlignedBufferConst<T> operator[](uint32_t element) const
        {
            return AlignedBuffersConst<T, alignment>::operator[](element);
        }

        AlignedBuffer<T> operator[](uint32_t element)
        {
            return AlignedBuffer<T>(GetPtrAt(element), AlignedBuffersConst<T, alignment>::m_BufferLength);
        }

    private:
        T* GetPtrAt(uint32_t element)
        {
            return reinterpret_cast<T*>(m_Data.get() + element * AlignedBuffersConst<T, alignment>::m_BytesPerBuffer);
        }

    private:
        std::unique_ptr<uint8_t, AlignedFree> m_Data;
    };

} // namespace AlignedStore
