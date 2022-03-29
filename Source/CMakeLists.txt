# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.
# Common versioning for managed and unmanaged assemblies
init_product_versioning(PRODUCT_VERSION COMMON_ASSEMBLY_INFO COMMON_VERSION_RC)

# Product version definition for components
add_compile_definitions(PRODUCT_VERSION=${PRODUCT_VERSION})

# Add platform specific defines
if (WIN32)
    set(ARCHITECTURE ${CMAKE_VS_PLATFORM_NAME})
    add_definitions(-DWINDOWS)
    if (${CMAKE_SYSTEM_NAME} STREQUAL WindowsStore)
        add_definitions(-DWINDOWSSTORE)
        set (WINDOWSSTORE TRUE)
    else()
        add_definitions(-DWINDOWSDESKTOP)
        set (WINDOWSDESKTOP TRUE)
    endif()
elseif (${CMAKE_SYSTEM_NAME} STREQUAL Android)
    set(ARCHITECTURE ${CMAKE_ANDROID_ARCH_ABI})
    add_definitions(-DANDROID)
    set (ANDROID TRUE)
endif ()

# Configuration
set (EXTERNAL_LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/External)

# HrtfDsp Version and Path
set (HRTFDSP_VERSION "Microsoft.ProjectAcoustics.HrtfDsp.3.0.125-prerelease")

include_directories (
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${EXTERNAL_LIB_PATH}
    ${EXTERNAL_LIB_PATH}/wil/include
    ${EXTERNAL_LIB_PATH}/${HRTFDSP_VERSION}/HrtfDsp/include)

# Compiler and linker options
if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    set(VCVER "vc142")

    string(REGEX REPLACE " /W([0-9]+)( |$)" " " CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    string(REGEX REPLACE " /EH([^ ]*)( |$)" " " CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /W4 /WX /fp:fast /guard:cf")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /W4 /WX /fp:fast /guard:cf")

    # Disable incremental linking, enable link-time code generation, enable control flow guard
    set(ADDITIONAL_LINKER_FLAGS "/INCREMENTAL:NO /LTCG /guard:cf /DYNAMICBASE /WX")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${ADDITIONAL_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} ${ADDITIONAL_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO} ${ADDITIONAL_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${ADDITIONAL_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO} ${ADDITIONAL_LINKER_FLAGS}")
    set(CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO} ${ADDITIONAL_LINKER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${ADDITIONAL_LINKER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${ADDITIONAL_LINKER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} ${ADDITIONAL_LINKER_FLAGS}")

    # Disable warnings & errors regarding unsecure CRT functions
    add_definitions(-D _CRT_SECURE_NO_WARNINGS -DNOMINMAX)
endif()

# Set compiler flags for custom 'research' build configuration
# Same as 'RelWithDebInfo' except without /DNDEBUG, and with MATLAB interop
string(REPLACE "/DNDEBUG" "" CMAKE_CXX_FLAGS_RESEARCH ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
set(CMAKE_CXX_FLAGS_RESEARCH "${CMAKE_CXX_FLAGS_RESEARCH} /DDEVELOPMENT")
set(CMAKE_SHARED_LINKER_FLAGS_RESEARCH "${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO}")
set(CMAKE_EXE_LINKER_FLAGS_RESEARCH "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO}")
set(CMAKE_MODULE_LINKER_FLAGS_RESEARCH "${CMAKE_MODULE_LINKER_FLAGS_RELWITHDEBINFO}")
set(CMAKE_CXX_FLAGS_RESEARCH "${CMAKE_CXX_FLAGS_RESEARCH} /DBUILD_INTEROP_MATLAB")

add_subdirectory (External)
add_subdirectory (Spatializer)
add_subdirectory (Utilities)
