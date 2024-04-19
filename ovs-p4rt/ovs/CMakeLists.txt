# CMake build file for ovs-p4rt/ovs
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

include(CheckBinutilsAVX512)
include(CheckCCompilerFlag)
include(CheckLibraryExists)

check_library_exists(rt aio_write /usr/lib/x86_64-linux-gnu HAVE_POSIX_AIO)
check_c_compiler_flag(-mavx512f HAVE_AVX512F)
check_binutils_avx512(HAVE_LD_AVX512_GOOD)

# Generate config.h file and add it to the include path.
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
               "${CMAKE_CURRENT_BINARY_DIR}/config.h")

find_package(Unbound)
find_package(Unwind)

include(vswitchd.cmake)
include(testcontroller.cmake)
