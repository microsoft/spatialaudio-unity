# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.
project(AudioPluginMicrosoftAcoustics)


# Enable whole program optimization for all DLLs/EXEs
if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GL")
endif ()

set (AUDIOPLUGIN_SRC
    ${COMMON_VERSION_RC}
    AudioPluginInterface.h
    AudioPluginUtil.cpp
    AudioPluginUtil.h
    AudioPluginMicrosoftAcoustics.def
    HrtfConstants.h
    HrtfWrapper.cpp
    HrtfWrapper.h
    Plugin_Spatializer.cpp
    Plugin_SpatialMixer.cpp
    PluginList.h)

set (AUDIOPLUGIN_LINK_LIBS
    VectorMath)

if (MSVC)
    set(AUDIOPLUGIN_SRC ${AUDIOPLUGIN_SRC}
    CircularBuffer.cpp
    CircularBuffer.h)

    set(AUDIOPLUGIN_LINK_LIBS ${AUDIOPLUGIN_LINK_LIBS}
    mmdevapi.lib
    windowsapp.lib
    mfuuid.lib)

    set (HRTFDSP_LIB ${EXTERNAL_LIB_PATH}/HrtfDsp/${CMAKE_SYSTEM_NAME}/${ARCHITECTURE}/HrtfDsp.lib)
elseif (ANDROID)
    set (HRTFDSP_LIB ${EXTERNAL_LIB_PATH}/HrtfDsp/${CMAKE_SYSTEM_NAME}/${ARCHITECTURE}/libHrtfDsp.so)
endif()

add_library (${PROJECT_NAME} SHARED ${AUDIOPLUGIN_SRC})

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PRODUCT_VERSION}
    SOVERSION ${PRODUCT_VERSION})

add_dependencies (${PROJECT_NAME}
    VectorMath)

target_link_libraries (${PROJECT_NAME}
    ${AUDIOPLUGIN_LINK_LIBS}
    ${HRTFDSP_LIB})