# CMake interface to Kernel Monitor
#
# Copyright 2022-2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

cmake_minimum_required(VERSION 3.15)

# Path to SAI source directory (for krnlmon)
set(SAI_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/SAI" CACHE PATH
    "Path to SAI source directory")

# Path to KRNLMON source directory (for infrap4d)
set(KRNLMON_SOURCE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/krnlmon" CACHE PATH
    "Path to kernel monitor source directory")

add_subdirectory(krnlmon)
