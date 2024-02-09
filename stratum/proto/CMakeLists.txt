# Builds protobuf object libraries
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

#############################
# Define Generate functions #
#############################

set(PROTO_PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# List of directories to be searched for Protobuf inputs.
string(JOIN ":" PROTO_IMPORT_PATH
    ${PROTO_PARENT_DIR}
    ${GOOGLEAPIS_SOURCE_DIR}
    ${P4RUNTIME_SOURCE_DIR}/proto
    ${STRATUM_SOURCE_DIR}
    /usr/local/include      # variable?
)

set(PB_HEADER_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR}/stratum/pb)
set(DOT_PROTO_INSTALL_DIR ${CMAKE_INSTALL_DATAROOTDIR}/stratum/proto)

include(ProtobufCompile)

#######################
# Build libgrpc_proto #
#######################

set(GRPC_PROTO_FILES
    google/rpc/status.proto
    google/rpc/code.proto
)

generate_proto_files("${GRPC_PROTO_FILES}" "${GOOGLEAPIS_SOURCE_DIR}")

# Internal target
add_library(grpc_proto SHARED
    ${PB_OUT_DIR}/google/rpc/status.pb.cc
    ${PB_OUT_DIR}/google/rpc/status.pb.h
    ${PB_OUT_DIR}/google/rpc/code.pb.cc
    ${PB_OUT_DIR}/google/rpc/code.pb.h
)

target_include_directories(grpc_proto PRIVATE ${PB_OUT_DIR})

target_link_libraries(grpc_proto PUBLIC protobuf::libprotobuf)

set_install_rpath(grpc_proto ${DEP_ELEMENT})

install(TARGETS grpc_proto LIBRARY)

############################
# Build libp4runtime_proto #
############################

set(P4RT_PROTO_FILES
    p4/v1/p4data.proto
    p4/v1/p4runtime.proto
    p4/config/v1/p4info.proto
    p4/config/v1/p4types.proto
)

generate_proto_files("${P4RT_PROTO_FILES}" "${P4RUNTIME_SOURCE_DIR}/proto")
generate_grpc_files("p4/v1/p4runtime.proto" "${P4RUNTIME_SOURCE_DIR}/proto")

# External target
add_library(p4runtime_proto SHARED
    ${PB_OUT_DIR}/p4/v1/p4runtime.pb.cc
    ${PB_OUT_DIR}/p4/v1/p4runtime.pb.h
    ${PB_OUT_DIR}/p4/v1/p4runtime.grpc.pb.cc
    ${PB_OUT_DIR}/p4/v1/p4runtime.grpc.pb.h
    ${PB_OUT_DIR}/p4/v1/p4data.pb.cc
    ${PB_OUT_DIR}/p4/v1/p4data.pb.h
    ${PB_OUT_DIR}/p4/config/v1/p4types.pb.cc
    ${PB_OUT_DIR}/p4/config/v1/p4types.pb.h
    ${PB_OUT_DIR}/p4/config/v1/p4info.pb.cc
    ${PB_OUT_DIR}/p4/config/v1/p4info.pb.h
)

#add_dependencies(p4runtime_proto grpc_proto)

target_include_directories(p4runtime_proto PRIVATE ${PB_OUT_DIR})

target_link_libraries(p4runtime_proto
    PUBLIC
        grpc_proto
        protobuf::libprotobuf
        absl::synchronization
)

set_install_rpath(p4runtime_proto $ORIGIN ${DEP_ELEMENT})

install(TARGETS p4runtime_proto LIBRARY)

##############################
# Build openconfig libraries #
##############################

add_subdirectory(gnmi)
add_subdirectory(openconfig)

##########################
# Build libstratum_proto #
##########################

set(STRATUM_P4_PROTO_FILES
    stratum/public/proto/error.proto
    stratum/hal/lib/common/common.proto
    stratum/hal/lib/p4/forwarding_pipeline_configs.proto
    stratum/hal/lib/phal/db.proto
)

