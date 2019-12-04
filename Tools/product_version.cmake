# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.
# Common versioning for managed and unmanaged assemblies
function (init_product_versioning PRODUCT_VERSION_OUT COMMON_ASSEMBLY_INFO_OUT COMMON_VERSION_RC_OUT)
    if (NOT DEFINED PRODUCT_VERSION)
        message ("Using product version defined in CMakeLists")
        # Set these values for a specific release
        set (PRODUCT_VERSION_MAJOR 0)
        set (PRODUCT_VERSION_MINOR 1)
        set (PRODUCT_VERSION_PATCH 0)
        set (PRODUCT_VERSION ${PRODUCT_VERSION_MAJOR}.${PRODUCT_VERSION_MINOR}.${PRODUCT_VERSION_PATCH})
    endif ()

    set (COMMON_VERSION_RC_IN "${CMAKE_SOURCE_DIR}/Source/include/CommonVersion.rc.in")

    configure_file(
        ${COMMON_VERSION_RC_IN}
        ${CMAKE_CURRENT_BINARY_DIR}/version.rc
        @ONLY)

    set ($ENV{PRODUCT_VERSION} PRODUCT_VERSION)
    set (${PRODUCT_VERSION_OUT} ${PRODUCT_VERSION} PARENT_SCOPE)
    set (${COMMON_VERSION_RC_OUT} "${CMAKE_CURRENT_BINARY_DIR}/version.rc" PARENT_SCOPE)
endfunction ()