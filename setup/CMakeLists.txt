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

option(DOWNLOAD     "Download repositories" TRUE)
option(FORCE_PATCH  "Specify -f when patching" FALSE)
option(ON_DEMAND    "Build targets on demand" FALSE)
option(USE_LDCONFIG "Use ldconfig when installing" FALSE)

# Note: USE_SUDO should be DISABLED by default.
# This is in keeping with the following security principles:
# - Principle of Least Privilege
# - Secure By Default
option(USE_SUDO     "Use sudo when installing" FALSE)

if(USE_SUDO)
  set(SUDO_CMD "sudo" "-E")
  if(USE_LDCONFIG)
      set(LDCONFIG_CMD COMMAND sudo ldconfig)
  endif()
endif()

if(FORCE_PATCH)
  set(FORCE_OPTION "-f")
endif()

if(CMAKE_CROSSCOMPILING)
  list(APPEND CMAKE_FIND_ROOT_PATH ${CMAKE_INSTALL_PREFIX})
else()
  list(APPEND CMAKE_PREFIX_PATH ${CMAKE_INSTALL_PREFIX})
endif()

##########################
# Set CMAKE_CXX_STANDARD #
#########################
if(CXX_STANDARD)
  set(version_ABSL_CXX_STANDARD -DABSL_CXX_STANDARD=${CXX_STANDARD})
  set(version_CMAKE_CXX_STANDARD -DCMAKE_CXX_STANDARD=${CXX_STANDARD})
endif()

###################
# GetDownloadSpec #
###################

# Generates the ExternalProject_Add() parameters needed to Download
# a git repository and returns them in the VAR parameter. Returns an
# empty string if the DOWNLOAD option is false.
function(GetDownloadSpec VAR _url _tag)
  if(DOWNLOAD)
    set(_spec
      GIT_REPOSITORY  ${_url}
      GIT_TAG         ${_tag}
      GIT_PROGRESS    ON
    )
  else()
    set(_spec "")
  endif()
  set(${VAR} ${_spec} PARENT_SCOPE)
endfunction(GetDownloadSpec)

##########
# ABSEIL #
##########

# Recommended for CMake 3.8 and up
set(ABSL_CXX_PROPAGATE_CXX_STD ON)

GetDownloadSpec(DOWNLOAD_ABSL ${ABSEIL_GIT_URL} ${ABSEIL_GIT_TAG})

ExternalProject_Add(abseil-cpp
  ${DOWNLOAD_ABSL}

  SOURCE_DIR
    ${CMAKE_SOURCE_DIR}/abseil-cpp
  CMAKE_ARGS
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
    -DCMAKE_FIND_ROOT_PATH=${CMAKE_FIND_ROOT_PATH}
    ${version_ABSL_CXX_STANDARD}
    ${version_CMAKE_CXX_STANDARD}
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

GetDownloadSpec(DOWNLOAD_ZLIB ${ZLIB_GIT_URL} ${ZLIB_GIT_TAG})

ExternalProject_Add(zlib
  ${DOWNLOAD_ZLIB}

  SOURCE_DIR
    ${CMAKE_SOURCE_DIR}/zlib
  CMAKE_ARGS
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
    -DCMAKE_FIND_ROOT_PATH=${CMAKE_FIND_ROOT_PATH}
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

GetDownloadSpec(DOWNLOAD_PROTOBUF ${PROTOBUF_GIT_URL} ${PROTOBUF_GIT_TAG})

ExternalProject_Add(protobuf
  ${DOWNLOAD_PROTOBUF}

  SOURCE_DIR
    ${CMAKE_SOURCE_DIR}/protobuf
  CMAKE_ARGS
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_POSITION_INDEPENDENT_CODE=on
    -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
    -DCMAKE_FIND_ROOT_PATH=${CMAKE_FIND_ROOT_PATH}
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

# Patch the gRPC build script to set the RUNPATH of the installed
# Protobuf compiler plugins to the relative paths of the library
# directories.
set(GRPC_INSTALL_RPATH $ORIGIN/../lib64:$ORIGIN/../lib)
configure_file(grpc.patch.in ${CMAKE_SOURCE_DIR}/grpc.patch @ONLY)

if(CMAKE_CROSSCOMPILING)
  # If we're cross-compiling for the target system, don't build the
  # gRPC code generation executables.
  set(gRPC_BUILD_CODEGEN_OPTION -DgRPC_BUILD_CODEGEN=off)
endif()

GetDownloadSpec(DOWNLOAD_GRPC ${GRPC_GIT_URL} ${GRPC_GIT_TAG})

ExternalProject_Add(grpc
  ${DOWNLOAD_GRPC}

  PATCH_COMMAND
    # Set RUNPATH in gRPC executables.
    patch -i ${CMAKE_SOURCE_DIR}/grpc.patch -p1 ${FORCE_OPTION}
  SOURCE_DIR
    ${CMAKE_SOURCE_DIR}/grpc
  CMAKE_ARGS
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_INSTALL_RPATH=$ORIGIN
    -DCMAKE_POSITION_INDEPENDENT_CODE=on
    -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
    -DCMAKE_FIND_ROOT_PATH=${CMAKE_FIND_ROOT_PATH}
    ${version_CMAKE_CXX_STANDARD}
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

GetDownloadSpec(DOWNLOAD_CCTZ ${CCTZ_GIT_URL} ${CCTZ_GIT_TAG})

ExternalProject_Add(cctz
  ${DOWNLOAD_CCTZ}

  SOURCE_DIR
    ${CMAKE_SOURCE_DIR}/cctz
  CMAKE_ARGS
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
    -DCMAKE_FIND_ROOT_PATH=${CMAKE_FIND_ROOT_PATH}
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

GetDownloadSpec(DOWNLOAD_GFLAGS ${GFLAGS_GIT_URL} ${GFLAGS_GIT_TAG})

ExternalProject_Add(gflags
  ${DOWNLOAD_GFLAGS}

  SOURCE_DIR
    ${CMAKE_SOURCE_DIR}/gflags
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
    -DCMAKE_FIND_ROOT_PATH=${CMAKE_FIND_ROOT_PATH}
    -DBUILD_SHARED_LIBS=on
    -DBUILD_TESTING=off
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

GetDownloadSpec(DOWNLOAD_GLOG ${GLOG_GIT_URL} ${GLOG_GIT_TAG})

ExternalProject_Add(glog
  ${DOWNLOAD_GLOG}

  SOURCE_DIR
    ${CMAKE_SOURCE_DIR}/glog
  CMAKE_ARGS
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
    -DCMAKE_FIND_ROOT_PATH=${CMAKE_FIND_ROOT_PATH}
    -DCMAKE_INSTALL_RPATH=$ORIGIN
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

GetDownloadSpec(DOWNLOAD_GMOCK_GBL ${GMOCK_GBL_GIT_URL} ${GMOCK_GBL_GIT_TAG})

ExternalProject_Add(gmock-gbl
  ${DOWNLOAD_GMOCK_GBL}

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

GetDownloadSpec(DOWNLOAD_GTEST ${GTEST_GIT_URL} ${GTEST_GIT_TAG})

ExternalProject_Add(gtest
  ${DOWNLOAD_GTEST}

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

GetDownloadSpec(DOWNLOAD_JSON ${JSON_GIT_URL} ${JSON_GIT_TAG})

ExternalProject_Add(json
  ${DOWNLOAD_JSON}

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
