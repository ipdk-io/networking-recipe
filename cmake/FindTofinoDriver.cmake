# FindTofinoDriver.cmake - import Tofino P4 Driver (SDE).
#
# Copyright 2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

# This module requires that SDE_INSTALL_DIR be defined.
# It needs additional work to make it a proper CMake find-module.

#-----------------------------------------------------------------------
# Define SDE_INCLUDE_DIR
#-----------------------------------------------------------------------
find_path(SDE_INCLUDE_DIR
    NAMES "bf_switchd/bf_switchd.h"
    PATHS ${SDE_INSTALL_DIR}/include
)
if(NOT SDE_INCLUDE_DIR)
  message(STATUS "bf_switchd.h not found")
  message("---------------------------------------------")
  message("Your SDE_INSTALL_DIR appears to be incorrect.")
  message("Its current value is '${SDE_INSTALL_DIR}'.")
  message("---------------------------------------------")
  message(FATAL_ERROR "Tofino SDE setup failed")
endif()
mark_as_advanced(SDE_INCLUDE_DIR)

#-----------------------------------------------------------------------
# Add SDE install directory to search path
#-----------------------------------------------------------------------
list(APPEND CMAKE_PREFIX_PATH ${SDE_INSTALL_DIR})

#-----------------------------------------------------------------------
# Find libraries
#-----------------------------------------------------------------------
find_library(LIBDRIVER driver)
if(NOT LIBDRIVER)
  message(FATAL_ERROR "sde::driver library not found")
endif()
mark_as_advanced(LIBDRIVER)

find_library(LIBTDI tdi)
if(NOT LIBTDI)
  message(FATAL_ERROR "sde::tdi library not found")
endif()
mark_as_advanced(LIBTDI)

find_library(LIBTDI_JSON_PARSER tdi_json_parser)
if(NOT LIBTDI_JSON_PARSER)
  message(FATAL_ERROR "sde::tdi_json_parser library not found")
endif()
mark_as_advanced(LIBTDI_JSON_PARSER)

find_library(LIBTARGET_SYS target_sys)
if(NOT LIBTARGET_SYS)
  message(FATAL_ERROR "sde::target_sys library not found")
endif()
mark_as_advanced(LIBTARGET_SYS)

find_library(LIBTARGET_UTILS target_utils)
if(NOT LIBTARGET_UTILS)
  message(FATAL_ERROR "sde::target_utils library not found")
endif()
mark_as_advanced(LIBTARGET_UTILS)

#-----------------------------------------------------------------------
# Get version number
#-----------------------------------------------------------------------
if(EXISTS ${SDE_INSTALL_DIR}/share/VERSION)
  file(READ ${SDE_INSTALL_DIR}/share/VERSION _sde_version)
  string(STRIP "${_sde_version}" _sde_version)
  if(NOT _sde_version STREQUAL "")
    set(SDE_VERSION "${_sde_version}" CACHE STRING "Tofino SDE version")
    mark_as_advanced(SDE_VERSION)
  endif()
  unset(_sde_version)
endif()

#-----------------------------------------------------------------------
# Define library targets
#-----------------------------------------------------------------------
add_library(sde::driver UNKNOWN IMPORTED)
set_target_properties(sde::driver PROPERTIES
    IMPORTED_LOCATION ${LIBDRIVER}
    INTERFACE_INCLUDE_DIRECTORIES ${SDE_INCLUDE_DIR}
    IMPORTED_LINK_INTERFACE_LANGUAGES C)

add_library(sde::tdi UNKNOWN IMPORTED)
set_target_properties(sde::tdi PROPERTIES
    IMPORTED_LOCATION ${LIBTDI}
    INTERFACE_INCLUDE_DIRECTORIES ${SDE_INCLUDE_DIR}
    IMPORTED_LINK_INTERFACE_LANGUAGES CXX)

add_library(sde::tdi_json_parser UNKNOWN IMPORTED)
set_target_properties(sde::tdi_json_parser PROPERTIES
    IMPORTED_LOCATION ${LIBTDI_JSON_PARSER}
    INTERFACE_INCLUDE_DIRECTORIES ${SDE_INCLUDE_DIR}
    IMPORTED_LINK_INTERFACE_LANGUAGES CXX)

add_library(sde::target_sys UNKNOWN IMPORTED)
set_target_properties(sde::target_sys PROPERTIES
    IMPORTED_LOCATION ${LIBTARGET_SYS}
    INTERFACE_INCLUDE_DIRECTORIES ${SDE_INCLUDE_DIR}
    IMPORTED_LINK_INTERFACE_LANGUAGES C)

add_library(sde::target_utils UNKNOWN IMPORTED)
set_target_properties(sde::target_utils PROPERTIES
    IMPORTED_LOCATION ${LIBTARGET_UTILS}
    INTERFACE_INCLUDE_DIRECTORIES ${SDE_INCLUDE_DIR}
    IMPORTED_LINK_INTERFACE_LANGUAGES C)

#-----------------------------------------------------------------------
# Define SDE_LIBRARY_DIRS
#-----------------------------------------------------------------------
set(SDE_LIBRARY_DIRS ${SDE_INSTALL_DIR}/lib)

#-----------------------------------------------------------------------
# Apply SDE properties to target
#-----------------------------------------------------------------------
function(add_tofino_target_libraries TGT)
  target_link_libraries(${TGT} PUBLIC
      sde::driver
      sde::target_sys
      sde::tdi
      sde::tdi_json_parser
  )
  target_link_directories(${TGT} PUBLIC ${SDE_LIBRARY_DIRS})
endfunction()
