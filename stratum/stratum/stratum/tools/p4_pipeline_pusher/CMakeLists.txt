# CMake build file for //stratum/tools/p4_pipeline_pusher
#
# Copyright 2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

######################
# p4_pipeline_pusher #
######################

add_executable(p4_pipeline_pusher
    p4_pipeline_pusher.cc
)

set_install_rpath(p4_pipeline_pusher ${EXEC_ELEMENT} ${DEP_ELEMENT})

target_link_libraries(p4_pipeline_pusher PUBLIC
    stratum_static
    $<TARGET_OBJECTS:p4runtime_session_o>
)

target_link_libraries(p4_pipeline_pusher PUBLIC p4_role_config_proto)

install(TARGETS p4_pipeline_pusher RUNTIME)
