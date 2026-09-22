add_library(${NX3D_LIB} STATIC)

find_package(NXDK REQUIRED)

target_compile_options(
    ${NX3D_LIB}
    PRIVATE
    -Wall -Wextra
    $<$<CONFIG:Debug>:-g>
    $<$<CONFIG:Debug>:-gdwarf-4>
    $<$<CONFIG:Debug>:-O0>
)

target_include_directories(
    ${NX3D_LIB}
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(
    ${NX3D_LIB}
    PUBLIC
    NXDK::NXDK
)

add_subdirectory(src)
