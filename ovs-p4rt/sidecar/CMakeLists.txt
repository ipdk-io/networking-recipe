# CMake build file for ovs-p4rt/sidecar
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

add_library(ovs_sidecar_o OBJECT
    ovsp4rt.cc
    ovsp4rt_session.cc
    ovsp4rt_session.h
    ovsp4rt_credentials.cc
    ovsp4rt_credentials.h
    ovsp4rt_utils.cc
    ovsp4rt_utils.h
)

if(DPDK_TARGET)
    add_subdirectory(dpdk)
elseif(ES2K_TARGET)
    add_subdirectory(es2k)
endif()

target_include_directories(ovs_sidecar_o PRIVATE
    ${OVS_INCLUDE_DIR}
    ${STRATUM_SOURCE_DIR}
    ${PB_OUT_DIR}
)

add_dependencies(ovs_sidecar_o
    stratum_proto
    p4runtime_proto
)

target_link_libraries(ovs_sidecar_o PUBLIC
    stratum_static
    p4_role_config_proto
)

