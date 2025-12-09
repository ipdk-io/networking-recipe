# Build file for //stratum/hal/lib/yang
#
# Copyright 2022-2025 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

#############################
# stratum_yang_parse_tree_o #
#############################

add_library(stratum_yang_parse_tree_o OBJECT
    yang_parse_tree.cc
    yang_parse_tree.h
    yang_parse_tree_chassis.cc
    yang_parse_tree_component.cc
    yang_parse_tree_component.h
    yang_parse_tree_helpers.cc
    yang_parse_tree_helpers.h
    yang_parse_tree_interface.cc
    yang_parse_tree_interface.h
    yang_parse_tree_ipsec.cc
    yang_parse_tree_node.cc
    yang_parse_tree_optical.cc
    yang_parse_tree_paths.cc
    yang_parse_tree_paths.h
    yang_parse_tree_singleton.cc
    yang_parse_tree_system.cc
    yang_parse_tree_virtual_port.cc
)

if(NOT DPDK_TARGET)
    target_sources(stratum_yang_parse_tree_o PRIVATE
        yang_add_subtree_interface.cc
    )
endif()

target_include_directories(stratum_yang_parse_tree_o PRIVATE
    ${STRATUM_INCLUDES}
)

add_dependencies(stratum_yang_parse_tree_o
    gnmi_proto
    openconfig_proto_o
    stratum_proto
)
