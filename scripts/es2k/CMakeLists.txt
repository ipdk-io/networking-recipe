# CMake build file for es2k scripts
#
# Copyright 2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

set(_filename copy_config_files.sh)
set(_template ${_filename}.jinja)
set(_outfile ${CMAKE_CURRENT_BINARY_DIR}/${_filename})

add_custom_target(es2k-copy-script ALL
  COMMAND
    ${GENSCRIPT_COMMAND}
    --template=${_template}
    --output=${_outfile}
    --target=es2k
  BYPRODUCTS
    ${_outfile}
  WORKING_DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT
    "Generating ${_filename}"
  VERBATIM
)

# Sets execute permissions
install(
  PROGRAMS ${_outfile}
  TYPE SBIN
)
