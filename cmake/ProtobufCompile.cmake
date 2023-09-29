# ProtobufCompile.cmake
#
# Copyright 2022-2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
# Compiles .proto files to C++
#

#-----------------------------------------------------------------------
# The following variables are defined externally:
#
# DOT_PROTO_INSTALL_DIR
#   Directory in which the input .proto files should be installed
# HOST_GRPC_CPP_PLUGIN
#   Path to the host grpc_cpp_plugin executable
# HOST_PROTOC
#   Path to the host protobuf compiler
# PB_HEADER_INSTALL_DIR
#   Directory in which the generated pb.h files should be installed
# PB_OUT_DIR
#   Directory in which the pb.cc and pb.h files should be generated
# PROTO_IMPORT_PATH
#   Directory paths to be searched for input Protobuf files
# STRATUM_SOURCE_DIR
#   Path to the root Stratum source directory
#-----------------------------------------------------------------------

option(INSTALL_PROTO_FILES "Install .proto files" OFF)

#-----------------------------------------------------------------------
# generate_proto_files()
# Generates C++ files for protobufs.
#-----------------------------------------------------------------------
function(generate_proto_files PROTO_FILES SRC_DIR)
  foreach(_file ${PROTO_FILES})
    get_filename_component(_path ${_file} DIRECTORY)
    get_filename_component(_name ${_file} NAME_WE)

    set(_src ${PB_OUT_DIR}/${_path}/${_name}.pb.cc)
    set(_hdr ${PB_OUT_DIR}/${_path}/${_name}.pb.h)

    set_source_files_properties(${_src} ${_hdr} PROPERTIES GENERATED TRUE)

    add_custom_command(
      OUTPUT
        ${_src} ${_hdr}
      COMMAND
        ${HOST_PROTOC}
        --proto_path=${PROTO_IMPORT_PATH}
        --cpp_out=${PB_OUT_DIR}
        -I${STRATUM_SOURCE_DIR}
        ${_file}
      WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS
        ${SRC_DIR}/${_file}
      COMMENT
        "Generating C++ files for ${_file}"
      VERBATIM
    )

    # Install .pb.h file in include/.
    install(
      FILES
        ${_hdr}
      DESTINATION
        ${PB_HEADER_INSTALL_DIR}/${_path}
    )

    # Install .proto file in share/.
    if(INSTALL_PROTO_FILES)
      install(
        FILES
          ${SRC_DIR}/${_file}
        DESTINATION
          ${DOT_PROTO_INSTALL_DIR}/${_path}
      )
    endif()
  endforeach()
endfunction(generate_proto_files)

#-----------------------------------------------------------------------
# generate_grpc_files()
# Generates GRPC C++ files for protobufs.
#-----------------------------------------------------------------------
function(generate_grpc_files PROTO_FILES SRC_DIR)
  foreach(_file ${PROTO_FILES})
    get_filename_component(_path ${_file} DIRECTORY)
    get_filename_component(_name ${_file} NAME_WE)

    set(_src ${PB_OUT_DIR}/${_path}/${_name}.grpc.pb.cc)
    set(_hdr ${PB_OUT_DIR}/${_path}/${_name}.grpc.pb.h)

    set_source_files_properties(${_src} ${_hdr} PROPERTIES GENERATED TRUE)

    add_custom_command(
      OUTPUT
        ${_src} ${_hdr}
      COMMAND
        ${HOST_PROTOC}
        --proto_path=${PROTO_IMPORT_PATH}
        --grpc_out=${PB_OUT_DIR}
        --plugin=protoc-gen-grpc=${HOST_GRPC_CPP_PLUGIN}
        -I${STRATUM_SOURCE_DIR}
        ${_file}
      WORKING_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS
        ${SRC_DIR}/${_file}
      COMMENT
        "Generating grpc files for ${_file}"
      VERBATIM
    )

    # Install .pb.h file in include/.
    install(
      FILES
        ${_hdr}
      DESTINATION
        ${PB_HEADER_INSTALL_DIR}/${_path}
      )

    # Install .proto file in share/.
    if(INSTALL_PROTO_FILES)
      install(
        FILES
          ${SRC_DIR}/${_file}
        DESTINATION
          ${DOT_PROTO_INSTALL_DIR}/${_path}
      )
    endif()
  endforeach()
endfunction(generate_grpc_files)
