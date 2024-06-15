# CMake build file for ovs-p4rt/ovs
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

include(CheckBinutilsAVX512)
include(CheckCCompilerFlag)
include(CheckLibraryExists)

check_library_exists(rt aio_write /usr/lib/x86_64-linux-gnu HAVE_POSIX_AIO)
check_c_compiler_flag(-mavx512f HAVE_AVX512F)
check_binutils_avx512(HAVE_LD_AVX512_GOOD)

# Generate config.h file and add it to the include path.
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
               "${CMAKE_CURRENT_BINARY_DIR}/config.h")

find_package(Unbound)
find_package(Unwind)

# =====================
# Set common properties
# =====================

function(set_ovs_target_properties ARTIFACT)
  if(TARGET ovs::avx512)
      target_link_libraries(${ARTIFACT} PUBLIC ovs::avx512)
  endif()

  if(TARGET unbound)
      target_link_libraries(${ARTIFACT} PUBLIC unbound)
  endif()

  if(TARGET unwind::unwind)
      target_link_libraries(${ARTIFACT} PUBLIC unwind::unwind)
  endif()


  if(ES2K_TARGET)
      target_link_libraries(${ARTIFACT} PUBLIC ovsp4rt_logging)
  endif()

  target_link_libraries(${ARTIFACT} PUBLIC
      absl::statusor
      absl::flags_private_handle_accessor
      absl::flags
  )

  target_link_libraries(${ARTIFACT} PUBLIC
      stratum_static
      p4_role_config_proto
      stratum_proto
      p4runtime_proto
  )
endfunction()

# =================
# Build executables
# =================

include(vswitchd.cmake)
include(testcontroller.cmake)
