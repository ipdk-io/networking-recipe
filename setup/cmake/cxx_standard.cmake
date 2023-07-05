# Identify default CXX standard
#
# Copyright 2023 Intel Corporation
# SPDX-License-Identifier: Apache-2.0

write_file(
      ${CMAKE_CURRENT_BINARY_DIR}/cmake/tmp-cxx-standard.cpp
      "#include <iostream>\nint main(){std::cout << __cplusplus << std::endl;return 0;}"
    )

    try_run(
      _cxx_standard_run_status _cxx_standard_build_status
      ${CMAKE_CURRENT_BINARY_DIR}
      ${CMAKE_CURRENT_BINARY_DIR}/cmake/tmp-cxx-standard.cpp
      RUN_OUTPUT_VARIABLE _COMPILER_DEFAULT_CXX_STANDARD)
    if(_cxx_standard_run_status EQUAL FAILED_TO_RUN
       OR NOT _cxx_standard_build_status)
      message(
        WARNING
          "Failed to build or run the script to get the _COMPILER_DEFAULT_CXX_STANDARD value from current compiler. Setting _COMPILER_DEFAULT_CXX_STANDARD to 201103"
      )
      set(_COMPILER_DEFAULT_CXX_STANDARD "201103")
    endif()

    string(STRIP "${_COMPILER_DEFAULT_CXX_STANDARD}"
                 _COMPILER_DEFAULT_CXX_STANDARD)

if(DEFINED CMAKE_CXX_STANDARD)
      set(_CXX_CURRENT_STANDARD ${CMAKE_CXX_STANDARD})
elseif(DEFINED _COMPILER_DEFAULT_CXX_STANDARD)
      # refer https://en.cppreference.com/w/cpp/preprocessor/replace#Predefined_macros
      # for constants
      if(_COMPILER_DEFAULT_CXX_STANDARD EQUAL 201103)
        set(_CXX_CURRENT_STANDARD 11)
      elseif(_COMPILER_DEFAULT_CXX_STANDARD EQUAL 201402)
        set(_CXX_CURRENT_STANDARD 14)
      elseif(_COMPILER_DEFAULT_CXX_STANDARD EQUAL 201703)
        set(_CXX_CURRENT_STANDARD 17)
      endif()
endif()
