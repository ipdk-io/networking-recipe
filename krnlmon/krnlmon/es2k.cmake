# CMake build configuration for DPDK target
set(TDI_TARGET "es2k" CACHE STRING "config: TDI target type")

set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install" CACHE STRING
    "config: install directory")

set(SET_RPATH TRUE CACHE BOOL "config: set RPATH in binaries")
