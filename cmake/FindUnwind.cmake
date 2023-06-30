# FindUnwind.cmake - Find libunwind package.
#
# Copyright 2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

#-----------------------------------------------------------------------
# Use pkg-config to search for the module.
#-----------------------------------------------------------------------
find_package(PkgConfig)

pkg_check_modules(UNWIND QUIET libunwind)

#-----------------------------------------------------------------------
# Find libraries and include directories.
#-----------------------------------------------------------------------
find_path(UNWIND_INCLUDE_DIR
    NAMES "unwind.h"
    PATHS ${UNWIND_INCLUDE_DIRS}
)

find_library(UNWIND_LIBRARY
    NAMES unwind
    PATHS ${UNWIND_LIBRARY_DIRS}
)

#-----------------------------------------------------------------------
# Get version number
#-----------------------------------------------------------------------
if(UNWIND_VERSION)
    set(UNWIND_VERSION ${UNWIND_VERSION} CACHE STRING "libunwind version")
endif()

#-----------------------------------------------------------------------
# Handle REQUIRED and QUIETLY arguments
#-----------------------------------------------------------------------
find_package_handle_standard_args(UNWIND
    REQUIRED_VARS
        UNWIND_LIBRARY
        UNWIND_INCLUDE_DIR
    VERSION_VAR
        UNWIND_VERSION
)

mark_as_advanced(UNWIND_INCLUDE_DIR UNWIND_LIBRARY UNWIND_VERSION)

#-----------------------------------------------------------------------
# Define library target
#-----------------------------------------------------------------------
add_library(unwind UNKNOWN IMPORTED)

set_target_properties(unwind PROPERTIES
    IMPORTED_LOCATION ${UNWIND_LIBRARY}
    INTERFACE_INCLUDE_DIRECTORIES ${UNWIND_INCLUDE_DIR}
    IMPORTED_LINK_INTERFACE_LANGUAGES "C"
)
