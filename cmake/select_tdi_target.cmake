# select_tdi_target.cmake
#
# Copyright 2022-2023 Intel Corporation
# SPDX-License-Identifier: Apache 2.0
#
# Processes the new TDI_TARGET string option and the legacy Boolean target
# options (DPDK_TARGET, ES2K_TARGET, and TOFINO_TARGET).
#
# Issues deprecation messages for legacy target options if they are enabled.
#
# Sets TARGETTYPE, TARGETFLAG, and the corresponding Boolean target option.
# Unsets all other Boolean target options.

# ----------------------------------------------------------------------
#
# The process is:
#
# - Check for each of the legacy xxxx_TARGET Boolean options. Create a list
#   of targets we've seen and unset the xxxx_TARGET options.
#
# - Process the TDI_TARGET=<target> option. Check for errors.
#
# - Determine target to enable.
#
# - Define the following variables:
#
#   TARGETFLAG    string specifying the name of the preprocessor variable
#                 (xxxx_TARGET) to be defined.
#
#   TARGETTYPE    string specifying the target type (DPDK, ES2K, etc.)
#
#   xxxx_TARGET   Boolean to enable target-specific cmake code.
#
# ----------------------------------------------------------------------

set(TDI_TARGET "" CACHE STRING "TDI Target Type")

# Valid TDI targets.
# Add to this list to define new TDI target types.
set(_valid_tdi_targets DPDK ES2K TOFINO)

# Default TDI target type.
set(_default_tdi_target DPDK)

#-----------------------------------------------------------------------
# Checks for the specified legacy target option.
#-----------------------------------------------------------------------
function(_check_legacy_target_option typename TARGETVAR)
    unset(${TARGETVAR} PARENT_SCOPE)
    set(varname ${typename}_TARGET)
    if(${varname})
        # Progression: NOTICE (accept), WARNING (accept), FATAL_ERROR (abort).
        message(NOTICE "${varname} option is deprecated; use TDI_TARGET=${typename} instead.")
        set(${TARGETVAR} ${typename} PARENT_SCOPE)
    endif()
    unset(${varname} PARENT_SCOPE)
    unset(${varname} CACHE)
endfunction()

#-----------------------------------------------------------------------
# Checks for the legacy DPDK_TARGET, ES2K_TARGET, and TOFINO_TARGET
# options.
#-----------------------------------------------------------------------
function(_get_legacy_target_option TARGETVAR)
    set(legacy_targets)

    _check_legacy_target_option(DPDK target_name)
    if(target_name)
        list(APPEND legacy_targets ${target_name})
    endif()

    _check_legacy_target_option(ES2K target_name)
    if(target_name)
        list(APPEND legacy_targets ${target_name})
    endif()

    _check_legacy_target_option(TOFINO target_name)
    if(target_name)
        list(APPEND legacy_targets ${target_name})
    endif()

    list(LENGTH legacy_targets num_targets)

    if(num_targets GREATER 1)
        message(FATAL_ERROR "More than one legacy target option specified!")
    elseif(num_targets EQUAL 1)
        set(${TARGETVAR} ${legacy_targets} PARENT_SCOPE)
    endif()
endfunction()

#-----------------------------------------------------------------------
# Checks for the TDI_TARGET=<target> option.
#-----------------------------------------------------------------------
function(_get_tdi_target_option TARGETVAR)
    string(STRIP "${TDI_TARGET}" tdi_target)
    string(TOUPPER "${tdi_target}" tdi_target)
    if(NOT tdi_target STREQUAL "")
        list(FIND _valid_tdi_targets ${tdi_target} index)
        if(index EQUAL -1)
            message(FATAL_ERROR "'${tdi_target}' is not a valid target!")
        endif()
        set(${TARGETVAR} ${tdi_target} PARENT_SCOPE)
    endif()
endfunction()

#-----------------------------------------------------------------------
# Selects the TDI target type.
# Sets the TARGETFLAG and TARGETTYPE cache variables.
#-----------------------------------------------------------------------
function(_select_tdi_target_type)
    _get_legacy_target_option(legacy_target)
    _get_tdi_target_option(target_param)

    if(DEFINED legacy_target AND NOT legacy_target STREQUAL "")
        if(DEFINED target_param AND NOT target_param STREQUAL "")
            set(param1 "${legacy_target}_TARGET")
            set(param2 "TDI_TARGET=${target_param}")
            message(FATAL_ERROR "${param1} and ${param2} both specified!")
        endif()
    endif()

    set(target_action "Building")

    if(target_param)
        set(target_type ${target_param})
    elseif(legacy_target)
        set(target_type ${legacy_target})
    else()
        set(target_type ${_default_tdi_target})
        set(target_action "Defaulting to")
    endif()

    set(target_flag "${target_type}_TARGET")

    # Set cache variables
    set(TARGETFLAG "${target_flag}" CACHE STRING "TDI target conditional" FORCE)
    set(TARGETTYPE "${target_type}" CACHE STRING "TDI target type" FORCE)

    message(NOTICE "${target_action} ${target_flag}")
endfunction()

# Set TARGETFLAG and TARGETTYPE
_select_tdi_target_type()

# Define/enable target variable (e.g. DPDK_TARGET)
set(${TARGETFLAG} ON)

unset(_default_tdi_target)
unset(_valid_tdi_targets)

