cmake_minimum_required(VERSION 3.12)

project(infrap4d VERSION 0.1.0 LANGUAGES C CXX)

if(DPDK_TARGET)
    set(INFRAP4D_MAIN $<TARGET_OBJECTS:stratum_dpdk_o>)
elseif(TOFINO_TARGET)
    set(INFRAP4D_MAIN $<TARGET_OBJECTS:stratum_tofino_o>)
endif()

add_executable(infrap4d
    ${INFRAP4D_MAIN}
)

target_include_directories(infrap4d PRIVATE ${STRATUM_SOURCE_DIR})

target_link_libraries(infrap4d PRIVATE
    -Wl,--whole-archive
    stratum_hal_lib_tdi
    -Wl,--no-whole-archive
)

install(TARGETS infrap4d DESTINATION sbin)
