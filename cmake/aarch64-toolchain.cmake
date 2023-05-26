# Experimental toolchain file for aarch64

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_SYSTEM_VERSION 1)

# Cross-compile options
set(CROSS_COMPILE_BUILD  "x86_64-build_pc-linux-gnu")
set(CROSS_COMPILE_HOST   "aarch64-intel-linux-gnu")
set(CROSS_COMPILE_TARGET "aarch64-intel-linux-gnu")

# System root
set(CMAKE_SYSROOT $ENV{SDKTARGETSYSROOT})

# Compilers
set(CMAKE_C_COMPILER   aarch64-intel-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-intel-linux-gnu-g++)

# CMake build types
# Converted to uppercase for use in variable names
set(configTypes DEBUG MINSIZEREL RELEASE RELWITHDEBINFO)

# Compiler flags
string(JOIN " " securityFlags
    -fstack-protector-strong
    -D_FORTIFY_SOURCE=2
    -Wformat
    -Wformat-security
    -Werror=format-security
)

string(JOIN " " extraFlags
    -pipe
    -feliminate-unused-debug-types
)

foreach(_config IN LISTS configTypes)
    set(CMAKE_C_FLAGS_${_config}_INIT    "${securityFlags} ${extraFlags}")
    set(CMAKE_CXX_FLAGS_${_config}_INIT  "${securityFlags} ${extraFlags}")
endforeach()

# CMake adds optimization and debug flags based on the build type:
# - Debug           -g
# - MinSizeRel      -Os -DNDEBUG
# - Release         -O3 -DNDEBUG
# - RelWithDebInfo  -O2 -g -DNDEBUG

# Linker Flags
string(JOIN " " linkerFlags
    -Wl,-O1
    -Wl,--hash-style=gnu
    -Wl,--as-needed
    -Wl,-z,relro,-z,now
)

foreach(_config IN LISTS configTypes)
    set(CMAKE_EXE_LINKER_FLAGS_${_config}_INIT    ${linkerFlags})
    set(CMAKE_SHARED_LINKER_FLAGS_${_config}_INIT ${linkerFlags})
endforeach()

unset(_config)
unset(configTypes)

# Default build type
set(CMAKE_BUILD_TYPE "RelWithDebInfo")

# Search paths
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

if(NOT CMAKE_SYSROOT STREQUAL "")
    include_directories(BEFORE ${CMAKE_SYSROOT}/usr/local/include)
    include_directories(BEFORE ${CMAKE_SYSROOT}/usr/include)
    include_directories(BEFORE ${CMAKE_SYSROOT}/include)
endif()
