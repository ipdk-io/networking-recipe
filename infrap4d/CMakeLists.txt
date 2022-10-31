cmake_minimum_required(VERSION 3.12)

project(infrap4d VERSION 0.1.0 LANGUAGES C CXX)

set(STRATUM_TDI_BIN_DIR ${STRATUM_SOURCE_DIR}/stratum/hal/bin/tdi)
set(STRATUM_BFRT_BIN_DIR ${STRATUM_SOURCE_DIR}/stratum/hal/bin/barefoot)

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

if(DPDK_TARGET)
    # Executable using own libraries, SDE libraries, DPDK libraries,
    # and dependency libraries
    set(_infrap4d_rpath "$ORIGIN/../lib" "${SDE_INSTALL_DIR}/lib"
        "${DPDK_LIBRARY_DIRS}")
    if(DEPEND_INSTALL_DIR)
        list(APPEND _infrap4d_rpath "${DEPEND_INSTALL_DIR}/lib")
    endif()
    list(JOIN _infrap4d_rpath ":" _infrap4d_rpath)
else()
    # Executable using own libraries, SDE, and dependencies
    set(_infrap4d_rpath ${EXEC_ORIGIN_SDE_AND_DEPS})
  endif()

set_target_properties(infrap4d PROPERTIES
    INSTALL_RPATH ${_infrap4d_rpath}
    INSTALL_RPATH_USE_LINK_PATH FALSE
)

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
