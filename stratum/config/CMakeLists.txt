# Installs stratum configuration files
#
# Copyright 2022-2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

set(STRATUM_TDI_BIN_DIR ${STRATUM_SOURCE_DIR}/stratum/hal/bin/tdi)
set(STRATUM_BFRT_BIN_DIR ${STRATUM_SOURCE_DIR}/stratum/hal/bin/barefoot)

if(DPDK_TARGET)
    install(
        FILES
            ${STRATUM_TDI_BIN_DIR}/dpdk/dpdk_port_config.pb.txt
            ${STRATUM_TDI_BIN_DIR}/dpdk/dpdk_skip_p4.conf
        DESTINATION
            ${CMAKE_INSTALL_DATAROOTDIR}/stratum/dpdk
    )
elseif(ES2K_TARGET)
    install(
        FILES
	    ${STRATUM_TDI_BIN_DIR}/es2k/es2k_port_config.pb.txt
            ${STRATUM_TDI_BIN_DIR}/es2k/es2k_skip_p4.conf
        DESTINATION
            ${CMAKE_INSTALL_DATAROOTDIR}/stratum/es2k
    )
elseif(TOFINO_TARGET)
    install(
        FILES
            ${STRATUM_BFRT_BIN_DIR}/tofino_skip_p4_no_bsp.conf
            ${STRATUM_BFRT_BIN_DIR}/tofino_skip_p4.conf
        DESTINATION
            ${CMAKE_INSTALL_DATAROOTDIR}/stratum
    )
endif()

