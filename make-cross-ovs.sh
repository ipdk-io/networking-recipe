#!/bin/bash
_BUILD_DIR=ovs/build
_INSTALL_DIR=${SDKTARGETSYSROOT}/opt/ovs

rm -fr ${_BUILD_DIR} ${_INSTALL_DIR}

cmake -S ovs -B ${_BUILD_DIR} \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DCMAKE_INSTALL_PREFIX=${_INSTALL_DIR} \
    -DP4OVS=ON -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}

cmake --build ${_BUILD_DIR} -j8 -- V=0    
