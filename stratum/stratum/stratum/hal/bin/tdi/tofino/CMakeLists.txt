# Build file for //stratum/hal/bin/tdi/tofino
#
# Copyright 2022-2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

target_sources(stratum_main_o PRIVATE
    tofino_main.cc
)

install(
    FILES
        tofino_skip_p4_no_bsp.conf
        tofino_skip_p4.conf
    DESTINATION
        ${CMAKE_INSTALL_DATAROOTDIR}/stratum
)
