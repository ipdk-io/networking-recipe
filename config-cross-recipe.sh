#!/bin/bash

# Sample script to configure the non-ovs portion of the networking
# recipe to cross-compile for the ES2K ACC platform.

if [ -z "${SDKTARGETSYSROOT}" ]; then
    echo ""
    echo "Error: SDKTARGETSYSROOT is not defined!"
    echo "Did you forget to source the environment variables?"
    echo ""
    exit 1
fi

_BUILD_DIR=build
_INSTALL_DIR=install

_OPT_DIR=${SDKTARGETSYSROOT}/opt

rm -fr ${_BUILD_DIR} ${_INSTALL_DIR}

cmake -S . -B ${_BUILD_DIR} \
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DDEPEND_INSTALL_DIR=${_OPT_DIR}/deps \
    -DOVS_INSTALL_DIR=${_OPT_DIR}/ovs \
    -DSDE_INSTALL_DIR=${_OPT_DIR}/p4sde \
    -DHOST_DEPEND_DIR=setup/host-deps \
    -DCMAKE_INSTALL_PREFIX=${_INSTALL_DIR} \
    -DSET_RPATH=TRUE \
    -DES2K_TARGET=ON

#cmake --build ${_BUILD_DIR} -j8 --target install
