# CMake build file for installing common scripts
#
# Copyright 2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

# Sets execute permissions
install(
    PROGRAMS
        set_hugepages.sh
        extract_table_ids_from_p4info.py
    TYPE
        SBIN
)

# Does not set execute permissions
install(
    FILES 
        setup_env.sh
        os_ver_details.sh
    TYPE
        SBIN
)
