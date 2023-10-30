# CompilerSettings.cmake
#
# Copyright 2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
# Apply recommended compiler settings.
#

include(CheckCCompilerFlag)
include(CheckPIESupported)
include(CMakePrintHelpers)

# Disabled by default, on the assumption that the choice of warnings
# is best left to the application.
option(ENABLE_WARNING_SETTINGS "Enable compiler warnings" OFF)
option(ENABLE_SPECTRE_SETTINGS "Enable Spectre mitigations" OFF)
mark_as_advanced(ENABLE_WARNING_SETTINGS)
mark_as_advanced(ENABLE_SPECTRE_SETTINGS)

macro(check_and_add_compile_option _option _have_flag)
  check_c_compiler_flag(${_option} ${_have_flag})
  if(${_have_flag})
    add_compile_options(${_option})
  endif()
endmacro()

function(set_basic_compiler_options)
  # Compiler flags
  add_compile_options("-pipe")
  add_compile_options("-feliminate-unused-debug-types")

  # Linker flags
  add_link_options("-Wl,-O1")
  add_link_options("-Wl,--hash-style=gnu")
  add_link_options("-Wl,--as-needed")
  add_link_options("-Wl,-z,now")
endfunction(set_basic_compiler_options)

# Enables the security settings used in earlier versions
# of the software.
function(set_legacy_security_options)
  # Format String Defense
  add_compile_options("-Wformat")
  add_compile_options("-Wformat-security")
  add_compile_options("-Werror=format-security")

  # Position Independent Code
  set(CMAKE_POSITION_INDEPENDENT_CODE TRUE PARENT_SCOPE)

  # Preprocessor Macros
  add_compile_definitions("_FORTIFY_SOURCE=2")

  # Read-only Relocation
  add_link_options("-Wl,-z,relro")

  # Stack Protection
  add_compile_options("-fstack-protector-strong")
endfunction(set_legacy_security_options)

# Enables additional security settings in accordance with the
# Intel Secure Coding Standards.
function(set_extended_security_options)
  string(TOUPPER "${CMAKE_BUILD_TYPE}" _build_type)
  if(_build_type STREQUAL "DEBUG")
    set(_is_release FALSE)
  else()
    set(_is_release TRUE)
  endif()

  # Compiler Warnings and Error Detection
  if(ENABLE_WARNING_SETTINGS)
    add_compile_options("-Wall")
    add_compile_options("-Wextra")
    if(_is_release)
      add_compile_options("-Werror")
    endif()
  endif()

  # Control Flow Integrity
  check_and_add_compile_option("-fsanitize=cfi" HAVE_SANITIZE_CFI)
  if(_is_release)
    check_and_add_compile_option("-flto" HAVE_LTO)
    check_and_add_compile_option("-fvisibility=hidden" HAVE_VISIBILITY_HIDDEN)
  endif()

  # Inexecutable Stack
  check_and_add_compile_option("-Wl,-z,noexecstack" HAVE_NOEXECSTACK)

  # Position Independent Execution
  check_pie_supported(LANGUAGES C CXX)
  if(CMAKE_C_PIE_SUPPORTED AND CMAKE_CXX_PIE_SUPPORTED)
    add_compile_options("-fPIE -pie")
  endif()

  # Spectre Protection
  if(_is_release AND ENABLE_SPECTRE_SETTINGS)
    # Mitigating Bounds Check Bypass (Spectre Variant 1)
    check_and_add_compile_option(
        "-mconditional-branch=keep" HAVE_COND_BRANCH_KEEP)
    check_and_add_compile_option(
        "-mconditional-branch=pattern-report" HAVE_COND_BRANCH_PATTERN_REPORT)
    check_and_add_compile_option(
        "-mconditional-branch=pattern-fix" HAVE_COND_BRANCH_PATTERN_FIX)

    # Mitigating Branch Target Injection (Spectre Variant 2)
    check_and_add_compile_option("-mretpoline" HAVE_RETPOLINE)
  endif()
endfunction(set_extended_security_options)
