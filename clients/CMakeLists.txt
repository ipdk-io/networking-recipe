# Build file for clients.
#
# Copyright 2022-2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

add_library(client_cert_options STATIC
    client_cert_options.cc
    client_cert_options.h
)

add_subdirectory(gnmi-ctl)
add_subdirectory(p4rt-ctl)
add_subdirectory(sgnmi_cli)
add_subdirectory(p4rt_perf_test)
