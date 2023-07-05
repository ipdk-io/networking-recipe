# FindUnbound.cmake - Find libunbound package.
#
# Copyright 2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

#-----------------------------------------------------------------------
# Use pkg-config to search for the module.
#-----------------------------------------------------------------------
find_package(PkgConfig)

pkg_check_modules(UNBOUND QUIET libunbound)

#-----------------------------------------------------------------------
# Find libraries and include directories.
#-----------------------------------------------------------------------
find_path(UNBOUND_INCLUDE_DIR
    NAMES "unbound.h"
    PATHS ${UNBOUND_INCLUDE_DIRS}
)

find_library(UNBOUND_LIBRARY
    NAMES unbound
    PATHS ${UNBOUND_LIBRARY_DIRS}
)

#-----------------------------------------------------------------------
# Get version number
#-----------------------------------------------------------------------
if(UNBOUND_VERSION)
    set(UNBOUND_VERSION ${UNBOUND_VERSION} CACHE STRING "libunbound version")
endif()

#-----------------------------------------------------------------------
# Handle REQUIRED and QUIET arguments
#-----------------------------------------------------------------------
find_package_handle_standard_args(UNBOUND
    REQUIRED_VARS
        UNBOUND_LIBRARY
        UNBOUND_INCLUDE_DIR
    VERSION_VAR
        UNBOUND_VERSION
)

mark_as_advanced(UNBOUND_INCLUDE_DIR UNBOUND_LIBRARY UNBOUND_VERSION)

#-----------------------------------------------------------------------
# Define library target
#-----------------------------------------------------------------------
add_library(unbound UNKNOWN IMPORTED)

set_target_properties(unbound PROPERTIES
    IMPORTED_LOCATION ${UNBOUND_LIBRARY}
    INTERFACE_INCLUDE_DIRECTORIES ${UNBOUND_INCLUDE_DIR}
    IMPORTED_LINK_INTERFACE_LANGUAGES "C"
)
