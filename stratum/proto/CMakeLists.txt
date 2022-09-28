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

set(STRATUM_P4_PROTO_FILES
    stratum/public/proto/error.proto
    stratum/hal/lib/common/common.proto
    stratum/hal/lib/p4/forwarding_pipeline_configs.proto
    stratum/hal/lib/phal/db.proto
)

set(GNMI_PROTO_FILES
    gnmi/gnmi.proto
    gnmi/gnmi_ext.proto
)

if(WITH_OPENCONFIG)
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
endif()

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
    foreach(PROTO_FILE ${PROTO_FILES})
        get_filename_component(PROTO_PATH ${PROTO_FILE} DIRECTORY)
        get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)

        set(SRCS ${PROTO_OUT_DIR}/${PROTO_PATH}/${PROTO_NAME}.pb.cc)
        set(HDRS ${PROTO_OUT_DIR}/${PROTO_PATH}/${PROTO_NAME}.pb.h)

        set_source_files_properties(${SRCS} ${HDRS} PROPERTIES GENERATED TRUE)

        add_custom_command(
            OUTPUT
                ${SRCS} ${HDRS}
            COMMAND
                ${PROTOBUF_PROTOC_EXECUTABLE}
                --proto_path=${PROTO_IMPORT_PATH}
                --cpp_out=${PROTO_OUT_DIR}
                -I${STRATUM_SOURCE_DIR}
                ${PROTO_FILE}
            WORKING_DIRECTORY
                ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS
                ${SRC_DIR}/${PROTO_FILE}
            COMMENT
                "Generating C++ files for ${PROTO_FILE}"
            VERBATIM
        )

        # Install header files in include/stratum/pb.
        install(FILES ${HDRS} DESTINATION
                ${CMAKE_INSTALL_PREFIX}/include/stratum/pb/${PROTO_PATH})

        if(INSTALL-PROTO)
            # Install protobuf files in share/stratum/proto.
            install(FILES ${SRC_DIR}/${PROTO_FILE} DESTINATION
                    ${CMAKE_INSTALL_PREFIX}/share/stratum/proto/${PROTO_PATH})
        endif()
    endforeach()
endfunction(generate_proto_files)

#######################
# generate_grpc_files #
#######################

# Generates GRPC C++ files for protobufs.
function(generate_grpc_files PROTO_FILES SRC_DIR)
    foreach(PROTO_FILE ${PROTO_FILES})
        get_filename_component(PROTO_PATH ${PROTO_FILE} DIRECTORY)
        get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)

        set(SRCS ${PROTO_OUT_DIR}/${PROTO_PATH}/${PROTO_NAME}.grpc.pb.cc)
        set(HDRS ${PROTO_OUT_DIR}/${PROTO_PATH}/${PROTO_NAME}.grpc.pb.h)

        set_source_files_properties(${SRCS} ${HDRS} PROPERTIES GENERATED TRUE)

        add_custom_command(
            OUTPUT
                ${SRCS} ${HDRS}
            COMMAND
                ${PROTOBUF_PROTOC_EXECUTABLE}
                --proto_path=${PROTO_IMPORT_PATH}
                --grpc_out=${PROTO_OUT_DIR}
                --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
                -I${STRATUM_SOURCE_DIR}
                ${PROTO_FILE}
            WORKING_DIRECTORY
                ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS
                ${SRC_DIR}/${PROTO_FILE}
            COMMENT
                "Generating gRPC files for ${PROTO_FILE}"
            VERBATIM
        )

        install(FILES ${HDRS} DESTINATION
                "${CMAKE_INSTALL_PREFIX}/include/stratum/pb/${PROTO_PATH}")

        if(INSTALL-PROTO)
            # Install protobuf files in share/stratum/proto.
            install(FILES ${SRC_DIR}/${PROTO_FILE} DESTINATION
                    ${CMAKE_INSTALL_PREFIX}/share/stratum/proto/${PROTO_PATH})
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

generate_proto_files("${GNMI_PROTO_FILES}" "${PROTO_PARENT_DIR}")
generate_grpc_files("gnmi/gnmi.proto" "${PROTO_PARENT_DIR}")

if(WITH_OPENCONFIG)
    generate_proto_files("${YGOT_PROTO_FILES}" "${PROTO_PARENT_DIR}")
    generate_proto_files("${OPENCONFIG_PROTO_FILES}" "${PROTO_PARENT_DIR}")
    generate_proto_files("${STRATUM_OC_PROTO_FILES}" "${STRATUM_SOURCE_DIR}")
endif()

generate_proto_files("${STRATUM_BF_PROTO_FILES}" "${STRATUM_SOURCE_DIR}")

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

#######################
# Build libgnmi_proto #
#######################

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

###########################
#  Build openconfig_proto #
###########################

if(WITH_OPENCONFIG)
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
endif(WITH_OPENCONFIG)

###########################
#  Build libstratum_proto #
###########################

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
