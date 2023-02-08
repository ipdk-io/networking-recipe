# CMake build file for Open vSwitch
#
# Copyright 2022 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
cmake_minimum_required(VERSION 3.5)

project(ovs LANGUAGES C)

include(CMakePrintHelpers)
include(ExternalProject)
include(GNUInstallDirs)

#############################
# Symbolic path definitions #
#############################

set(OVS_RUNTIME_DIR "" CACHE PATH "OVS runtime root directory")

set(OVS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ovs" CACHE PATH
    "OVS source directory")

# Runtime directories
if(NOT("${OVS_RUNTIME_DIR}" STREQUAL ""))
  set(WITH_DBDIR "${OVS_RUNTIME_DIR}/etc/openvswitch" CACHE PATH
      "Directory used for OVS conf.db")
  set(WITH_LOGDIR
      "${OVS_RUNTIME_DIR}/${CMAKE_INSTALL_LOCALSTATEDIR}/log/openvswitch"
      CACHE PATH "Directory used for OVS logfiles")
  set(WITH_RUNDIR
      "${OVS_RUNTIME_DIR}/${CMAKE_INSTALL_LOCALSTATEDIR}/run/openswitch"
      CACHE PATH "Directory used for OVS pidfiles")
endif()

#########################
# Configuration options #
#########################

option(P4OVS "Build OVS with P4Runtime support" OFF)

if(P4OVS)
    set(P4OVS_OPTION --with-p4ovs)
endif()

if(NOT ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug"))
    set(NDEBUG_OPTION --enable-ndebug)
endif()

set(SSL_OPTION --disable-ssl)

if(CMAKE_CROSSCOMPILING)
  set(TARGET_OPTION --target=aarch64-intel-linux-gnu)
  set(HOST_OPTION --host=aarch64-intel-linux-gnu)
  set(BUILD_OPTION --build=x86_64-build_pc-linux-gnu)
  set(WITH_LIBTOOL_SYSROOT --with-libtool-sysroot=${CMAKE_SYSROOT}})
endif()

######################
# Build Open vSwitch #
######################

ExternalProject_Add(ovs
  SOURCE_DIR
    ${OVS_SOURCE_DIR}
  CONFIGURE_COMMAND
    cd ${OVS_SOURCE_DIR} && ./boot.sh
  COMMAND
    ${OVS_SOURCE_DIR}/configure
    --prefix=${CMAKE_INSTALL_PREFIX}
    ${TARGET_OPTION}
    ${HOST_OPTION}
    ${BUILD_OPTION}
    ${WITH_LIBTOOL_SYSROOT}
    ${WITH_DBDIR}
    ${WITH_LOGDIR}
    ${WITH_RUNDIR}
    ${P4OVS_OPTION}
    ${NDEBUG_OPTION}
    ${SSL_OPTION}
  BINARY_DIR
    ${CMAKE_CURRENT_BINARY_DIR}
  BUILD_COMMAND
    # Using $(MAKE) causes CMake not to disable the jobserver, allowing us
    # to do parallel builds. The number of jobs defaults to the -jN value
    # specified on the CMake command line IN ADDITION TO any CMake jobs.
    # This does not apply in our case because we've made p4ovs dependent
    # on ovs, so CMake won't continue until the ovs build completes.
    $(MAKE)
  INSTALL_COMMAND
    ${CMAKE_MAKE_PROGRAM} install
)
