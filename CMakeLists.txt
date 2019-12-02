# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.
cmake_minimum_required(VERSION 3.14)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
project (Microsoft.SpatialAudio.Unity)
set (CMAKE_CXX_STANDARD 17)
add_definitions(-D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS -D_ENFORCE_MATCHING_ALLOCATORS=0)

# Module to help with versioning
include (${CMAKE_CURRENT_SOURCE_DIR}/tools/product_version.cmake)

if (${CMAKE_TEST} MATCHES "TRUE")
    set(CMAKE_TEST "TRUE")
    enable_testing ()
    include(GoogleTest)

    # Make GTest link the runtime dynamically
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
else()
    set(CMAKE_TEST "FALSE")
endif()

# Account for build config when setting up output dirs
set (CONFIG_DIR $<$<CONFIG:Debug>:Debug>$<$<CONFIG:RelWithDebInfo>:RelWithDebInfo>$<$<CONFIG:Release>:Release>)
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CONFIG_DIR})
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CONFIG_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CONFIG_DIR})

# Add custom configuration for research builds (some optimizations, some debug)
if(CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES Debug;RelWithDebInfo)
    set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING
        "Reset to the configurations that we need"
        FORCE)
endif()

add_subdirectory (source)
