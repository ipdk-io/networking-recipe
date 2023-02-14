#!/bin/bash

# Sample script to configure and build the dependency libraries
# on the development host when cross-compiling for the ES2K ACC.

if [ -z "${SDKTARGETSYSROOT}" ]; then
    echo ""
    echo "-----------------------------------------------------"
    echo "Error: SDKTARGETSYSROOT is not defined!"
    echo "Did you forget to source the environment variables?"
    echo "-----------------------------------------------------"
    echo ""
    exit 1
fi

_SYSROOT=${SDKTARGETSYSROOT}

# Default values
_BLD_DIR=build
_DRY_RUN=false
_PREFIX=//opt/deps
_TOOLFILE=${CMAKE_TOOLCHAIN_FILE}

# Displays help text
print_help() {
    echo ""
    echo "Build target dependency libraries"
    echo ""
    echo "Options:"
    echo "  --build=DIR      -B  Build directory path [${_BLD_DIR}]"
    echo "  --dry-run        -n  Display cmake parameters and exit"
    echo "  --prefix=DIR*    -P  Install directory prefix [${_PREFIX}]"
    echo "  --toolchain=FILE -T  CMake toolchain file"
    echo ""
    echo "* '//' at the beginning of the directory path will be replaced"
    echo "  with the sysroot directory path."
    echo ""
    echo "Environment variables:"
    echo "  CMAKE_TOOLCHAIN_FILE - Default toolchain file"
    echo "  SDKTARGETSYSROOT - sysroot directory"
    echo ""
}

# Parse options
SHORTOPTS=B:P:T:hn
LONGOPTS=build:,dry-run,help,prefix:,toolchain:

eval set -- `getopt -o ${SHORTOPTS} --long ${LONGOPTS} -- "$@"`

while true ; do
    case "$1" in
    -B|--build)
        _BLD_DIR=$2
        shift 2 ;;
    -h|--help)
        print_help
        exit 99 ;;
    -n|--dry-run)
        _DRY_RUN=true
        shift 1 ;;
    -P|--prefix)
        _PREFIX=$2
        shift 2 ;;
    -T|--toolchain)
        _TOOLFILE=$2
        shift 2 ;;
    --)
        shift
        break ;;
    *)
        echo "Invalid parameter: $1"
        exit 1 ;;
    esac
done

# Replace "//"" prefix with "${_SYSROOT}/""
[ "${_PREFIX:0:2}" = "//" ] && _PREFIX=${_SYSROOT}/${_PREFIX:2}

if [ "${_DRY_RUN}" = "true" ]; then
    echo ""
    echo "CMAKE_INSTALL_PREFIX=${_PREFIX}"
    echo "CMAKE_TOOLCHAIN_FILE=${_TOOLFILE}"
    echo ""
    exit 0
fi

rm -fr ${_BLD_DIR} ${_PREFIX}

cmake -S . -B ${_BLD_DIR} \
    -DCMAKE_INSTALL_PREFIX=${_PREFIX} \
    -DCMAKE_TOOLCHAIN_FILE=${_TOOLFILE}

cmake --build ${_BLD_DIR} -j8
