# Builds protobuf object libraries

cmake_minimum_required(VERSION 3.5)

find_package(Protobuf REQUIRED)
if(NOT Protobuf_FOUND)
    message(SEND_ERROR "Error: Protobuf package not found")
endif()

###############################
# Get path to GRPC C++ plugin #
###############################

execute_process(
    COMMAND which grpc_cpp_plugin
    OUTPUT_VARIABLE GRPC_CPP_PLUGIN
    RESULT_VARIABLE GRPC_CPP_PLUGIN_ERROR
)

if(GRPC_CPP_PLUGIN_ERROR)
    message(FATAL_ERROR "Error: Could not find grpc_cpp_plugin")
else()
    string(STRIP "${GRPC_CPP_PLUGIN}" GRPC_CPP_PLUGIN)
endif()

##########################
# Compile protobuf files #
##########################

set(PROTO_PARENT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# List of directories to be searched for Protobuf inputs.
string(JOIN ":" PROTO_IMPORT_PATH
    ${PROTO_PARENT_DIR}
    ${GOOGLE_SOURCE_DIR}
    ${P4RUNTIME_SOURCE_DIR}/proto
    ${STRATUM_SOURCE_DIR}
    /usr/local/include
)

set(RPC_PROTO_FILES
    google/rpc/status.proto
    google/rpc/code.proto
)

set(P4V1_PROTO_FILES
    p4/v1/p4data.proto
    p4/v1/p4runtime.proto
    p4/config/v1/p4info.proto
    p4/config/v1/p4types.proto
)

set(PROCMON_PROTO_FILES
    stratum/procmon/procmon.proto
)

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

########################
# generate_proto_files #
########################

# Generates C++ files for protobufs.
function(generate_proto_files PROTO_FILES SRC_DIR)
    foreach(_file ${PROTO_FILES})
        get_filename_component(_path ${_file} DIRECTORY)
        get_filename_component(_name ${_file} NAME_WE)

        set(_src ${PROTO_OUT_DIR}/${_path}/${_name}.pb.cc)
        set(_hdr ${PROTO_OUT_DIR}/${_path}/${_name}.pb.h)

        set_source_files_properties(${_src} ${_hdr} PROPERTIES GENERATED TRUE)

        add_custom_command(
            OUTPUT
                ${_src} ${_hdr}
            COMMAND
                ${PROTOBUF_PROTOC_EXECUTABLE}
                --proto_path=${PROTO_IMPORT_PATH}
                --cpp_out=${PROTO_OUT_DIR}
                -I${STRATUM_SOURCE_DIR}
                ${_file}
            WORKING_DIRECTORY
                ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS
                ${SRC_DIR}/${_file}
            COMMENT
                "Generating C++ files for ${_file}"
            VERBATIM
        )

        # Install header files in include/stratum/pb.
        install(FILES ${_hdr} DESTINATION
                ${CMAKE_INSTALL_PREFIX}/include/stratum/pb/${_path})

        if(INSTALL_PROTO)
            # Install protobuf files in share/stratum/proto.
            install(FILES ${SRC_DIR}/${_file} DESTINATION
                    ${CMAKE_INSTALL_PREFIX}/share/stratum/proto/${_path})
        endif()
    endforeach()
endfunction(generate_proto_files)

#######################
# generate_grpc_files #
#######################

# Generates GRPC C++ files for protobufs.
function(generate_grpc_files PROTO_FILES SRC_DIR)
    foreach(_file ${PROTO_FILES})
        get_filename_component(_path ${_file} DIRECTORY)
        get_filename_component(_name ${_file} NAME_WE)

        set(_src ${PROTO_OUT_DIR}/${_path}/${_name}.grpc.pb.cc)
        set(_hdr ${PROTO_OUT_DIR}/${_path}/${_name}.grpc.pb.h)

        set_source_files_properties(${_src} ${_hdr} PROPERTIES GENERATED TRUE)

        add_custom_command(
            OUTPUT
                ${_src} ${_hdr}
            COMMAND
                ${PROTOBUF_PROTOC_EXECUTABLE}
                --proto_path=${PROTO_IMPORT_PATH}
                --grpc_out=${PROTO_OUT_DIR}
                --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
                -I${STRATUM_SOURCE_DIR}
                ${_file}
            WORKING_DIRECTORY
                ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS
                ${SRC_DIR}/${_file}
            COMMENT
                "Generating gRPC files for ${_file}"
            VERBATIM
        )

        install(FILES ${_hdr} DESTINATION
                ${CMAKE_INSTALL_PREFIX}/include/stratum/pb/${_path})

        if(INSTALL-PROTO)
            # Install protobuf files in share/stratum/proto.
            install(FILES ${SRC_DIR}/${_file} DESTINATION
                    ${CMAKE_INSTALL_PREFIX}/share/stratum/proto/${_path})
        endif()
    endforeach()
endfunction(generate_grpc_files)

###############################
# Generate c++ protobuf files #
###############################

generate_proto_files("${RPC_PROTO_FILES}" "${GOOGLE_SOURCE_DIR}")

generate_proto_files("${P4V1_PROTO_FILES}" "${P4RUNTIME_SOURCE_DIR}/proto")
generate_grpc_files("p4/v1/p4runtime.proto" "${P4RUNTIME_SOURCE_DIR}/proto")

generate_proto_files("${STRATUM_P4_PROTO_FILES}" "${STRATUM_SOURCE_DIR}")

generate_proto_files("${STRATUM_BF_PROTO_FILES}" "${STRATUM_SOURCE_DIR}")

if(WITH_PROCMON)
    generate_proto_files("${PROCMON_PROTO_FILES}" "${STRATUM_SOURCE_DIR}")
    generate_grpc_files("${PROCMON_PROTO_FILES}" "${STRATUM_SOURCE_DIR}")
endif()

######################
# Build librpc_proto #
######################

# Internal target
add_library(rpc_proto SHARED
    ${PROTO_OUT_DIR}/google/rpc/status.pb.cc
    ${PROTO_OUT_DIR}/google/rpc/status.pb.h
    ${PROTO_OUT_DIR}/google/rpc/code.pb.cc
    ${PROTO_OUT_DIR}/google/rpc/code.pb.h
)

target_include_directories(rpc_proto PRIVATE ${PROTO_OUT_DIR})

target_link_libraries(rpc_proto PUBLIC protobuf)

install(TARGETS rpc_proto LIBRARY)

#######################
# Build libp4v1_proto #
#######################

# External target
add_library(p4v1_proto SHARED
    ${PROTO_OUT_DIR}/p4/v1/p4runtime.pb.cc
    ${PROTO_OUT_DIR}/p4/v1/p4runtime.pb.h
    ${PROTO_OUT_DIR}/p4/v1/p4runtime.grpc.pb.cc
    ${PROTO_OUT_DIR}/p4/v1/p4runtime.grpc.pb.h
    ${PROTO_OUT_DIR}/p4/v1/p4data.pb.cc
    ${PROTO_OUT_DIR}/p4/v1/p4data.pb.h
    ${PROTO_OUT_DIR}/p4/config/v1/p4types.pb.cc
    ${PROTO_OUT_DIR}/p4/config/v1/p4types.pb.h
    ${PROTO_OUT_DIR}/p4/config/v1/p4info.pb.cc
    ${PROTO_OUT_DIR}/p4/config/v1/p4info.pb.h
)

target_include_directories(p4v1_proto PRIVATE ${PROTO_OUT_DIR})

target_link_libraries(p4v1_proto PUBLIC rpc_proto absl_synchronization)
add_dependencies(p4v1_proto rpc_proto)

install(TARGETS p4v1_proto LIBRARY)

###########################
# Build libprocmon_proto  #
###########################

if(WITH_PROCMON)
    add_library(procmon_proto SHARED
        ${PROTO_OUT_DIR}/stratum/procmon/procmon.pb.cc
        ${PROTO_OUT_DIR}/stratum/procmon/procmon.pb.h
        ${PROTO_OUT_DIR}/stratum/procmon/procmon.grpc.pb.cc
        ${PROTO_OUT_DIR}/stratum/procmon/procmon.grpc.pb.h
    )
    target_include_directories(procmon_proto PUBLIC ${PROTO_OUT_DIR})
    install(TARGETS procmon_proto LIBRARY)
endif()

##############################
# Build openconfig libraries #
##############################

add_subdirectory(gnmi)
if(WITH_GNOI)
    add_subdirectory(gnoi)
endif()
add_subdirectory(openconfig)

##########################
# Build libstratum_proto #
##########################

# stratum_proto1_o
add_library(stratum_proto1_o OBJECT
    ${PROTO_OUT_DIR}/stratum/public/proto/p4_table_defs.pb.h
    ${PROTO_OUT_DIR}/stratum/public/proto/p4_table_defs.pb.cc
    ${PROTO_OUT_DIR}/stratum/public/proto/p4_annotation.pb.h
    ${PROTO_OUT_DIR}/stratum/public/proto/p4_annotation.pb.cc
    ${PROTO_OUT_DIR}/stratum/hal/lib/p4/p4_control.pb.h
    ${PROTO_OUT_DIR}/stratum/hal/lib/p4/p4_control.pb.cc
    ${PROTO_OUT_DIR}/stratum/hal/lib/p4/common_flow_entry.pb.h
    ${PROTO_OUT_DIR}/stratum/hal/lib/p4/common_flow_entry.pb.cc
    ${PROTO_OUT_DIR}/stratum/hal/lib/p4/p4_table_map.pb.h
    ${PROTO_OUT_DIR}/stratum/hal/lib/p4/p4_table_map.pb.cc
    ${PROTO_OUT_DIR}/stratum/hal/lib/p4/p4_pipeline_config.pb.h
    ${PROTO_OUT_DIR}/stratum/hal/lib/p4/p4_pipeline_config.pb.cc
    ${PROTO_OUT_DIR}/stratum/hal/lib/common/common.pb.cc
    ${PROTO_OUT_DIR}/stratum/hal/lib/common/common.pb.h
    ${PROTO_OUT_DIR}/stratum/hal/lib/tdi/tdi.pb.h
    ${PROTO_OUT_DIR}/stratum/hal/lib/tdi/tdi.pb.cc
)

add_dependencies(stratum_proto1_o p4v1_proto)

target_include_directories(stratum_proto1_o PRIVATE ${PROTO_OUT_DIR})

# stratum_proto2_o
add_library(stratum_proto2_o OBJECT
    ${PROTO_OUT_DIR}/stratum/public/proto/error.pb.cc
    ${PROTO_OUT_DIR}/stratum/public/proto/error.pb.h
    ${PROTO_OUT_DIR}/stratum/hal/lib/p4/forwarding_pipeline_configs.pb.cc
    ${PROTO_OUT_DIR}/stratum/hal/lib/p4/forwarding_pipeline_configs.pb.h
    ${PROTO_OUT_DIR}/stratum/hal/lib/phal/db.pb.cc
    ${PROTO_OUT_DIR}/stratum/hal/lib/phal/db.pb.h
)

target_link_libraries(stratum_proto2_o PUBLIC p4v1_proto)
target_link_libraries(stratum_proto2_o PUBLIC pthread)

add_dependencies(stratum_proto2_o p4v1_proto)
add_dependencies(stratum_proto2_o stratum_proto1_o)

target_include_directories(stratum_proto2_o PRIVATE ${PROTO_OUT_DIR})

# stratum_proto
add_library(stratum_proto SHARED
    $<TARGET_OBJECTS:stratum_proto1_o>
    $<TARGET_OBJECTS:stratum_proto2_o>
)

install(TARGETS stratum_proto LIBRARY)

################################
# Download gNOI Protobuf files #
################################

include(ExternalProject)

# Update to correspond to the version used by Stratum.
# The definitions can be found in stratum/stratum/bazel/deps.bzl.
set(GNOI_COMMIT 437c62e630389aa4547b4f0521d0bca3fb2bf811)
set(GNOI_SHA 77d8c271adc22f94a18a5261c28f209370e87a5e615801a4e7e0d09f06da531f)

# Path to the stratum/proto/gnoi directory.
set(PROTO_GNOI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/gnoi)

ExternalProject_Add(fetch-gnoi
  URL https://github.com/openconfig/gnoi/archive/${GNOI_COMMIT}.zip
  URL_HASH SHA256=${GNOI_SHA}
  PATCH_COMMAND
    find . -name *.proto | xargs sed -i -e "s#github.com/openconfig/##g"
  COMMAND
    cp -v cert/cert.proto ${PROTO_GNOI_DIR}/cert
  COMMAND
    cp -v common/common.proto ${PROTO_GNOI_DIR}/common
  COMMAND
    cp -v diag/diag.proto ${PROTO_GNOI_DIR}/diag
  COMMAND
    cp -v file/file.proto ${PROTO_GNOI_DIR}/file
  COMMAND
    cp -v system/system.proto ${PROTO_GNOI_DIR}/system
  COMMAND
    cp -v types/types.proto ${PROTO_GNOI_DIR}/types
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
)

# The target is only built when expressly requested to do so.
#     cmake --build build --target fetch-gnoi
set_target_properties(fetch-gnoi PROPERTIES EXCLUDE_FROM_ALL TRUE)

