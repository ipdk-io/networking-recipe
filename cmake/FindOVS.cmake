# FindOVS.cmake - find Open vSwitch package.
#
# Copyright 2022-2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

#-----------------------------------------------------------------------
# Use pkg-config to search for the modules
#-----------------------------------------------------------------------
find_package(PkgConfig REQUIRED)
pkg_check_modules(PC_OVS libopenvswitch REQUIRED)

#-----------------------------------------------------------------------
# Find include directory
# We rely on find_package_handle_standard_args() to check for errors
#-----------------------------------------------------------------------
find_path(OVS_INCLUDE_DIR
    NAMES "openvswitch/version.h"
    PATHS ${PC_OVS_INCLUDE_DIRS}
)
mark_as_advanced(OVS_INCLUDE_DIR)

#-----------------------------------------------------------------------
# Find libraries
# We rely on find_package_handle_standard_args() to check for errors
# Not all libraries are required
#-----------------------------------------------------------------------
find_library(OVS_LIBRARY
    NAMES openvswitch
    PATHS ${PC_OVS_LIBRARY_DIRS}
)
mark_as_advanced(OVS_LIBRARY)

find_library(OVS_LIBAVX512
    NAMES openvswitchavx512
    PATHS ${PC_OVS_LIBRARY_DIRS}
)
mark_as_advanced(OVS_LIBAVX512)

find_library(OFPROTO_LIBRARY
    NAMES ofproto
    PATHS ${PC_OVS_LIBRARY_DIRS}
)
mark_as_advanced(OFPROTO_LIBRARY)

find_library(OVSDB_LIBRARY
    NAMES ovsdb
    PATHS ${PC_OVS_LIBRARY_DIRS}
)
mark_as_advanced(OVSDB_LIBRARY)

find_library(SFLOW_LIBRARY
    NAMES sflow
    PATHS ${PC_OVS_LIBRARY_DIRS}
)
mark_as_advanced(SFLOW_LIBRARY)

find_library(LIBVSWITCHD
    NAMES vswitchd
    PATHS ${PC_OVS_LIBRARY_DIRS}
)
mark_as_advanced(LIBVSWITCHD)

find_library(LIBTESTCONTROLLER
    NAMES testcontroller
    PATHS ${PC_OVS_LIBRARY_DIRS}
)
mark_as_advanced(LIBTESTCONTROLLER)

#-----------------------------------------------------------------------
# Get version number
#-----------------------------------------------------------------------
if(PC_OVS_VERSION)
    set(OVS_VERSION ${PC_OVS_VERSION} CACHE STRING "OVS version")
    mark_as_advanced(OVS_VERSION)
elseif(EXISTS "${OVS_INCLUDE_DIR}/openvswitch/version.h")
    # Extract version string from version.h file.
    file(STRINGS
        "${OVS_INCLUDE_DIR}/openvswitch/version.h"
        _version_string REGEX "OVS_PACKAGE_VERSION")
    string(REGEX REPLACE
        "[^0-9.]+([0-9.]+).*" "\\1" OVS_VERSION ${_version_string})
    set(OVS_VERSION ${PC_OVS_VERSION} CACHE STRING "OVS version")
    mark_as_advanced(OVS_VERSION)
    unset(_version_string)
endif()

#-----------------------------------------------------------------------
# Handle REQUIRED and QUIET arguments
#-----------------------------------------------------------------------
find_package_handle_standard_args(OVS
    REQUIRED_VARS
        OVS_LIBRARY
        OVS_INCLUDE_DIR
        OFPROTO_LIBRARY
        SFLOW_LIBRARY
    VERSION_VAR
        OVS_VERSION
)

#-----------------------------------------------------------------------
# Define library targets
#-----------------------------------------------------------------------
add_library(ovs::openvswitch UNKNOWN IMPORTED)
set_target_properties(ovs::openvswitch PROPERTIES
    IMPORTED_LOCATION ${OVS_LIBRARY}
    INTERFACE_INCLUDE_DIRECTORIES ${OVS_INCLUDE_DIR}
    IMPORTED_LINK_INTERFACE_LANGUAGES "C"
)

add_library(ovs::ovsdb UNKNOWN IMPORTED)
set_target_properties(ovs::ovsdb PROPERTIES
    IMPORTED_LOCATION ${OVSDB_LIBRARY}
    INTERFACE_INCLUDE_DIRECTORIES ${OVS_INCLUDE_DIR}
    IMPORTED_LINK_INTERFACE_LANGUAGES "C"
)

add_library(ovs::ofproto UNKNOWN IMPORTED)
set_target_properties(ovs::ofproto PROPERTIES
    IMPORTED_LOCATION ${OFPROTO_LIBRARY}
    INTERFACE_INCLUDE_DIRECTORIES ${OVS_INCLUDE_DIR}
    IMPORTED_LINK_INTERFACE_LANGUAGES "C"
)

add_library(ovs::sflow UNKNOWN IMPORTED)
set_target_properties(ovs::sflow PROPERTIES
    IMPORTED_LOCATION ${SFLOW_LIBRARY}
    INTERFACE_INCLUDE_DIRECTORIES ${OVS_INCLUDE_DIR}
    IMPORTED_LINK_INTERFACE_LANGUAGES "C"
)

if(LIBVSWITCHD)
    add_library(ovs::vswitchd UNKNOWN IMPORTED)
    set_target_properties(ovs::vswitchd PROPERTIES
        IMPORTED_LOCATION ${LIBVSWITCHD}
        INTERFACE_INCLUDE_DIRECTORIES ${OVS_INCLUDE_DIR}
        IMPORTED_LINK_INTERFACE_LANGUAGES "C"
    )
endif()

if(LIBTESTCONTROLLER)
    add_library(ovs::testcontroller UNKNOWN IMPORTED)
    set_target_properties(ovs::testcontroller PROPERTIES
        IMPORTED_LOCATION ${LIBTESTCONTROLLER}
        INTERFACE_INCLUDE_DIRECTORIES ${OVS_INCLUDE_DIR}
        IMPORTED_LINK_INTERFACE_LANGUAGES "C"
    )
endif()

if(OVS_LIBAVX512)
    add_library(ovs::avx512 UNKNOWN IMPORTED)
    set_target_properties(ovs::avx512 PROPERTIES
        IMPORTED_LOCATION ${OVS_LIBAVX512}
        INTERFACE_INCLUDE_DIRECTORIES ${OVS_INCLUDE_DIR}
        IMPORTED_LINK_INTERFACE_LANGUAGES "C"
    )
endif()
