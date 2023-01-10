# Copyright (c) 2022 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.15)

project(dependencies VERSION 1 LANGUAGES C CXX)

include(ExternalProject)
include(CMakePrintHelpers)
include(GNUInstallDirs)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL 3.24)
  message(STATUS "CMake version is ${CMAKE_VERSION}")
  message(STATUS "Protobuf build is broken in CMake 3.24.x and 3.25.x")
  message(STATUS "Versions 3.16 through 3.22 are recommended")
  message(FATAL_ERROR "Invalid CMake version")
endif()

option(ON_DEMAND    "Build targets on demand" OFF)
option(USE_SUDO     "Use sudo when installing" OFF)
option(USE_LDCONFIG "Use ldconfig when installing" ON)

if(USE_SUDO)
  set(SUDO_CMD "sudo" "-E")
  if(USE_LDCONFIG)
      set(LDCONFIG_CMD COMMAND sudo ldconfig)
  endif()
endif()

list(APPEND CMAKE_PREFIX_PATH ${CMAKE_INSTALL_PREFIX})

##########
# ABSEIL #
##########

set(ABSEIL_GIT_TAG 273292d1cfc0a94a65082ee350509af1d113344d) #20220623.0

# Recommended for CMake 3.8 and up
set(ABSL_CXX_PROPAGATE_CXX_STD ON)

ExternalProject_Add(abseil-cpp
  GIT_REPOSITORY  https://github.com/abseil/abseil-cpp.git
  GIT_TAG         ${ABSEIL_GIT_TAG}
  GIT_PROGRESS    ON

  SOURCE_DIR
    ${CMAKE_SOURCE_DIR}/abseil-cpp
  CMAKE_ARGS
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

set(ZLIB_GIT_TAG 04f42ceca40f73e2978b50e93806c2a18c1281fc) # v1.2.13

ExternalProject_Add(zlib
  GIT_REPOSITORY  https://github.com/madler/zlib
  GIT_TAG         ${ZLIB_GIT_FLAG}
  GIT_PROGRESS    ON

  SOURCE_DIR
    ${CMAKE_SOURCE_DIR}/zlib
  CMAKE_ARGS
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

set(PROTOBUF_GIT_TAG fe271ab76f2ad2b2b28c10443865d2af21e27e0e) # v3.20.3

ExternalProject_Add(protobuf
  GIT_REPOSITORY  https://github.com/google/protobuf.git
  GIT_TAG         ${PROTOBUF_GIT_TAG}
  GIT_PROGRESS    ON

  SOURCE_DIR
    ${CMAKE_SOURCE_DIR}/protobuf
  CMAKE_ARGS
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

configure_file(grpc.patch.in ${CMAKE_SOURCE_DIR}/grpc.patch @ONLY)

set(GRPC_GIT_TAG 90ccf24d22b6fc909a1021ebd89fd8c838467d26) # v1.50.0

ExternalProject_Add(grpc
  GIT_REPOSITORY  https://github.com/google/grpc.git
  GIT_TAG         ${GRPC_GIT_TAG}
  GIT_PROGRESS    ON

  PATCH_COMMAND
    # Set RUNPATH in gRPC executables.
    patch -i ${CMAKE_SOURCE_DIR}/grpc.patch -p1
  SOURCE_DIR
    ${CMAKE_SOURCE_DIR}/grpc
  CMAKE_ARGS
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

set(CCTTZ_GIT_TAG 02918d62329ef440935862719829d061a5f4beba)

ExternalProject_Add(cctz
  GIT_REPOSITORY  https://github.com/google/cctz.git
  GIT_TAG         ${CCTZ_GIT_TAG}
  GIT_PROGRESS    ON

  SOURCE_DIR
    ${CMAKE_SOURCE_DIR}/cctz
  CMAKE_ARGS
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

set(GFLAGS_GIT_TAG 827c769e5fc98e0f2a34c47cef953cc6328abced)

ExternalProject_Add(gflags
  GIT_REPOSITORY  https://github.com/gflags/gflags.git
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

set(GLOG_GIT_TAG a8e0007e96ff96145022c488e367da10f835c75d) #v0.6.0-rc1

ExternalProject_Add(glog
  GIT_REPOSITORY  https://github.com/google/glog.git
  GIT_TAG         ${GLOG_GIT_TAG}
  GIT_PROGRESS    ON

  SOURCE_DIR
    ${CMAKE_SOURCE_DIR}/glog
  CMAKE_ARGS
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
  GIT_REPOSITORY  https://github.com/apriorit/gmock-global-sample.git
  GIT_PROGRESS    ON

  SOURCE_DIR
    ${CMAKE_SOURCE_DIR}/gmock-gbl
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
)
if(ON_DEMAND)
  set_target_properties(gmock-gbl PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif()

#########
# GTEST #
#########

set(GTEST_GIT_TAG release-1.11.0)

ExternalProject_Add(gtest
  GIT_REPOSITORY  https://github.com/google/googletest.git
  GIT_TAG         ${GTEST_GIT_TAG}
  GIT_PROGRESS    ON

  SOURCE_DIR
    ${CMAKE_SOURCE_DIR}/gtest
  CMAKE_ARGS
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

set(JSON_GIT_TAG 760304635dc74a5bf77903ad92446a6febb85acf)

ExternalProject_Add(json
  GIT_REPOSITORY  https://github.com/nlohmann/json.git
  GIT_TAG         ${JSON_GIT_FLAG}
  GIT_PROGRESS    ON

  SOURCE_DIR
    ${CMAKE_SOURCE_DIR}/json
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DJSON_BuildTests=off
  INSTALL_COMMAND
    ${SUDO_CMD} ${CMAKE_MAKE_PROGRAM} install
    ${LDCONFIG_CMD}
)
if(ON_DEMAND)
  set_target_properties(json PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif()
