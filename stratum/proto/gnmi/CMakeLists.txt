# Builds stratum/proto/openconfig
cmake_minimum_required(VERSION 3.5)

set(GNMI_PROTO_FILES
    gnmi/gnmi.proto
    gnmi/gnmi_ext.proto
)

generate_proto_files("${GNMI_PROTO_FILES}" "${PROTO_PARENT_DIR}")
generate_grpc_files("gnmi/gnmi.proto" "${PROTO_PARENT_DIR}")

add_library(gnmi_proto SHARED
    ${PROTO_OUT_DIR}/gnmi/gnmi.grpc.pb.cc
    ${PROTO_OUT_DIR}/gnmi/gnmi.grpc.pb.h
    ${PROTO_OUT_DIR}/gnmi/gnmi.pb.cc
    ${PROTO_OUT_DIR}/gnmi/gnmi.pb.h
    ${PROTO_OUT_DIR}/gnmi/gnmi_ext.pb.cc
    ${PROTO_OUT_DIR}/gnmi/gnmi_ext.pb.h
)

add_dependencies(gnmi_proto rpc_proto)

target_include_directories(gnmi_proto PRIVATE ${PROTO_OUT_DIR})

target_link_libraries(gnmi_proto
    PUBLIC
        rpc_proto
        absl_hash
        absl_strings
        absl_synchronization
        absl_time
)

install(TARGETS gnmi_proto LIBRARY)
