// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
#pragma once

// Constant vector iterator.
template <typename T>
class vector_const_iterator
{
public:
    // Because of a compiler mismatch between windows and vs2017, we need to manually define these instead of inheriting
    using iterator_category = std::random_access_iterator_tag;
    using pointer = const T*;
    using reference = const T&;
    using difference_type = ptrdiff_t;
    using value_type = T;

    // The type of an unsigned distance between two elements.
    typedef size_t size_type;

    // The type of this template.
    typedef vector_const_iterator<T> this_type;

    // Constructor with null pointer.
    vector_const_iterator()
    {
        m_Base = 0;
    }

    // Construct with pointer and offset.
    explicit vector_const_iterator(pointer ptr, size_t offset = 0)
    {
        m_Base = ptr + offset;
    }

    // Return designated object.
    reference operator*() const
    {
        return *m_Base;
    }

    // Return pointer to class object.
    pointer operator->() const
    {
        return m_Base;
    }

    // Preincrement.
    this_type& operator++()
    {
        ++m_Base;
        return *this;
    }

    // Postincrement.
    this_type operator++(int)
    {
        this_type temp = *this;
        ++*this;
        return temp;
    }

    // Predecrement
    this_type& operator--()
    {
        --m_Base;
        return *this;
    }

    // Postdecrement.
    this_type operator--(int)
    {
        this_type temp = *this;
        --*this;
        return temp;
    }

    // Increment by integer.
    this_type& operator+=(difference_type offset)
    {
        m_Base += offset;
        return *this;
    }

    // Return this + integer.
    this_type operator+(difference_type offset) const
    {
        this_type temp = *this;
        return temp += offset;
    }

    // Decrement by integer.
    this_type& operator-=(difference_type offset)
    {
        return *this += -offset;
    }

    // Return this - integer.
    this_type operator-(difference_type offset) const
    {
        this_type temp = *this;
        return temp -= offset;
    }

    // Return difference of iterators.
    difference_type operator-(const this_type& right) const
    {
        return m_Base - right.m_Base;
    }

    // Return difference of iterators.
    reference operator[](difference_type offset) const
    {
        return *(*this + offset);
    }

    // Test for iterator equality.
    bool operator==(const this_type& right) const
    {
        return m_Base == right.m_Base;
    }

    // Test for iterator inequality
    bool operator!=(const this_type& right) const
    {
        return !(*this == right);
    }

    // Test if this < right.
    bool operator<(const this_type& right) const
    {
        return m_Base < right.m_Base;
    }

    // Test if this > right.
    bool operator>(const this_type& right) const
    {
        return right < *this;
    }

    // Test if this <= right.
    bool operator<=(const this_type& right) const
    {
        return !(right < *this);
    }

    // Test if this >= right.
    bool operator>=(const this_type& right) const
    {
        return !(*this < right);
    }

private:
    // Beginning of the vector.
    pointer m_Base;
};

template <typename T>
class ConstVector
{
public:
    typedef vector_const_iterator<T> const_iterator;

public:
    ConstVector() = default;
    ConstVector(const ConstVector&) = default;
    ConstVector& operator=(const ConstVector&) = default;
    ConstVector& operator=(ConstVector&& ref) = default;

    void Initialize(const uint8_t* data, uint32_t dataSize)
    {
        if (!data || !AlignedStore::IsAligned(data, sizeof(T)))
        {
            throw std::invalid_argument("");
        }
        m_Base = reinterpret_cast<const T*>(data);
        m_Length = dataSize / sizeof(T);
    }

    uint32_t size() const
    {
        return m_Length;
    }

    const T* data() const
    {
        return m_Base;
    }

    const T& operator[](uint32_t index) const
    {
        // Note no bounds check same as std::vector
        return *(m_Base + index);
    }

    const_iterator begin() const
    {
        return vector_const_iterator<T>(m_Base);
    }

    const_iterator end() const
    {
        return vector_const_iterator<T>(m_Base + m_Length);
    }

private:
    const T* m_Base = nullptr; // Base ptr for the mapped data
    uint32_t m_Length = 0;     // Total number of elements
};
