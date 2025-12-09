# CMake build file for //stratum/tools/stratum_replay
#
# Copyright 2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

##################
# stratum_replay #
##################

add_executable(stratum_replay
    stratum_replay.cc
)

set_install_rpath(stratum_replay ${EXEC_ELEMENT} ${DEP_ELEMENT})

target_link_libraries(stratum_replay PUBLIC
    stratum_static
)

install(TARGETS stratum_replay RUNTIME)
