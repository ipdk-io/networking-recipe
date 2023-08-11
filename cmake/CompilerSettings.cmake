# CompilerSettings.cmake - Apply recommended compiler settings.
#
# Copyright 2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#

include(CheckCCompilerFlag)
include(CheckPIESupported)
include(CMakePrintHelpers)

# Disabled by default, on the assumption that the choice of warnings
# is best left to the application.
option(ENABLE_WARNING_SETTINGS "Enable compiler warnings" OFF)
option(ENABLE_SPECTRE_SETTINGS "Enable Spectre mitigations" ON)

function(add_basic_settings)
  # Compiler flags
  add_compile_options("-pipe")
  add_compile_options("-feliminate-unused-debug-types")

  # Linker flags
  add_link_options("-Wl,-O1")
  add_link_options("-Wl,--hash-style=gnu")
  add_link_options("-Wl,--as-needed")
  add_link_options("-Wl,-z,now")
endfunction(add_basic_settings)

# Defines the security settings used in earlier versions
# of the software.
function(add_legacy_security_settings)
  # Format String Defense
  add_compile_options("-Wformat")
  add_compile_options("-Wformat-security")
  add_compile_options("-Werror=format-security")

  # Position Independent Code
  set(CMAKE_POSITION_INDEPENDENT_CODE TRUE PARENT_SCOPE)

  # Preprocessor Macros
  add_compile_options("-D_FORTIFY_SOURCE=2")

  # Read-only Relocation
  add_link_options("-Wl,-z,relro")

  # Stack Protection
  add_compile_options("-fstack-protector-strong")
endfunction()

# Defines security settings according to the
# Intel Secure Coding Standards.
function(add_recent_security_settings CONFIG)
  string(TOUPPER ${CONFIG} CONFIG)
  if(CONFIG STREQUAL "DEBUG")
    set(IS_RELEASE FALSE)
  else()
    set(IS_RELEASE TRUE)
  endif()

  macro(check_and_add_option OPTION HAVE_FLAG)
    check_c_compiler_flag(${OPTION} ${HAVE_FLAG})
    if(${HAVE_FLAG})
      add_compile_options(${OPTION})
    endif()
  endmacro()

  # Compiler Warnings and Error Detection
  if(ENABLE_WARNING_SETTINGS)
    add_compile_options("-Wall")
    add_compile_options("-Wextra")
    if(IS_RELEASE)
      add_compile_options("-Werror")
    endif()
  endif()

  # Control Flow Integrity
  check_and_add_option("-fsanitize=cfi" HAVE_SANITIZE_CFI)
  if(IS_RELEASE)
    check_and_add_option("-flto" HAVE_LTO)
    check_and_add_option("-fvisibility=hidden" HAVE_VISIBILITY_HIDDEN)
  endif()

  # Format String Defense
  check_and_add_option("-Wformat" HAVE_WFORMAT)
  check_and_add_option("-Wformat-security" HAVE_WFORMAT_SECURITY)
  if(IS_RELEASE)
    check_and_add_option(
        "-Werror=format-security" FLAGS_WERROR_FORMAT_SECURITY)
  endif()

  # Inexecutable Stack
  check_and_add_option("-Wl,-z,noexecstack" HAVE_NOEXECSTACK)

  # Position Independent Code
  set(CMAKE_POSITION_INDEPENDENT_CODE TRUE PARENT_SCOPE)

  # Position Independent Execution
  check_pie_supported(LANGUAGES C CXX)
  if(CMAKE_C_PIE_SUPPORTED AND CMAKE_CXX_PIE_SUPPORTED)
    add_compile_options("-fPIE -pie")
  endif()

  # Preprocessor Macros
  add_compile_definitions("FORTIFY_SOURCE=2")

  # Read-only Relocation
  check_and_add_option("-Wl,-z,relro" HAVE_RELRO)

  # Stack Protection
  add_compile_options("-fstack-protector-strong")

  # Spectre Protection
  if(IS_RELEASE AND ENABLE_SPECTRE_SETTINGS)
    # Mitigating Bounds Check Bypass (Spectre Variant 1)
    check_and_add_option(
        "-mconditional-branch=keep" HAVE_COND_BRANCH_KEEP)
    check_and_add_option(
        "-mconditional-branch=pattern-report" HAVE_COND_BRANCH_PATTERN_REPORT)
    check_and_add_option(
        "-mconditional-branch=pattern-fix" HAVE_COND_BRANCH_PATTERN_FIX)

    # Mitigating Branch Target Injection (Spectre Variant 2)
    check_and_add_option("-mretpoline" HAVE_RETPOLINE)
  endif()
endfunction(add_recent_security_settings)
