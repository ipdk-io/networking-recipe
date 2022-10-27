# Builds OpenConfig Protobuf library

cmake_minimum_required(VERSION 3.5)

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

# ygot_proto_o
add_library(ygot_proto_o OBJECT
    ${PROTO_OUT_DIR}/${YGOT_PROTO_PATH}/yext/yext.pb.cc
    ${PROTO_OUT_DIR}/${YGOT_PROTO_PATH}/yext/yext.pb.h
    ${PROTO_OUT_DIR}/${YGOT_PROTO_PATH}/ywrapper/ywrapper.pb.cc
    ${PROTO_OUT_DIR}/${YGOT_PROTO_PATH}/ywrapper/ywrapper.pb.h
)

target_include_directories(ygot_proto_o PRIVATE ${PROTO_OUT_DIR})

# openconfig_enums_proto_o
add_library(openconfig_enums_proto_o OBJECT
    ${PROTO_OUT_DIR}/openconfig/enums/enums.pb.cc
    ${PROTO_OUT_DIR}/openconfig/enums/enums.pb.h
)

add_dependencies(openconfig_enums_proto_o ygot_proto_o)

target_include_directories(
    openconfig_enums_proto_o PRIVATE ${PROTO_OUT_DIR})

# openconfig_proto_o
add_library(openconfig_proto_o OBJECT
    ${PROTO_OUT_DIR}/openconfig/openconfig.pb.cc
    ${PROTO_OUT_DIR}/openconfig/openconfig.pb.h
)

add_dependencies(openconfig_proto_o openconfig_enums_proto_o)

target_include_directories(openconfig_proto_o PRIVATE ${PROTO_OUT_DIR})

# openconfig_goog_bcm_proto_o
add_library(openconfig_goog_bcm_proto_o OBJECT
    ${PROTO_OUT_DIR}/stratum/public/proto/openconfig-goog-bcm.pb.cc
    ${PROTO_OUT_DIR}/stratum/public/proto/openconfig-goog-bcm.pb.h
)

add_dependencies(openconfig_goog_bcm_proto_o ygot_proto_o)

target_include_directories(
    openconfig_goog_bcm_proto_o PRIVATE ${PROTO_OUT_DIR})

# openconfig_proto
add_library(openconfig_proto SHARED
    $<TARGET_OBJECTS:ygot_proto_o>
    $<TARGET_OBJECTS:openconfig_enums_proto_o>
    $<TARGET_OBJECTS:openconfig_proto_o>
    $<TARGET_OBJECTS:openconfig_goog_bcm_proto_o>
)

install(TARGETS openconfig_proto LIBRARY)
