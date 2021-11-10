# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.
project(AudioPluginMicrosoftSpatializerCrossPlatform)


# Enable whole program optimization for all DLLs/EXEs
if (${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GL")
endif ()

set (AUDIOPLUGIN_SRC
    ${COMMON_VERSION_RC}
    AudioPluginInterface.h
    AudioPluginUtil.cpp
    AudioPluginUtil.h
    HrtfConstants.h
    HrtfWrapper.cpp
    HrtfWrapper.h
    SpatializerPlugin.cpp
    SpatializerMixerPlugin.cpp
    PluginList.h)

if (MSVC)
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
    VectorMath
    ${HRTFDSP_LIB})

# Copy external dependencies
if (WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${EXTERNAL_LIB_PATH}/HrtfDsp/${CMAKE_SYSTEM_NAME}/${ARCHITECTURE}/HrtfDsp.dll"
        $<TARGET_FILE_DIR:AudioPluginMicrosoftSpatializerCrossPlatform>)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${EXTERNAL_LIB_PATH}/HrtfDsp/${CMAKE_SYSTEM_NAME}/${ARCHITECTURE}/HrtfDsp.pdb"
        $<TARGET_FILE_DIR:AudioPluginMicrosoftSpatializerCrossPlatform>)
elseif (ANDROID)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${EXTERNAL_LIB_PATH}/HrtfDsp/${CMAKE_SYSTEM_NAME}/${ARCHITECTURE}/libHrtfDsp.so"
        $<TARGET_FILE_DIR:AudioPluginMicrosoftSpatializerCrossPlatform>)
endif ()