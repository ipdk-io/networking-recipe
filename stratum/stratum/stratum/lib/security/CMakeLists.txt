# CMake build file for //stratum/lib/security
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

##########################
# stratum_lib_security_o #
##########################

add_library(stratum_lib_security_o OBJECT
    auth_policy_checker.cc
    auth_policy_checker.h
    credentials_manager.cc
    credentials_manager.h
)

target_link_libraries(stratum_lib_security_o PUBLIC absl::strings)

target_include_directories(stratum_lib_security_o PRIVATE ${STRATUM_INCLUDES})

add_dependencies(stratum_lib_security_o stratum_proto) # gnmi_proto?
