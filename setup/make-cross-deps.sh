# Sample script to configure and build the dependency libraries
# on the development host when cross-compiling for the ES2K ACC.

CROSS_BUILD=build
CROSS_INSTALL=${SDKTARGETSYSROOT}/opt/deps

rm -fr ${CROSS_BUILD} ${CROSS_INSTALL}

cmake -S . -B ${CROSS_BUILD} \
    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} \
    -DCMAKE_INSTALL_PREFIX=${CROSS_INSTALL}

cmake --build ${CROSS_BUILD} -j12
