# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.
set (CMAKE_FOLDER VectorMath)
project(VectorMath)

add_library (${PROJECT_NAME}
  vectormath_factory.cpp
  vectormath_generic.cpp
  vectormath_generic.h
  vectormath_neon.cpp
  vectormath_neon.h
  vectormath_sse2.cpp
  vectormath_sse2.h)

set_property(TARGET VectorMath PROPERTY POSITION_INDEPENDENT_CODE ON)

if (NOT ${CMAKE_TEST} MATCHES "FALSE")
    add_subdirectory (test)
endif()

