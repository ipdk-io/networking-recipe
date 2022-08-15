#!/bin/bash

# Parse command-line options.
SHORTOPTS=p:
LONGOPTS=prefix:,sde-prefix:,dep-prefix:

GETOPTS=`getopt -o ${SHORTOPTS} --long ${LONGOPTS} -- "$@"`
eval set -- "${GETOPTS}"

# Set defaults.
PREFIX="install"

# Process command-line options.
while true ; do
    case "$1" in
    -p|--prefix)
	PREFIX=$2
	shift 2 ;;
    --dep-prefix)
	DEPEND_INSTALL=$2
	shift 2 ;;
    --sde-prefix)
	SDE_INSTALL=$2
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

# Do a clean build of the of the recipe.
# This preserves the private OVS build tree if there is one.
rm -fr build install

# Build OVS with a private install tree.
cmake -S ovs -B ovs/build -DOVS_INSTALL_PREFIX=ovs/install
cmake --build ovs/build -j6 -- V=0

# Build the rest of the recipe using the private OVS install tree.
# This allows us to do a clean build without having to remake OVS.
cmake -S . -B build \
    -DCMAKE_INSTALL_PREFIX=${PREFIX} \
    -DOVS_INSTALL_PREFIX=ovs/install \
    -DSDE_INSTALL_PREFIX=${SDE_INSTALL} \
    -DDEPEND_INSTALL_PREFIX=${DEPEND_INSTALL}

cmake --build build -j6
cmake --install build
