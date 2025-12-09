# CMake build file for //stratum/lib/p4runtime
#
# Copyright 2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

###########################
# stratum_lib_p4runtime_o #
###########################

add_library(stratum_lib_p4runtime_o OBJECT
    sdn_controller_manager.cc
    sdn_controller_manager.h
)

if(ES2K_TARGET AND IPDK_ROLE_FIX)
    target_compile_definitions(stratum_lib_p4runtime_o PUBLIC IPDK_ROLE_FIX)
endif()

target_include_directories(stratum_lib_p4runtime_o PRIVATE
    ${STRATUM_INCLUDES}
    ${PB_OUT_DIR}
)

target_link_libraries(stratum_lib_p4runtime_o PRIVATE stratum_proto)

#######################
# p4runtime_session_o #
#######################

add_library(p4runtime_session_o OBJECT
    p4runtime_session.cc
    p4runtime_session.h
)

target_include_directories(p4runtime_session_o PUBLIC
    ${STRATUM_INCLUDES}
    ${PB_OUT_DIR}
)

target_link_libraries(p4runtime_session_o PUBLIC stratum_proto p4_role_config_proto)
