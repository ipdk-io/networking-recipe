cmake_minimum_required(VERSION 3.12)

project(infrap4d VERSION 0.1.0 LANGUAGES C CXX)

set(STRATUM_TDI_BIN_DIR ${STRATUM_SOURCE_DIR}/stratum/hal/bin/tdi)
set(STRATUM_BFRT_BIN_DIR ${STRATUM_SOURCE_DIR}/stratum/hal/bin/barefoot)

add_executable(infrap4d infrap4d_main.cc)

target_include_directories(infrap4d PRIVATE 
    ${STRATUM_SOURCE_DIR}
    ${KRNLMON_SOURCE_DIR})

target_link_libraries(infrap4d PRIVATE
    -Wl,--whole-archive
    stratum
    -Wl,--no-whole-archive
    krnlmon
    pthread
)

add_dependencies(infrap4d stratum krnlmon)

install(TARGETS infrap4d DESTINATION sbin)

if(DPDK_TARGET)
    install(
        FILES
            ${STRATUM_TDI_BIN_DIR}/dpdk/dpdk_port_config.pb.txt
            ${STRATUM_TDI_BIN_DIR}/dpdk/dpdk_skip_p4.conf
        DESTINATION
            ${CMAKE_INSTALL_PREFIX}/share/stratum/dpdk
    )
elseif(TOFINO_TARGET)
  install(
      FILES
          ${STRATUM_BFRT_BIN_DIR}/tofino_skip_p4_no_bsp.conf
          ${STRATUM_BFRT_BIN_DIR}/tofino_skip_p4.conf
      DESTINATION
          ${CMAKE_INSTALL_PREFIX}/share/stratum
  )
endif()