set(STRATUM_BF_PROTO_FILES
    stratum/public/proto/p4_table_defs.proto
    stratum/public/proto/p4_annotation.proto
    stratum/hal/lib/p4/p4_control.proto
    stratum/hal/lib/p4/common_flow_entry.proto
    stratum/hal/lib/p4/p4_table_map.proto
    stratum/hal/lib/p4/p4_pipeline_config.proto
    stratum/hal/lib/tdi/tdi.proto
)

generate_proto_files("${STRATUM_P4_PROTO_FILES}" "${STRATUM_SOURCE_DIR}")
generate_proto_files("${STRATUM_BF_PROTO_FILES}" "${STRATUM_SOURCE_DIR}")

# stratum_proto1_o
add_library(stratum_proto1_o OBJECT
    ${PB_OUT_DIR}/stratum/public/proto/p4_table_defs.pb.h
    ${PB_OUT_DIR}/stratum/public/proto/p4_table_defs.pb.cc
    ${PB_OUT_DIR}/stratum/public/proto/p4_annotation.pb.h
    ${PB_OUT_DIR}/stratum/public/proto/p4_annotation.pb.cc
    ${PB_OUT_DIR}/stratum/hal/lib/p4/p4_control.pb.h
    ${PB_OUT_DIR}/stratum/hal/lib/p4/p4_control.pb.cc
    ${PB_OUT_DIR}/stratum/hal/lib/p4/common_flow_entry.pb.h
    ${PB_OUT_DIR}/stratum/hal/lib/p4/common_flow_entry.pb.cc
    ${PB_OUT_DIR}/stratum/hal/lib/p4/p4_table_map.pb.h
    ${PB_OUT_DIR}/stratum/hal/lib/p4/p4_table_map.pb.cc
    ${PB_OUT_DIR}/stratum/hal/lib/p4/p4_pipeline_config.pb.h
    ${PB_OUT_DIR}/stratum/hal/lib/p4/p4_pipeline_config.pb.cc
    ${PB_OUT_DIR}/stratum/hal/lib/common/common.pb.cc
    ${PB_OUT_DIR}/stratum/hal/lib/common/common.pb.h
    ${PB_OUT_DIR}/stratum/hal/lib/tdi/tdi.pb.h
    ${PB_OUT_DIR}/stratum/hal/lib/tdi/tdi.pb.cc
)

# Ensure that the header files on which we depend have been generated
# before we start building the current library.
add_dependencies(stratum_proto1_o p4runtime_proto)

target_include_directories(stratum_proto1_o PRIVATE ${PB_OUT_DIR})

# stratum_proto2_o
add_library(stratum_proto2_o OBJECT
    ${PB_OUT_DIR}/stratum/public/proto/error.pb.cc
    ${PB_OUT_DIR}/stratum/public/proto/error.pb.h
    ${PB_OUT_DIR}/stratum/hal/lib/p4/forwarding_pipeline_configs.pb.cc
    ${PB_OUT_DIR}/stratum/hal/lib/p4/forwarding_pipeline_configs.pb.h
    ${PB_OUT_DIR}/stratum/hal/lib/phal/db.pb.cc
    ${PB_OUT_DIR}/stratum/hal/lib/phal/db.pb.h
)

# Ensure that the header files on which we depend have been generated
# before we start building the current library.
add_dependencies(stratum_proto2_o p4runtime_proto)
add_dependencies(stratum_proto2_o stratum_proto1_o)

target_include_directories(stratum_proto2_o PRIVATE ${PB_OUT_DIR})

# stratum_proto
add_library(stratum_proto SHARED
    $<TARGET_OBJECTS:stratum_proto1_o>
    $<TARGET_OBJECTS:stratum_proto2_o>
)

target_link_libraries(stratum_proto PUBLIC protobuf::libprotobuf)

install(TARGETS stratum_proto LIBRARY)

########################
# Build p4_role_config #
########################

generate_proto_files("stratum/public/proto/p4_role_config.proto" "${STRATUM_SOURCE_DIR}")

add_library(p4_role_config SHARED
    ${PB_OUT_DIR}/stratum/public/proto/p4_role_config.pb.cc
    ${PB_OUT_DIR}/stratum/public/proto/p4_role_config.pb.h
)

target_include_directories(p4_role_config PUBLIC ${PB_OUT_DIR})

install(TARGETS p4_role_config LIBRARY)
