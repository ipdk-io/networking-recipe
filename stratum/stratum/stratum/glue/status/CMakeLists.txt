# Build file for //stratum/glue/status
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

#########################
# stratum_glue_status_o #
#########################

add_library(stratum_glue_status_o OBJECT
    canonical_errors.cc
    canonical_errors.h
    posix_error_space.cc
    posix_error_space.h
    status.cc
    status.h
    status_macros.cc
    status_macros.h
    statusor.cc
    statusor.h
)

target_include_directories(stratum_glue_status_o PUBLIC
    ${DEPEND_INSTALL_DIR}/include
    ${STRATUM_INCLUDES}
)

target_link_libraries(stratum_glue_status_o PUBLIC
    gflags::gflags_shared
    glog::glog
)
