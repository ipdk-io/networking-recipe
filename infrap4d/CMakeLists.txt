cmake_minimum_required(VERSION 3.12)

project(infrap4d VERSION 0.1.0 LANGUAGES C CXX)

set(STRATUM_TDI_BIN_DIR ${STRATUM_SOURCE_DIR}/stratum/hal/bin/tdi)

if(DPDK_TARGET)
    set(INFRAP4D_MAIN ${STRATUM_TDI_BIN_DIR}/dpdk/main.cc)
elseif(TOFINO_TARGET)
    set(INFRAP4D_MAIN ${STRATUM_TDI_BIN_DIR}/tofino/main.cc)
endif()

add_executable(infrap4d
    ${INFRAP4D_MAIN}
)

target_include_directories(infrap4d PRIVATE ${STRATUM_SOURCE_DIR})

target_link_libraries(infrap4d PRIVATE
    -Wl,--whole-archive
    stratum
    -Wl,--no-whole-archive
)

install(TARGETS infrap4d DESTINATION sbin)
