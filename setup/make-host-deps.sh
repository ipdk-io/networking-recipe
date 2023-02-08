#!/bin/bash

# Sample script to configure and build the subset of the dependency
# libraries needed to compile the Protobuf files on the development
# host when cross-compiling for the ES2K ACC platform.

# The Protobuf compiler and grpc plugins run natively on the development
# host. They generate C++ source and header files that can be compiled
# to run natively on the host or cross-compiled to run on the target
# platform.

if [ -n "${SDKTARGETSYSROOT}" ]; then
    echo ""
    echo "-----------------------------------------------------"
    echo "Error: SDKTARGETSYSROOT is defined!"
    echo "The host dependencies must be built WITHOUT sourcing"
    echo "the cross-compile environment variables."
    echo "-----------------------------------------------------"
    echo ""
    exit 1
fi
HOST_BUILD=build
HOST_INSTALL=host-deps

rm -fr ${HOST_BUILD} ${HOST_INSTALL}

cmake -S . -B ${HOST_BUILD} \
    -DCMAKE_INSTALL_PREFIX=${HOST_INSTALL} \
    -DON_DEMAND=TRUE

cmake --build ${HOST_BUILD} -j8 --target grpc
