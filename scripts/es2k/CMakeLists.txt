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

# Sets execute permissions
install(
    PROGRAMS
        set_hugepages.sh
        ${CMAKE_CURRENT_BINARY_DIR}/copy_config_files.sh
    TYPE
        SBIN
)

# Does not set execute permissions
install(FILES setup_env.sh TYPE SBIN)
