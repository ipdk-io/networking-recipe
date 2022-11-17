cmake_minimum_required(VERSION 3.15)

project(krnlmon)

set(SAI_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/SAI" CACHE PATH
    "SAI source directory")

set(KRNLMON_SOURCE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/krnlmon" CACHE PATH
    "Path to kernel monitor source directory")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(krnlmon)
