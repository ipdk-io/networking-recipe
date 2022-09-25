cmake_minimum_required(VERSION 3.12)

project(infrap4d VERSION 0.1.0 LANGUAGES C CXX)

add_executable(infrap4d
    ${STRATUM_SOURCE_DIR}/stratum/hal/bin/tdi/main.cc
)

target_include_directories(infrap4d PRIVATE ${STRATUM_SOURCE_DIR})

target_link_libraries(infrap4d PRIVATE
    -Wl,--whole-archive
    stratum
    -Wl,--no-whole-archive
)

install(TARGETS infrap4d DESTINATION sbin)
