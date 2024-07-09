# Builds OpenConfig Protobuf library
#
# Copyright 2022-2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

########################
# Compile .proto files #
########################

set(YGOT_PROTO_PATH github.com/openconfig/ygot/proto)

set(YGOT_PROTO_FILES
    ${YGOT_PROTO_PATH}/yext/yext.proto
    ${YGOT_PROTO_PATH}/ywrapper/ywrapper.proto
)

set(OPENCONFIG_PROTO_FILES
    openconfig/enums/enums.proto
    openconfig/openconfig.proto
)

set(STRATUM_OC_PROTO_FILES
    stratum/public/proto/openconfig-goog-bcm.proto
)

generate_proto_files("${YGOT_PROTO_FILES}" "${PROTO_PARENT_DIR}")
generate_proto_files("${OPENCONFIG_PROTO_FILES}" "${PROTO_PARENT_DIR}")
generate_proto_files("${STRATUM_OC_PROTO_FILES}" "${STRATUM_SOURCE_DIR}")

#############################
# Build libopenconfig_proto #
#############################

# ywrapper_proto_o
add_library(ywrapper_proto_o OBJECT
    ${PB_OUT_DIR}/${YGOT_PROTO_PATH}/yext/yext.pb.cc
    ${PB_OUT_DIR}/${YGOT_PROTO_PATH}/yext/yext.pb.h
    ${PB_OUT_DIR}/${YGOT_PROTO_PATH}/ywrapper/ywrapper.pb.cc
    ${PB_OUT_DIR}/${YGOT_PROTO_PATH}/ywrapper/ywrapper.pb.h
)

target_include_directories(ywrapper_proto_o PRIVATE ${PB_OUT_DIR})

# openconfig_enums_proto_o
add_library(openconfig_enums_proto_o OBJECT
    ${PB_OUT_DIR}/openconfig/enums/enums.pb.cc
    ${PB_OUT_DIR}/openconfig/enums/enums.pb.h
)

# Ensure that the header files on which we depend have been generated.
add_dependencies(openconfig_enums_proto_o ywrapper_proto_o)

target_include_directories(
    openconfig_enums_proto_o PRIVATE ${PB_OUT_DIR})

# openconfig_proto_o
add_library(openconfig_proto_o OBJECT
    ${PB_OUT_DIR}/openconfig/openconfig.pb.cc
    ${PB_OUT_DIR}/openconfig/openconfig.pb.h
)

# Ensure that the header files on which we depend have been generated.
add_dependencies(openconfig_proto_o openconfig_enums_proto_o)

target_include_directories(openconfig_proto_o PRIVATE ${PB_OUT_DIR})

# openconfig_goog_bcm_proto_o
add_library(openconfig_goog_bcm_proto_o OBJECT
    ${PB_OUT_DIR}/stratum/public/proto/openconfig-goog-bcm.pb.cc
    ${PB_OUT_DIR}/stratum/public/proto/openconfig-goog-bcm.pb.h
)

# Ensure that the header files on which we depend have been generated.
add_dependencies(openconfig_goog_bcm_proto_o ywrapper_proto_o)

target_include_directories(
    openconfig_goog_bcm_proto_o PRIVATE ${PB_OUT_DIR})

# openconfig_proto
add_library(openconfig_proto SHARED
    $<TARGET_OBJECTS:ywrapper_proto_o>
    $<TARGET_OBJECTS:openconfig_enums_proto_o>
    $<TARGET_OBJECTS:openconfig_proto_o>
    $<TARGET_OBJECTS:openconfig_goog_bcm_proto_o>
)

target_link_libraries(openconfig_proto PUBLIC protobuf::libprotobuf)

set_install_rpath(openconfig_proto ${DEP_ELEMENT})

install(TARGETS openconfig_proto LIBRARY)
