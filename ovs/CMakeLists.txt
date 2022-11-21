# CMake build file for Open vSwitch
#
# Copyright 2022 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
cmake_minimum_required(VERSION 3.5)

project(ovs LANGUAGES C)

include(ExternalProject)

set(OVS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ovs" CACHE PATH
    "OVS source directory")

option(P4OVS "Build OVS with P4Runtime support" OFF)

if(P4OVS)
    set(P4OVS_OPTION --with-p4ovs)
endif()

ExternalProject_Add(ovs
  SOURCE_DIR
    ${OVS_SOURCE_DIR}
  CONFIGURE_COMMAND
    cd ${OVS_SOURCE_DIR} && ./boot.sh
  COMMAND
    ${OVS_SOURCE_DIR}/configure
    --prefix=${CMAKE_INSTALL_PREFIX}
    ${P4OVS_OPTION}
    --disable-ssl
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
