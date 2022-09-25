#!/bin/bash

# Parse command-line options.
SHORTOPTS=p:
LONGOPTS=clean,prefix:,sde-install:,dep-install:,develop,target:

GETOPTS=`getopt -o ${SHORTOPTS} --long ${LONGOPTS} -- "$@"`
eval set -- "${GETOPTS}"

# Set defaults.
PREFIX="install"
TARGET="TOFINO"
DEVELOP=0

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
    --develop)
        DEVELOP=1
        shift ;;
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

if [ ${DEVELOP} -ne 0 ]; then
    OVS_BUILD=ovs/build
    OVS_INSTALL=ovs/install
else
    OVS_BUILD=build/ovs
    OVS_INSTALL=${PREFIX}
fi

# Build OVS first.
cmake -S ovs -B ${OVS_BUILD} -DCMAKE_INSTALL_PREFIX=${OVS_INSTALL}
cmake --build ${OVS_BUILD} -j6 -- V=0

# Build the rest of the recipe.
cmake -S . -B build \
    -DCMAKE_INSTALL_PREFIX=${PREFIX} \
    -DOVS_INSTALL_PREFIX=${OVS_INSTALL} \
    -DSDE_INSTALL_PREFIX=${SDE_INSTALL} \
    ${DEPEND_INSTALL_OPTION} \
    -D${TARGET}_TARGET=ON

cmake --build build -j6
cmake --install build
