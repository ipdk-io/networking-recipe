# Build file for //stratum/hal/lib/common
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

############################
# stratum_hal_lib_common_o #
############################

add_library(stratum_hal_lib_common_o OBJECT
    channel_writer_wrapper.h
    config_monitoring_service.cc
    config_monitoring_service.h
    error_buffer.cc
    error_buffer.h
    gnmi_publisher.cc
    gnmi_publisher.h
    openconfig_converter.cc
    openconfig_converter.h
    p4_service.cc
    p4_service.h
    server_writer_wrapper.h
    target_options.cc
    target_options.h
    utils.cc
    utils.h
    writer_interface.h
)

set_source_files_properties(
    config_monitoring_service.cc
    PROPERTIES COMPILE_OPTIONS "-Wno-deprecated-declarations"
)

target_include_directories(stratum_hal_lib_common_o PRIVATE
    ${STRATUM_INCLUDES}
    ${SDE_INSTALL_DIR}/include
)

add_dependencies(stratum_hal_lib_common_o
    gnmi_proto
    openconfig_proto
    stratum_proto
)

########################
# Build common_proto_o #
########################

generate_proto_files(
    "stratum/hal/lib/common/common.proto"
    "${STRATUM_SOURCE_DIR}"
)

add_library(common_proto_o OBJECT
    ${PB_OUT_DIR}/stratum/hal/lib/common/common.pb.cc
    ${PB_OUT_DIR}/stratum/hal/lib/common/common.pb.h
)

target_include_directories(common_proto_o PUBLIC ${PB_OUT_DIR})
