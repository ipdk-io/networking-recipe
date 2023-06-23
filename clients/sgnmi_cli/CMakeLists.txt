# Build file for sgnmi_cli.
#
# Copyright 2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

###################
# Build sgnmi_cli #
###################

add_executable(sgnmi_cli
    sgnmi_cli.cc
)

set_install_rpath(sgnmi_cli ${EXEC_ELEMENT} ${DEP_ELEMENT})

target_link_libraries(sgnmi_cli
    PUBLIC
        stratum_static
        gnmi_proto grpc_proto
        grpc protobuf gflags grpc++
        pthread re2
)

if(HAVE_POSIX_AIO)
    target_link_libraries(sgnmi_cli PUBLIC rt)
endif()

target_include_directories(sgnmi_cli
    PRIVATE
        ${STRATUM_SOURCE_DIR}
        ${PB_OUT_DIR}
)

add_dependencies(sgnmi_cli
    gnmi_proto
    grpc_proto
)

install(TARGETS sgnmi_cli RUNTIME)
