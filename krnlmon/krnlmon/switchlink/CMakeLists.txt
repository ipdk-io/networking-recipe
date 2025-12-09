# CMake build file for switchlink
#
# Copyright 2022-2024 Intel Corporation
# SPDX-License-Identifier: Apache 2.0

add_subdirectory(sai)

add_library(switchlink_o OBJECT
    switchlink_address.c
    switchlink_db.c
    switchlink_db.h
    switchlink_db_int.h
    switchlink.h
    switchlink_globals.c
    switchlink_globals.h
    switchlink_handlers.h
    switchlink_int.h
    switchlink_link.c
    switchlink_link_types.h
    switchlink_main.c
    switchlink_main.h
    switchlink_neigh.c
    switchlink_route.c
    switchlink_utils.c
    switchlink_utils.h
    $<TARGET_OBJECTS:switchlink_sai_o>
)

if(ES2K_TARGET)
    target_sources(switchlink_o PRIVATE switchlink_validate_driver.c)
endif()

target_compile_options(switchlink_o PRIVATE -DHAVE_NLA_BITFIELD32)

if(WITH_OVSP4RT)
    target_compile_definitions(switchlink_o PRIVATE OVSP4RT_SUPPORT)
endif()

target_link_libraries(switchlink_o PkgConfig::nl3)

target_include_directories(switchlink_o PRIVATE
    ${SDE_INSTALL_DIR}/include/target-utils/third-party  # judy
    ${SDE_INSTALL_DIR}/include/target-utils/third-party/tommyds
    ${SDE_INSTALL_DIR}/include/target-utils/third-party/xxHash
    ${SDE_INSTALL_DIR}/include
    ${SAI_INCLUDE_DIR}
)

########################
# switchlink_link_test #
########################

add_executable(switchlink_link_test
    switchlink_link_test.cc
    switchlink_globals.c
    switchlink_int.h
    switchlink_link.c
    switchlink_link_types.h
)

define_unit_test(switchlink_link_test)

###########################
# switchlink_address_test #
###########################

add_executable(switchlink_address_test
    switchlink_address_test.cc
    switchlink_address.c
    switchlink_globals.c
)

define_unit_test(switchlink_address_test)

############################
# switchlink_neighbor_test #
############################

add_executable(switchlink_neighbor_test
    switchlink_neigh_test.cc
    switchlink_globals.c
    switchlink_int.h
    switchlink_neigh.c
)

define_unit_test(switchlink_neighbor_test)

#########################
# switchlink_route_test #
#########################

add_executable(switchlink_route_test
    switchlink_route_test.cc
    switchlink_route.c
    switchlink_int.h
)

define_unit_test(switchlink_route_test)
