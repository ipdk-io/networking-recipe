# CMake build file for switchutils
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0

add_library(switchutils_o OBJECT
    switch_log.h
    switch_utils.h
)

target_include_directories(switchutils_o PRIVATE
    ${SDE_INSTALL_DIR}/include
)
