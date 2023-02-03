# Downloads and builds dependencies
#
# Copyright 2022-2023 Intel Corporation
# SPDX-License-Identifier: Apache-2.0
#

cmake_minimum_required(VERSION 3.15)

project(dependencies VERSION 1 LANGUAGES C CXX)

include(ExternalProject)
include(CMakePrintHelpers)
include(GNUInstallDirs)

# Define git repository tags and urls.
include(deps.cmake)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.24)
  message(STATUS "CMake version is ${CMAKE_VERSION}")
  message(STATUS "Protobuf build is broken in CMake 3.24.x and 3.25.x")
  message(STATUS "Versions 3.16 through 3.22 are recommended")
  message(FATAL_ERROR "Invalid CMake version")
endif()

option(ON_DEMAND    "Build targets on demand" OFF)
option(USE_SUDO     "Use sudo when installing" OFF)
option(USE_LDCONFIG "Use ldconfig when installing" OFF)

if(USE_SUDO)
  set(SUDO_CMD "sudo" "-E")
  if(USE_LDCONFIG)
      set(LDCONFIG_CMD COMMAND sudo ldconfig)
  endif()
endif()

# CMAKE_PREFIX_PATH specifies the directories to be searched by
# find_file(), find_library(), find_path(), and find_program().
#
# We add our own install prefix to this list so the libraries we build
# will take precedence over versions installed elsewhere on the system.
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_INSTALL_PREFIX})

##########
# ABSEIL #
##########

# Recommended for CMake 3.8 and up
set(ABSL_CXX_PROPAGATE_CXX_STD ON)

