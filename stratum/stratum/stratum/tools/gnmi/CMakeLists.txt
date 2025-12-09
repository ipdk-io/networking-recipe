# Build file for //stratum/tools/gnmi
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

############
# gnmi_cli #
############

add_executable(gnmi_cli gnmi_cli.cc)

set_install_rpath(gnmi_cli ${EXEC_ELEMENT} ${DEP_ELEMENT})

target_link_libraries(gnmi_cli PUBLIC
    stratum_static
    gnmi_proto
    gflags::gflags_shared
    gRPC::grpc
    gRPC::grpc++
    re2::re2
    protobuf::libprotobuf
    pthread
    google_rpc_proto
)

if(HAVE_POSIX_AIO)
    target_link_libraries(gnmi_cli PUBLIC rt)
endif()

if(NOT DPDK_TARGET)
    install(TARGETS gnmi_cli RUNTIME)
endif()
