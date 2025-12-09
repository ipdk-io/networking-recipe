# Build file for //stratum/hal/bin/tdi/es2k
#
# Copyright 2022-2023,2025 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

target_sources(stratum_main_o PRIVATE
    es2k_main.cc
)

install(
    FILES
        es2k_port_config.pb.txt
        es2k_skip_p4.conf
        ${STRATUM_SOURCE_DIR}/tools/tls/ca.conf
        ${STRATUM_SOURCE_DIR}/tools/tls/grpc-client.conf
        ${STRATUM_SOURCE_DIR}/tools/tls/grpc-server.conf
    DESTINATION
        ${CMAKE_INSTALL_DATAROOTDIR}/stratum/es2k
)

install(
    PROGRAMS
        ${STRATUM_SOURCE_DIR}/tools/tls/generate-certs.sh
    TYPE
        SBIN
)
