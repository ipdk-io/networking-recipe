# CMake build file for dpdk scripts
#
# Copyright 2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

set(TARGET_NAME "dpdk")
configure_file("../common/copy_config_files.sh.in"
               ${CMAKE_CURRENT_BINARY_DIR}/copy_config_files.sh @ONLY)
unset(TARGET_NAME)

# Sets execute permissions
install(
    PROGRAMS
        ${CMAKE_CURRENT_BINARY_DIR}/copy_config_files.sh
    TYPE
        SBIN
)
