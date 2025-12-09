# Build file for //stratum/hal/bin/tdi
#
# Copyright 2022-2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

##################
# stratum_main_o #
##################

add_library(stratum_main_o OBJECT
    main.h
)

if(DPDK_TARGET)
    add_subdirectory(dpdk)
elseif(ES2K_TARGET)
    add_subdirectory(es2k)
elseif(TOFINO_TARGET)
    add_subdirectory(tofino)
endif()

target_include_directories(stratum_main_o PRIVATE
    ${STRATUM_INCLUDES}
    ${SDE_INSTALL_DIR}/include
)

add_dependencies(stratum_main_o stratum_proto gnmi_proto)

########################
# tdi_pipeline_builder #
########################

add_executable(tdi_pipeline_builder tdi_pipeline_builder.cc)

# Suppress "warning: attribute ignored" on ABSL_MUST_USE_RESULT [[nodiscard]]
target_compile_options(tdi_pipeline_builder PRIVATE -Wno-attributes)

set_install_rpath(tdi_pipeline_builder ${EXEC_ELEMENT} ${DEP_ELEMENT})

target_include_directories(tdi_pipeline_builder PRIVATE
    ${STRATUM_SOURCE_DIR}
    ${PB_OUT_DIR}
)

target_link_libraries(tdi_pipeline_builder PRIVATE
    stratum_static
)

target_link_libraries(tdi_pipeline_builder PUBLIC
    stratum_proto
    p4runtime_proto
)

target_link_libraries(tdi_pipeline_builder PUBLIC
    gflags::gflags_shared
    glog::glog
    gRPC::grpc
    gRPC::grpc++
    protobuf::libprotobuf
)

#add_dependencies(tdi_pipeline_builder p4runtime_proto stratum_proto)

install(TARGETS tdi_pipeline_builder RUNTIME)
