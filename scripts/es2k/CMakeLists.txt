# CMake build file for es2k scripts
#
# Copyright 2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

# Customize copy_config_files.sh script

set(TARGET_NAME "es2k")

configure_file("../common/copy_config_files.sh.in"
               ${CMAKE_CURRENT_BINARY_DIR}/copy_config_files.sh @ONLY)

unset(TARGET_NAME)

install(
    PROGRAMS
        ${CMAKE_CURRENT_BINARY_DIR}/copy_config_files.sh
    TYPE
        SBIN
)
