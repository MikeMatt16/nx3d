set(NX3D_SAMPLE "nx3d-example")
set(XBE_TITLE ${NX3D_SAMPLE})

# add_subdirectory(shaders)

include(XBEUtils REQUIRED)
find_package(NXDK REQUIRED)

add_executable(${NX3D_SAMPLE})

target_include_directories(
    ${NX3D_SAMPLE}
    PRIVATE
    "${CMAKE_SOURCE_DIR}"
)

target_compile_options(
    ${NX3D_SAMPLE}
    PRIVATE
    -Wall
    -fno-strict-aliasing
    $<$<CONFIG:Debug>:-g>
    $<$<CONFIG:Debug>:-gdwarf-4>
    $<$<CONFIG:Debug>:-O0>
    $<$<NOT:$<CONFIG:Debug>>:-O3>
)

target_compile_definitions(
    ${NX3D_SAMPLE}
    PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
)

target_link_options(
    ${NX3D_SAMPLE}
    PRIVATE
    $<$<CONFIG:Debug>:/debug:full>
    $<$<NOT:$<CONFIG:Debug>>:/debug:none>
)

target_link_libraries(
    ${NX3D_SAMPLE}
    PRIVATE
    NXDK::NXDK
    nx3d
)

target_sources(
    ${NX3D_SAMPLE}
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/math3d.cpp
)

split_debug(${NX3D_SAMPLE})

set(EXECUTABLE_BINARY "${CMAKE_CURRENT_BINARY_DIR}/${NX3D_SAMPLE}.exe")

add_xbe(
    xbe_file "${EXECUTABLE_BINARY}"
    TITLE "${XBE_TITLE}"
)

add_xiso(example xbe_file)
