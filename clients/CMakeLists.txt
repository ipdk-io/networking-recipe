# Build file for clients.
#
# Copyright 2022-2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

add_subdirectory(gnmi-ctl)
add_subdirectory(p4rt-ctl)
add_subdirectory(sgnmi_cli)

##################
# Build gnmi_cli #
##################

add_executable(gnmi_cli
    ${STRATUM_SOURCE_DIR}/stratum/tools/gnmi/gnmi_cli.cc
)

set_install_rpath(gnmi_cli ${EXEC_ELEMENT} ${DEP_ELEMENT})

target_link_libraries(gnmi_cli
    PUBLIC
        stratum_static
        gnmi_proto grpc_proto
        grpc protobuf gflags grpc++
        pthread re2
)

if(HAVE_POSIX_AIO)
    target_link_libraries(gnmi_cli PUBLIC rt)
endif()

target_include_directories(gnmi_cli
    PRIVATE
        ${STRATUM_SOURCE_DIR}
        ${PB_OUT_DIR}
)

add_dependencies(gnmi_cli
    gnmi_proto
    grpc_proto
)

if(NOT DPDK_TARGET)
    install(TARGETS gnmi_cli RUNTIME)
endif()

##############################
# Build tdi_pipeline_builder #
##############################

add_executable(tdi_pipeline_builder
    ${STRATUM_SOURCE_DIR}/stratum/hal/bin/tdi/tdi_pipeline_builder.cc
)

# Suppress "warning: attribute ignored" on ABSL_MUST_USE_RESULT [[nodiscard]]
target_compile_options(tdi_pipeline_builder PRIVATE -Wno-attributes)

set_install_rpath(tdi_pipeline_builder ${EXEC_ELEMENT} ${DEP_ELEMENT})

target_include_directories(tdi_pipeline_builder
    PRIVATE
        ${STRATUM_SOURCE_DIR}
        ${PB_OUT_DIR}
)

target_link_libraries(tdi_pipeline_builder PUBLIC stratum_proto p4runtime_proto)

target_link_libraries(tdi_pipeline_builder PUBLIC
    protobuf gflags glog grpc grpc++
)

target_link_libraries(tdi_pipeline_builder PRIVATE stratum_static)

add_dependencies(tdi_pipeline_builder p4runtime_proto stratum_proto)

install(TARGETS tdi_pipeline_builder RUNTIME)
