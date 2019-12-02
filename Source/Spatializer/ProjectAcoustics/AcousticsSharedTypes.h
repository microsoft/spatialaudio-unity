// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
//! \file AcousticsSharedTypes.h
//! \brief Common vector types to be used with the acoustics toolkit

#pragma once

//! A simple vector of floats
typedef struct ATKVectorF
{
    //! The x-component of the vector
    float x;
    //! The y-component of the vector
    float y;
    //! The z-component of the vector
    float z;
} VectorF;

//! A simple vector of ints
typedef struct ATKVectorI
{
    //! The x-component of the vector
    int x;
    //! The y-component of the vector
    int y;
    //! The z-component of the vector
    int z;
} VectorI;

//! A matrix of floats, with row-column index
typedef struct ATKMatrix4x4
{
    //! Component in row 1, column 1
    float m11;
    //! Component in row 1, column 2
    float m12;
    //! Component in row 1, column 3
    float m13;
    //! Component in row 1, column 4
    float m14;
    //! Component in row 2, column 1
    float m21;
    //! Component in row 2, column 2
    float m22;
    //! Component in row 2, column 3
    float m23;
    //! Component in row 2, column 4
    float m24;
    //! Component in row 3, column 1
    float m31;
    //! Component in row 3, column 2
    float m32;
    //! Component in row 3, column 3
    float m33;
    //! Component in row 3, column 4
    float m34;
    //! Component in row 4, column 1
    float m41;
    //! Component in row 4, column 2
    float m42;
    //! Component in row 4, column 3
    float m43;
    //! Component in row 4, column 4
    float m44;
} ATKMatrix4x4;

//! A pointer to an object returned from this API.
//! OBJECT_HANDLEs are always validated internally before use
typedef const void* OBJECT_HANDLE;

//! RAII helper class
#ifdef __cplusplus
template <typename deleter>
class UniqueObjectHandle final
{
public:
    UniqueObjectHandle()
    {
    }
    //! Constructs a UniqueObjectHandle that owns the provided handle
    //! \param o The object handle for which to own the lifetime
    UniqueObjectHandle(OBJECT_HANDLE o) : m_ObjectHandle(o)
    {
    }
    //! Move constructor that takes over ownership from another UniqueObjectHandle
    //! \param o The rvalue reference from which to take ownership
    UniqueObjectHandle(UniqueObjectHandle&& o) : m_ObjectHandle(o.m_ObjectHandle)
    {
        o.m_ObjectHandle = nullptr;
    }
    ~UniqueObjectHandle()
    {
        m_Deleter(m_ObjectHandle);
    }

    //! Returns the value of the handle held by this instance
    //! \return The value of the handle held by this instance
    inline OBJECT_HANDLE Get() const
    {
        return m_ObjectHandle;
    }

    //! Useful for outparam allocation functions.
    //! \return A non-const pointer to the object handle member this object owns
    inline OBJECT_HANDLE* operator&()
    {
        return &m_ObjectHandle;
    }

private:
    OBJECT_HANDLE m_ObjectHandle;
    deleter m_Deleter;
};
#endif

#ifndef EXPORT_API
#ifdef _MSC_VER
#define EXPORT_API __cdecl
#else
#define EXPORT_API __attribute__((__visibility__("default")))
#endif
#endif
