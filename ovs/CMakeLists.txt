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

option(P4OVS "Build OVS with P4Runtime support" OFF)
option(WITH_RUNDIRS "Build with OVS runtime directories" OFF)

####################
# Path definitions #
####################

set(OVS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ovs" CACHE PATH
    "OVS source directory")

#####################
# OVS configuration #
#####################

if(P4OVS)
    set(P4OVS_OPTION --with-p4ovs)
endif()

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(NDEBUG_OPTION --enable-ndebug)
endif()

set(SSL_OPTION --disable-ssl)

#######################
# Runtime directories #
#######################

set(OVS_RUNTIME_DIR "" CACHE PATH "OVS runtime root directory")

set(WITH_DBDIR "${OVS_RUNTIME_DIR}/etc/openvswitch" CACHE PATH
    "Directory used for OVS conf.db")

set(WITH_LOGDIR
    "${OVS_RUNTIME_DIR}/${CMAKE_INSTALL_LOCALSTATEDIR}/log/openvswitch"
    CACHE PATH "Directory used for OVS logfiles")

set(WITH_RUNDIR
    "${OVS_RUNTIME_DIR}/${CMAKE_INSTALL_LOCALSTATEDIR}/run/openswitch"
    CACHE PATH "Directory used for OVS pidfiles")

if(WITH_RUNDIRS)
    set(RUNDIR_OPTIONS
        --with-dbdir=${WITH_DBDIR}
        --with-logdir=${WITH_LOGDIR}
        --with-rundir=${WITH_RUNDIR}
    )
endif()

#####################
# Cross-compilation #
#####################

function(print_crosscompile_variables)
    cmake_print_variables(CROSS_COMPILE_OPTIONS)
    cmake_print_variables(CC_VARIABLE)
    cmake_print_variables(LD_VARIABLE)
    cmake_print_variables(CFLAGS_VARIABLE)
    cmake_print_variables(LDFLAGS_VARIABLE)
endfunction()

# Derives the OVS configuration from the production toolchain file and
# environment variables.
macro(apply_crosscompile_toolchain TOOLFILE)
    message(STATUS "Applying ${TOOLFILE} options")

    # Convert the CONFIGURE_FLAGS environment variable to a cmake list.
    string(REPLACE " " ";" CROSS_COMPILE_OPTIONS $ENV{CONFIGURE_FLAGS})

    get_filename_component(c_compiler ${CMAKE_C_COMPILER} NAME)
    set(CC_VARIABLE "CC=${c_compiler} --sysroot=${CMAKE_SYSROOT}")

    get_filename_component(linker ${CMAKE_LINKER} NAME)
    set(LD_VARIABLE "LD=${linker} --sysroot=${CMAKE_SYSROOT}")

    string(JOIN " " security_flags
        -fstack-protector-strong
        -D_FORTIFY_SOURCE=2
        -Wformat
        -Wformat-security
        -Werror=format-security
    )
    set(CFLAGS_VARIABLE "CFLAGS=${security_flags} $ENV{CFLAGS}")

    set(LDFLAGS_VARIABLE "LDFLAGS=$ENV{LDFLAGS}")
endmacro(apply_crosscompile_toolchain)

# Derives the OVS configuration from the experimental toolchain file.
macro(apply_aarch64_toolchain TOOLFILE)
    message(STATUS "Applying ${TOOLFILE} options")

    # Command-line options
    list(APPEND CROSS_COMPILE_OPTIONS --build=${CROSS_COMPILE_BUILD})
    list(APPEND CROSS_COMPILE_OPTIONS --host=${CROSS_COMPILE_HOST})
    list(APPEND CROSS_COMPILE_OPTIONS --target=${CROSS_COMPILE_TARGET})
    list(APPEND CROSS_COMPILE_OPTIONS --with-libtool-sysroot=${CMAKE_SYSROOT})

    # Variable definitions
    get_filename_component(c_compiler ${CMAKE_C_COMPILER} NAME)
    set(CC_VARIABLE "CC=${c_compiler} --sysroot=${CMAKE_SYSROOT}")

    get_filename_component(linker ${CMAKE_LINKER} NAME)
    set(LD_VARIABLE "LD=${linker} --sysroot=${CMAKE_SYSROOT}")

    string(TOUPPER ${CMAKE_BUILD_TYPE} config)

    if(NOT config STREQUAL "")
        set(c_flags ${CMAKE_C_FLAGS_${config}})
        set(linker_flags ${CMAKE_EXE_LINKER_FLAGS_${config}})
    else()
        set(c_flags ${CMAKE_C_FLAGS)
        set(linker_flags ${CMAKE_EXE_LINKER_FLAGS})
    endif()

    set(CFLAGS_VARIABLE "CFLAGS=${c_flags}")
    set(LDFLAGS_VARIABLE "LDFLAGS=${linker_flags}")
endmacro(apply_aarch64_toolchain)

if(CMAKE_TOOLCHAIN_FILE AND NOT CMAKE_TOOLCHAIN_FILE STREQUAL "")
    get_filename_component(toolfile ${CMAKE_TOOLCHAIN_FILE} NAME)
    if(toolfile STREQUAL "aarch64-toolchain.cmake")
        apply_aarch64_toolchain(${toolfile})
    elseif(toolfile STREQUAL "CrossCompile.cmake")
        apply_crosscompile_toolchain(${toolfile})
    endif()
   # print_crosscompile_variables()
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
    ${CROSS_COMPILE_OPTIONS}
    # OVS options
    ${RUNDIR_OPTIONS}
    ${P4OVS_OPTION}
    ${NDEBUG_OPTION}
    ${SSL_OPTION}
    # Cross-compile variables
    "${CC_VARIABLE}"
    "${CFLAGS_VARIABLE}"
    "${LD_VARIABLE}"
    "${LDFLAGS_VARIABLE}"
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
