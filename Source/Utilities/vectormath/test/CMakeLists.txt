# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.
project (VectorMathTests)

# No need to build test for UWP
if (NOT ${CMAKE_SYSTEM_NAME} STREQUAL WindowsStore)
    add_executable(${PROJECT_NAME} vectormath_tests.cpp)

    # Enable whole program optimization for all DLLs/EXEs
    if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GL")
    endif()

    include_directories (
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${EXTERNAL_LIB_PATH}/googletest/googletest/include/gtest)

    target_link_libraries(${PROJECT_NAME} 
        gtest_main 
        VectorMath)

    gtest_add_tests(TARGET ${PROJECT_NAME})
endif ()