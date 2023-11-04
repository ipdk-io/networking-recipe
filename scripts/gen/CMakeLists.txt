#
# Copyright 2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

set(GENSCRIPT_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/genscript.py CACHE PATH "")
mark_as_advanced(GENSCRIPT_COMMAND)

function(generate_script _template _outfile)
  get_filename_component(_filename ${_outfile} NAME)
  add_custom_command(
    OUTPUT
      ${_outfile}
    COMMAND
      ${GENSCRIPT_COMMAND}
      --template=${_template}
      --output=${_outfile}
      --target=${TARGETTYPE}
    WORKING_DIRECTORY
      ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT
      "Generating ${_filename}"
    VERBATIM
  )
endfunction()
