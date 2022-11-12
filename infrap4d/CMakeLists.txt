cmake_minimum_required(VERSION 3.12)

project(infrap4d VERSION 0.1.0 LANGUAGES C CXX)

set(STRATUM_TDI_BIN_DIR ${STRATUM_SOURCE_DIR}/stratum/hal/bin/tdi)

add_executable(infrap4d
    $<$<BOOL:${TOFINO_TARGET}>:${STRATUM_TDI_BIN_DIR}/tofino/main.cc>
    $<$<BOOL:${TOFINO_TARGET}>:$<TARGET_OBJECTS:stratum_tofino_o>>

    $<$<BOOL:${DPDK_TARGET}>:${STRATUM_TDI_BIN_DIR}/dpdk/main.cc>
    $<$<BOOL:${DPDK_TARGET}>:$<TARGET_OBJECTS:stratum_dpdk_o>>
)

target_include_directories(infrap4d PRIVATE ${STRATUM_SOURCE_DIR})

# stratum built as static library
target_link_libraries(infrap4d PRIVATE
    -Wl,--whole-archive
    stratum_tdi
    -Wl,--no-whole-archive
)

install(TARGETS infrap4d DESTINATION sbin)
