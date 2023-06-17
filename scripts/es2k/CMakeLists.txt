# CMake build file for es2k scripts
#
# Copyright 2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

# Sets execute permissions
install(
    PROGRAMS
        set_hugepages.sh
        copy_config_files.sh
    TYPE
        SBIN
)

# Does not set execute permissions
install(FILES setup_env.sh TYPE SBIN)
