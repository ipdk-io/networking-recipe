#!/bin/bash

# Parse command-line options.
SHORTOPTS=p:
LONGOPTS=clean,prefix:,sde-install:,dep-install:,target:

GETOPTS=`getopt -o ${SHORTOPTS} --long ${LONGOPTS} -- "$@"`
eval set -- "${GETOPTS}"

# Set defaults.
PREFIX="install"
TARGET="TOFINO"

# Process command-line options.
while true ; do
    case "$1" in
    --clean)
        rm -fr build install
        shift 1;;
    -p|--prefix)
        PREFIX=$2
        shift 2 ;;
    --dep-install)
        DEPEND_INSTALL=$2
        shift 2 ;;
    --sde-install)
        SDE_INSTALL=$2
        shift 2 ;;
    --target)
        # convert to uppercase
        TARGET=${2^^}
        shift 2 ;;
    --)
        shift
        break ;;
    *)
        echo "Internal error!"
        exit 1 ;;
    esac
done

if [ -z "${SDE_INSTALL}" ]; then
    echo "ERROR: SDE_INSTALL not defined!"
    exit 1
fi

if [ -n "${DEPEND_INSTALL}" ]; then
    DEPEND_INSTALL_OPTION=-DDEPEND_INSTALL_PREFIX=${DEPEND_INSTALL}
fi

# Build OVS first.
cmake -S ovs -B build/ovs -DOVS_INSTALL_PREFIX=${PREFIX}
cmake --build build/ovs -j6 -- V=0

# Build the rest of the recipe.
cmake -S . -B build \
    -DCMAKE_INSTALL_PREFIX=${PREFIX} \
    -DSDE_INSTALL_PREFIX=${SDE_INSTALL} \
    ${DEPEND_INSTALL_OPTION} \
    -D${TARGET}_TARGET=ON

cmake --build build -j6
cmake --install build