ExternalProject_Add(abseil-cpp
  GIT_REPOSITORY  ${ABSEIL_GIT_URL}
  GIT_TAG         ${ABSEIL_GIT_TAG}
  GIT_PROGRESS    ON

  SOURCE_DIR
    ${CMAKE_SOURCE_DIR}/abseil-cpp
  CMAKE_ARGS
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_POSITION_INDEPENDENT_CODE=on
    -DCMAKE_INSTALL_RPATH=$ORIGIN
    -DBUILD_SHARED_LIBS=on
    -DBUILD_TESTING=off
  INSTALL_COMMAND
    ${SUDO_CMD} ${CMAKE_MAKE_PROGRAM} install
    ${LDCONFIG_CMD}
)
if(ON_DEMAND)
  set_target_properties(abseil-cpp PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif()

########
# ZLIB #
########

ExternalProject_Add(zlib
  GIT_REPOSITORY  ${ZLIB_GIT_URL}
  GIT_TAG         ${ZLIB_GIT_FLAG}
  GIT_PROGRESS    ON

  SOURCE_DIR
    ${CMAKE_SOURCE_DIR}/zlib
  CMAKE_ARGS
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
  INSTALL_COMMAND
    ${SUDO_CMD} ${CMAKE_MAKE_PROGRAM} install
    ${LDCONFIG_CMD}
)
if(ON_DEMAND)
  set_target_properties(zlib PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif()

############
# PROTOBUF #
############

ExternalProject_Add(protobuf
  GIT_REPOSITORY  ${PROTOBUF_GIT_URL}
  GIT_TAG         ${PROTOBUF_GIT_TAG}
  GIT_PROGRESS    ON

  SOURCE_DIR
    ${CMAKE_SOURCE_DIR}/protobuf
  CMAKE_ARGS
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_POSITION_INDEPENDENT_CODE=on
    -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
    -DBUILD_SHARED_LIBS=on
    -Dprotobuf_BUILD_TESTS=off
  SOURCE_SUBDIR
    cmake
  INSTALL_COMMAND
    ${SUDO_CMD} ${CMAKE_MAKE_PROGRAM} install
    ${LDCONFIG_CMD}
  DEPENDS
    zlib
)
if(ON_DEMAND)
  set_target_properties(protobuf PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif()

########
# GRPC #
########

# This is a brute-force solution, but CMAKE_INSTALL_LIBDIR isn't
# doing the trick.
set(GRPC_INSTALL_RPATH $ORIGIN/../lib64:$ORIGIN/../lib)
configure_file(grpc.patch.in ${CMAKE_SOURCE_DIR}/grpc.patch @ONLY)

if(CMAKE_CROSSCOMPILING)
  # If we're cross-compiling for the target system, don't build the
  # gRPC code generation executables.
  set(gRPC_BUILD_CODEGEN_OPTION -DgRPC_BUILD_CODEGEN=off)
endif()

ExternalProject_Add(grpc
  GIT_REPOSITORY  ${GRPC_GIT_URL}
  GIT_TAG         ${GRPC_GIT_TAG}
  GIT_PROGRESS    ON

  PATCH_COMMAND
    # Set RUNPATH in gRPC executables.
    patch -i ${CMAKE_SOURCE_DIR}/grpc.patch -p1
  SOURCE_DIR
    ${CMAKE_SOURCE_DIR}/grpc
  CMAKE_ARGS
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_INSTALL_RPATH=$ORIGIN
    -DCMAKE_POSITION_INDEPENDENT_CODE=on
    -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
    -DBUILD_SHARED_LIBS=on
    -DgRPC_ABSL_PROVIDER=package
    -DgRPC_PROTOBUF_PROVIDER=package
    # gRPC builds BoringSSL, which is incompatible with libpython.
    # We use whatever version of OpenSSL is installed instead.
    -DgRPC_SSL_PROVIDER=package
    -DgRPC_ZLIB_PROVIDER=package
    -DgRPC_BUILD_GRPC_CSHARP_PLUGIN=off
    -DgRPC_BUILD_GRPC_NODE_PLUGIN=off
    -DgRPC_BUILD_GRPC_OBJECTIVE_C_PLUGIN=off
    -DgRPC_BUILD_GRPC_PHP_PLUGIN=off
    -DgRPC_BUILD_GRPC_RUBY_PLUGIN=off
    -DgRPC_BUILD_TESTS=off
    -DgRPC_INSTALL=on
    ${gRPC_BUILD_CODEGEN_OPTION}
  INSTALL_COMMAND
    ${SUDO_CMD} ${CMAKE_MAKE_PROGRAM} install
    ${LDCONFIG_CMD}
  DEPENDS
    abseil-cpp
    protobuf
    zlib
)
if(ON_DEMAND)
  set_target_properties(grpc PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif()

########
# CCTZ #
########

ExternalProject_Add(cctz
  GIT_REPOSITORY  ${CCTZ_GIT_URL}
  GIT_TAG         ${CCTZ_GIT_TAG}
  GIT_PROGRESS    ON

  SOURCE_DIR
    ${CMAKE_SOURCE_DIR}/cctz
  CMAKE_ARGS
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_POSITION_INDEPENDENT_CODE=on
    -DBUILD_TESTING=off
  INSTALL_COMMAND
    ${SUDO_CMD} ${CMAKE_MAKE_PROGRAM} install
    ${LDCONFIG_CMD}
)
if(ON_DEMAND)
  set_target_properties(cctz PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif()

##########
# GFLAGS #
##########

ExternalProject_Add(gflags
  GIT_REPOSITORY  ${GFLAGS_GIT_URL}
  GIT_TAG         ${GFLAGS_GIT_TAG}
  GIT_PROGRESS    ON

  SOURCE_DIR
    ${CMAKE_SOURCE_DIR}/gflags
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DBUILD_SHARED_LIBS=on
  INSTALL_COMMAND
    ${SUDO_CMD} ${CMAKE_MAKE_PROGRAM} install
    ${LDCONFIG_CMD}
)
if(ON_DEMAND)
  set_target_properties(gflags PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif()

########
# GLOG #
########

ExternalProject_Add(glog
  GIT_REPOSITORY  ${GLOG_GIT_URL}
  GIT_TAG         ${GLOG_GIT_TAG}
  GIT_PROGRESS    ON

  SOURCE_DIR
    ${CMAKE_SOURCE_DIR}/glog
  CMAKE_ARGS
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_INSTALL_RPATH=$ORIGIN
    -Dgflags_DIR:PATH=${CMAKE_INSTALL_PREFIX}/lib/cmake/gflags
    -DWITH_GTEST=OFF
  INSTALL_COMMAND
    ${SUDO_CMD} ${CMAKE_MAKE_PROGRAM} install
    ${LDCONFIG_CMD}
  DEPENDS
    gflags
)
if(ON_DEMAND)
  set_target_properties(glog PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif()

#############
# GMOCK_GBL #
#############

ExternalProject_Add(gmock-gbl
  GIT_REPOSITORY  ${GMOCK_GBL_GIT_URL}
  GIT_TAG         ${GMOCK_GIT_GIT_TAG}
  GIT_PROGRESS    ON

  SOURCE_DIR
    ${CMAKE_SOURCE_DIR}/gmock-gbl
  CMAKE_ARGS
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
)
if(ON_DEMAND)
  set_target_properties(gmock-gbl PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif()

#########
# GTEST #
#########

ExternalProject_Add(gtest
  GIT_REPOSITORY  ${GTEST_GIT_URL}
  GIT_TAG         ${GTEST_GIT_TAG}
  GIT_PROGRESS    ON

  SOURCE_DIR
    ${CMAKE_SOURCE_DIR}/gtest
  CMAKE_ARGS
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_INSTALL_RPATH=$ORIGIN
    -DBUILD_SHARED_LIBS=on
  INSTALL_COMMAND
    ${SUDO_CMD} ${CMAKE_MAKE_PROGRAM} install
    ${LDCONFIG_CMD}
)
if(ON_DEMAND)
  set_target_properties(gtest PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif()

########
# JSON #
########

ExternalProject_Add(json
  GIT_REPOSITORY  ${JSON_GIT_URL}
  GIT_TAG         ${JSON_GIT_TAG}
  GIT_PROGRESS    ON

  SOURCE_DIR
    ${CMAKE_SOURCE_DIR}/json
  CMAKE_ARGS
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DJSON_BuildTests=off
  INSTALL_COMMAND
    ${SUDO_CMD} ${CMAKE_MAKE_PROGRAM} install
    ${LDCONFIG_CMD}
)
if(ON_DEMAND)
  set_target_properties(json PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif()
