rm -fr build install

cmake -S . -B build -DOVS_INSTALL_PREFIX=install
cmake --build build -j6 -- V=0
