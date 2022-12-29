# p4ovs/cmake/CheckBinUtilsAVX512.cmake
#
# Copyright (c) 2022 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
# Checks for a known issue with AVX512 in binutils.
#   GCC: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=90028
# Adapted from the OVS_CHECK_BINUTILS_AVX512 macro in OvS.
#
function(check_binutils_avx512 STATUS_VAR)
    set(status_var FALSE)

    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
        set(objfile "${CMAKE_BINARY_DIR}/check_avx512.o")
        set(params "0x8(,%ymm1,1),%ymm0{%k2}")

        # Generate test program and pipe it to the assembler.
        execute_process(
            COMMAND
                # Output is piped to next command
                echo "vpgatherqq ${params}"
            COMMAND
                as --64 -o ${objfile} -
            RESULT_VARIABLE
                result_value
            OUTPUT_VARIABLE
                stdout_value
            ERROR_VARIABLE
                stderr_value
        )

        # Run objdump on assembler output and check results for bug.
        execute_process(
            COMMAND
                objdump -d --no-show-raw-insn ${objfile}
            COMMAND
                grep -q "${params}"
            RESULT_VARIABLE
                result_value
            OUTPUT_VARIABLE
                stdout_value
            ERROR_VARIABLE
                stderr_value
        )

        # Result code will be zero if grep found a match.
        if(${result_value} STREQUAL "0")
            set(status_var TRUE)
        endif()

        file(REMOVE ${objfile})
    endif()

    set(${STATUS_VAR} ${status_var} PARENT_SCOPE)
endfunction()
