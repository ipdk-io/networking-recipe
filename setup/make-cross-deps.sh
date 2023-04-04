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
_JOBS=8
_PREFIX=//opt/deps
_TOOLFILE=${CMAKE_TOOLCHAIN_FILE}

# Displays help text
print_help() {
    echo ""
    echo "Build target dependency libraries"
    echo ""
    echo "Options:"
    echo "  --build=DIR      -B  Build directory path [${_BLD_DIR}]"
    echo "  --cxx=VERSION    -c  CXX_STANDARD to build dependencies (Default: empty)"
    echo "  --dry-run        -n  Display cmake parameters and exit"
    echo "  --force          -f  Specify -f when patching (Default: false)"
    echo "  --jobs=NJOBS     -j  Number of build threads (Default: ${_JOBS})"
    echo "  --no-download        Do not download repositories (Default: false)"
    echo "  --prefix=DIR*    -P  Install directory prefix [${_PREFIX}]"
    echo "  --sudo               Use sudo when installing (Default: false)"
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
SHORTOPTS=B:P:T:hj:n
LONGOPTS=build:,cxx:,dry-run,force,jobs:,help,no-download,prefix:,sudo,toolchain:

eval set -- `getopt -o ${SHORTOPTS} --long ${LONGOPTS} -- "$@"`

while true ; do
    case "$1" in
    -B|--build)
        _BLD_DIR=$2
        shift 2 ;;
    --cxx)
        _CXX_STANDARD_OPTION="-DCXX_STANDARD=$2"
        shift 2 ;;
    -h|--help)
        print_help
        exit 99 ;;
    -j|--jobs)
        _JOBS=$2
        shift 2 ;;
    -n|--dry-run)
        _DRY_RUN=true
        shift 1 ;;
    -f|--force)
        _FORCE_PATCH="-DFORCE_PATCH=TRUE"
        shift 1 ;;
    --no-download)
        _DOWNLOAD="-DDOWNLOAD=FALSE"
        shift 1 ;;
    -P|--prefix)
        _PREFIX=$2
        shift 2 ;;
    --sudo)
	_USE_SUDO="-DUSE_SUDO=TRUE"
	shift ;;
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
    echo "JOBS=${_JOBS}"
    [ -n "${_CXX_STANDARD_OPTION}" ] && echo "${_CXX_STANDARD_OPTION:2}"
    [ -n "${_DOWNLOAD}" ] && echo "${_DOWNLOAD:2}"
    [ -n "${_FORCE_PATCH}" ] && echo "${_FORCE_PATCH:2}"
    [ -n "${_USE_SUDO}" ] && echo "${_USE_SUDO:2}"
    echo ""
    exit 0
fi

rm -fr ${_BLD_DIR}

cmake -S . -B ${_BLD_DIR} \
    -DCMAKE_INSTALL_PREFIX=${_PREFIX} \
    -DCMAKE_TOOLCHAIN_FILE=${_TOOLFILE} \
    ${_CXX_STANDARD_OPTION} \
    ${_DOWNLOAD} ${_FORCE_PATCH} ${_USE_SUDO}

cmake --build ${_BLD_DIR} -j${_JOBS}
