#
# Copyright 2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
# Builds and installs a Python wheel.
#

set(WHEELGEN ${CMAKE_BINARY_DIR}/wheelgen)

file(MAKE_DIRECTORY ${WHEELGEN})

file(COPY
    content/LICENSE
    content/README.md
    content/pyproject.toml
    content/setup.py
  DESTINATION
    ${WHEELGEN}
)

configure_file(
  content/setup.cfg.in
  ${WHEELGEN}/setup.cfg
  @ONLY
)

add_custom_target(py-wheel ALL
  COMMAND
    cp -pr ${PY_OUT}/p4 ${WHEELGEN}
  COMMAND
    env -C ${WHEELGEN} python -m build
  DEPENDS
    google_py_out
    p4rt_py_out
  COMMENT
    "Generating Python packages"
  VERBATIM
)

install(
  DIRECTORY
    ${WHEELGEN}/dist/
  DESTINATION
    ${CMAKE_INSTALL_DATAROOTDIR}/p4runtime
)
