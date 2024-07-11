# SetInstallRpath.cmake
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
# Functions defined:
#   set_install_rpath
#   strip_sysroot_prefix
#
# CMake variables:
#   CMAKE_INSTALL_LIBDIR
#   CMAKE_INSTALL_PREFIX
#   CMAKE_SYSROOT
#
# Input variables:
#   DEPEND_INSTALL_DIR
#   SDE_INSTALL_DIR
#   SET_RPATH
#
# Output variables:
#   DEP_ELEMENT
#   EXEC_ELEMENT
#   SDE_ELEMENT
#

#-----------------------------------------------------------------------
# strip_sysroot_prefix
#-----------------------------------------------------------------------

function(strip_sysroot_prefix _IN _OUT)
    string(REGEX REPLACE "^${CMAKE_SYSROOT}(.+)\$" "\\1" _out "${_IN}")
    set(${_OUT} ${_out} PARENT_SCOPE)
endfunction()

#-----------------------------------------------------------------------
# RPATH elements
#-----------------------------------------------------------------------

# TODO:
# If the SDE or DEPEND install tree is combined with the
# RECIPE install tree (which we might want to do for a runtime
# system), we can omit the SDE or DEPEND element in favor of
# the ORIGIN element.
#
# The ORIGIN element is equivalent to ${CMAKE_PREFIX_PATH}/lib or
# ${CMAKE_PREFIX_PATH}/lib64.
#
# The elements will need to be expanded to include /lib64 paths where
# appropriate.
#
# We shouldn't need an element for ${SDE_INSTALL_DIR}/lib/x86_64-linux-gnu.
# It's only used for DPDK libraries, which are transitive dependencies of
# the SDE libraries (specifically dpdk_infra.so). In the unlikely event
# that we do need to include the DPDK library directory in the RPATH, we
# can use DPDK_LIBRARY_DIRS, which is defined by pkg_check_modules(DPDK).

# SDE_ELEMENT
if(NOT SDE_INSTALL_DIR STREQUAL "")
    strip_sysroot_prefix(${SDE_INSTALL_DIR} _sde_stem)
    if(EXISTS ${SDE_INSTALL_DIR}/lib)
        list(APPEND SDE_ELEMENT ${_sde_stem}/lib)
    endif()
    if(EXISTS ${SDE_INSTALL_DIR}/lib64)
        list(APPEND SDE_ELEMENT ${_sde_stem}/lib64)
    endif()
    unset(_sde_stem)
endif()

# DEP_ELEMENT
if(NOT DEPEND_INSTALL_DIR STREQUAL "")
    strip_sysroot_prefix(${DEPEND_INSTALL_DIR} _dep_stem)
    if(EXISTS ${DEPEND_INSTALL_DIR}/lib)
        list(APPEND DEP_ELEMENT ${_dep_stem}/lib)
    endif()
    if(EXISTS ${DEPEND_INSTALL_DIR}/lib64)
        list(APPEND DEP_ELEMENT ${_dep_stem}/lib64)
    endif()
    unset(_dep_stem)
endif()

# EXEC_ELEMENT
if(EXISTS ${CMAKE_INSTALL_PREFIX}/lib OR CMAKE_INSTALL_LIBDIR STREQUAL "lib")
    list(APPEND EXEC_ELEMENT $ORIGIN/../lib)
endif()
if(EXISTS ${CMAKE_INSTALL_PREFIX}/lib64 OR CMAKE_INSTALL_LIBDIR STREQUAL "lib64")
    list(APPEND EXEC_ELEMENT $ORIGIN/../lib64)
endif()

#-----------------------------------------------------------------------
# RPATH functions
#-----------------------------------------------------------------------

# Constructs an RPATH (RUNPATH) value from a list of component paths.
function(_define_rpath VAR)
    set(_rpath ${ARGN})
    list(REMOVE_DUPLICATES _rpath)
    list(JOIN _rpath ":" _rpath)
    set(${VAR} ${_rpath} PARENT_SCOPE)
endfunction()

# Constructs an RPATH (RUNPATH) value and sets the INSTALL_RPATH property
# of the specified target.
function(set_install_rpath TARGET)
    if(SET_RPATH)
        _define_rpath(_rpath ${ARGN})
        set_target_properties(${TARGET} PROPERTIES INSTALL_RPATH ${_rpath})
        # message(STATUS "${TARGET}::RPATH=\"${_rpath}\"")
    endif()
endfunction()
