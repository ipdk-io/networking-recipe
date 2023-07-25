# CMake build file for script files
#
# Copyright 2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

add_subdirectory(common)

if(DPDK_TARGET)
    add_subdirectory(dpdk)
elseif(ES2K_TARGET)
    add_subdirectory(es2k)
endif()
