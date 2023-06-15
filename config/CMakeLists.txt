# Installs networking recipe configuration files
#
# Copyright 2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

set(NW_RECIPE_SCRIPT_DIR ${NW_RECIPE_SOURCE_DIR}/scripts)

if(DPDK_TARGET)
    install(
        FILES
            ${NW_RECIPE_SCRIPT_DIR}/dpdk/set_hugepages.sh
            ${NW_RECIPE_SCRIPT_DIR}/dpdk/setup_env.sh
	    ${NW_RECIPE_SCRIPT_DIR}/dpdk/copy_config_files.sh
        DESTINATION
            ${NW_RECIPE_INSTALL_DIR}/sbin/dpdk/scripts
    )
elseif(ES2K_TARGET)
    install(
        FILES
            ${NW_RECIPE_SCRIPT_DIR}/es2k/set_hugepages.sh
            ${NW_RECIPE_SCRIPT_DIR}/es2k/setup_env.sh
            ${NW_RECIPE_SCRIPT_DIR}/es2k/copy_config_files.sh
        DESTINATION
            ${NW_RECIPE_INSTALL_DIR}/sbin/es2k/scripts
    )
endif()

