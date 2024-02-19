# Builds protobuf object libraries
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

set(PROTO_PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# List of directories to be searched for Protobuf inputs.
string(JOIN ":" PROTO_IMPORT_PATH
    ${PROTO_PARENT_DIR}
    ${GOOGLEAPIS_SOURCE_DIR}
    ${P4RUNTIME_SOURCE_DIR}/proto
    ${STRATUM_SOURCE_DIR}
)

include(ProtobufCompile)

add_subdirectory(google)
add_subdirectory(p4)
add_subdirectory(gnmi)
add_subdirectory(openconfig)
add_subdirectory(stratum)
