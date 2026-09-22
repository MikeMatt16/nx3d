cmake_minimum_required(VERSION 3.30)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
include(PrebuildNXDK)

project(nx3d LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL "Generate verbose build output")

if (NOT CMAKE_TOOLCHAIN_FILE MATCHES "toolchain-nxdk.cmake")
    message(FATAL_ERROR "This project must be built with the nxdk toolchain: -DCMAKE_TOOLCHAIN_FILE=<NXDK_DIR>/share/toolchain-nxdk.cmake")
endif ()

add_subdirectory(nx3d)
add_subdirectory(example)
