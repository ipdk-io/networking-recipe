# Configures CMake for a DPDK build.
# Usage: cmake -B <build-dir> -C dpdk.cmake

set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install" CACHE PATH "p4cp install directory")
set(DEPEND_INSTALL_DIR "$ENV{DEPEND_INSTALL}" CACHE PATH "stratum-deps install directory")
set(OVS_INSTALL_DIR "${CMAKE_SOURCE_DIR}/ovs/install" CACHE PATH "ovs install directory")
set(SDE_INSTALL_DIR "$ENV{SDE_INSTALL}" CACHE PATH "SDE install directory")
set(SET_RPATH YES CACHE BOOL "Whether to set RPATH in binary artifacts")
set(TDI_TARGET "DPDK" CACHE STRING "TDI target to build")
