# Copyright (c) 2022 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.15)

project(dependencies VERSION 1 LANGUAGES C CXX)

include(ExternalProject)
include(CMakePrintHelpers)

option(USE_SUDO       "Use sudo when installing" OFF)

if(USE_SUDO)
  set(SUDO_CMD "sudo" "-E")
endif()

list(APPEND CMAKE_PREFIX_PATH ${CMAKE_INSTALL_PREFIX})

##########
# ABSEIL #
##########

set(ABSEIL_GIT_TAG ec0d76f1d012cc1a4b3b08dfafcfc5237f5ba2c9)

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
    -DBUILD_SHARED_LIBS=on
    -DBUILD_TESTING=off
  INSTALL_COMMAND
    ${SUDO_CMD} ${CMAKE_MAKE_PROGRAM} install
    ${LDCONFIG_CMD}
)

############
# PROTOBUF #
############

set(PROTOBUF_GIT_TAG v3.18.1)

ExternalProject_Add(protobuf
  GIT_REPOSITORY  https://github.com/google/protobuf.git
  GIT_TAG         ${PROTOBUF_GIT_TAG}
  GIT_SHALLOW     ON    # depth=1
  GIT_PROGRESS    ON

  SOURCE_DIR
    ${CMAKE_SOURCE_DIR}/protobuf
  CMAKE_ARGS
    -S ${CMAKE_SOURCE_DIR}/protobuf/cmake
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_POSITION_INDEPENDENT_CODE=on
    -DBUILD_SHARED_LIBS=on
    -Dprotobuf_BUILD_TESTS=off
  INSTALL_COMMAND
    ${SUDO_CMD} ${CMAKE_MAKE_PROGRAM} install
)

########
# GRPC #
########

set(GRPC_GIT_TAG v1.42.0)

ExternalProject_Add(grpc
  GIT_REPOSITORY  https://github.com/google/grpc.git
  GIT_TAG         ${GRPC_GIT_TAG}
  GIT_SHALLOW     ON    # depth=1
  GIT_PROGRESS    ON

  SOURCE_DIR
    ${CMAKE_SOURCE_DIR}/grpc
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_POSITION_INDEPENDENT_CODE=on
    -DBUILD_SHARED_LIBS=on
    -DgRPC_SSL_PROVIDER=package
    -DgRPC_ABSL_PROVIDER=package
    -DgRPC_PROTOBUF_PROVIDER=package
    -DgRPC_BUILD_GRPC_CSHARP_PLUGIN=off
    -DgRPC_BUILD_GRPC_NODE_PLUGIN=off
    -DgRPC_BUILD_GRPC_OBJECTIVE_C_PLUGIN=off
    -DgRPC_BUILD_GRPC_PHP_PLUGIN=off
    -DgRPC_BUILD_GRPC_RUBY_PLUGIN=off
    -DgRPC_BUILD_TESTS=off
    -DgRPC_INSTALL=on
  INSTALL_COMMAND
    ${SUDO_CMD} ${CMAKE_MAKE_PROGRAM} install
  DEPENDS
    abseil-cpp
    protobuf
)

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
)

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
)

########
# GLOG #
########

set(GLOG_GIT_TAG 503e3dec8d1fe071376befc62119a837c26612a3)

ExternalProject_Add(glog
  GIT_REPOSITORY  https://github.com/google/glog.git
  GIT_TAG         ${GLOG_GIT_TAG}
  GIT_PROGRESS    ON

  SOURCE_DIR
    ${CMAKE_SOURCE_DIR}/glog
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -Dgflags_DIR:PATH=${CMAKE_INSTALL_PREFIX}/lib/cmake/gflags
    -DWITH_GTEST=OFF
  INSTALL_COMMAND
    ${SUDO_CMD} ${CMAKE_MAKE_PROGRAM} install
  DEPENDS
    gflags
)

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
    -DBUILD_SHARED_LIBS=on
  INSTALL_COMMAND
    ${SUDO_CMD} ${CMAKE_MAKE_PROGRAM} install
)

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
)
