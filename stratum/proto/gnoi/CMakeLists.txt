# Builds stratum/proto/gnoi
cmake_minimum_required(VERSION 3.5)

set(GNOI_COMMON_FILES
    gnoi/common/common.proto
    gnoi/types/types.proto
)

set(GNOI_SERVICE_FILES
    gnoi/cert/cert.proto
    gnoi/diag/diag.proto
    gnoi/file/file.proto
    gnoi/system/system.proto
)

generate_proto_files("${GNOI_COMMON_FILES}" "${PROTO_PARENT_DIR}")
generate_proto_files("${GNOI_SERVICE_FILES}" "${PROTO_PARENT_DIR}")
generate_grpc_files("${GNOI_SERVICE_FILES}" "${PROTO_PARENT_DIR}")

# gnoi_types
add_library(gnoi_types SHARED
    ${PROTO_OUT_DIR}/gnoi/types/types.pb.cc
    ${PROTO_OUT_DIR}/gnoi/types/types.pb.h
)

target_include_directories(gnoi_types PUBLIC ${PROTO_OUT_DIR})

# gnoi_common
add_library(gnoi_common SHARED
    ${PROTO_OUT_DIR}/gnoi/common/common.pb.cc
    ${PROTO_OUT_DIR}/gnoi/common/common.pb.h
)
add_dependencies(gnoi_common gnoi_types)

target_include_directories(gnoi_common PUBLIC ${PROTO_OUT_DIR})
target_link_libraries(gnoi_common PUBLIC gnoi_types)

# gnoi_cert
add_library(gnoi_cert SHARED
    ${PROTO_OUT_DIR}/gnoi/cert/cert.pb.cc
    ${PROTO_OUT_DIR}/gnoi/cert/cert.pb.h
    ${PROTO_OUT_DIR}/gnoi/cert/cert.grpc.pb.cc
    ${PROTO_OUT_DIR}/gnoi/cert/cert.grpc.pb.h
)
add_dependencies(gnoi_cert gnoi_types)

target_include_directories(gnoi_cert PUBLIC ${PROTO_OUT_DIR})
target_link_libraries(gnoi_cert PUBLIC gnoi_types)

# gnoi_diag
add_library(gnoi_diag SHARED
    ${PROTO_OUT_DIR}/gnoi/diag/diag.pb.cc
    ${PROTO_OUT_DIR}/gnoi/diag/diag.pb.h
    ${PROTO_OUT_DIR}/gnoi/diag/diag.grpc.pb.cc
    ${PROTO_OUT_DIR}/gnoi/diag/diag.grpc.pb.h
)
add_dependencies(gnoi_diag gnoi_types)

target_include_directories(gnoi_diag PUBLIC ${PROTO_OUT_DIR})
target_link_libraries(gnoi_diag PUBLIC gnoi_types)

# gnoi_file
add_library(gnoi_file SHARED
    ${PROTO_OUT_DIR}/gnoi/file/file.pb.cc
    ${PROTO_OUT_DIR}/gnoi/file/file.pb.h
    ${PROTO_OUT_DIR}/gnoi/file/file.grpc.pb.cc
    ${PROTO_OUT_DIR}/gnoi/file/file.grpc.pb.h
)
add_dependencies(gnoi_file gnoi_types)

target_include_directories(gnoi_file PUBLIC ${PROTO_OUT_DIR})
target_link_libraries(gnoi_file PUBLIC gnoi_types)

# gnoi_system
add_library(gnoi_system SHARED
    ${PROTO_OUT_DIR}/gnoi/system/system.pb.cc
    ${PROTO_OUT_DIR}/gnoi/system/system.pb.h
    ${PROTO_OUT_DIR}/gnoi/system/system.grpc.pb.cc
    ${PROTO_OUT_DIR}/gnoi/system/system.grpc.pb.h
)
add_dependencies(gnoi_system gnoi_common gnoi_types)

target_include_directories(gnoi_system PUBLIC ${PROTO_OUT_DIR})
target_link_libraries(gnoi_system PUBLIC gnoi_common gnoi_types)

install(TARGETS gnoi_types gnoi_common LIBRARY)
install(TARGETS gnoi_cert gnoi_diag gnoi_file gnoi_system LIBRARY)
