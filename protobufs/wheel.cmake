#
# Copyright 2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
# Builds and installs a Python wheel.
#

add_custom_target(py-wheel ALL
  COMMAND
    rm -fr wheelgen
  COMMAND
    mkdir wheelgen
  COMMAND
    cp -pv ${CMAKE_CURRENT_SOURCE_DIR}/py/* wheelgen
  COMMAND
    cp -prv ${PY_OUT}/p4 wheelgen
  COMMAND
    # Generate Python wheel
    env -C wheelgen python setup.py bdist_wheel
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}
)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/wheelgen/dist/
  DESTINATION
    ${CMAKE_INSTALL_DATAROOTDIR}/stratum
)
