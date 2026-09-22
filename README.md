# nx3d

New Xbox 3D graphics library, a C graphics library for [nxdk](https://github.com/XboxDev/nxdk) projects based on pbkit.
## Build

Use Linux or WSL with the [nxdk toolchain dependencies](https://github.com/XboxDev/nxdk/wiki/Install-the-Prerequisites#linux) and CMake 3.30+ installed.

Clone the repository and use cmake to build:
```sh
cmake \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_TOOLCHAIN_FILE=$NXDK_DIR/share/toolchain-nxdk.cmake \
    -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE \
    -S . -B build -G Ninja
cmake --build build
```
_Note: NXDK_DIR has to be set to or replaced by your nxdk repository directory path. Ensure `$NXDK_DIR/share/toolchain-nxdk.cmake` exists and that the nxdk's submodules are cloned as well._
Alternatively you can initialize the nxdk submodule within this repository, then build using included build presets:
```sh
git submodule update --init --recursive
cmake --preset debug
cmake --build --preset debug
```

Replace `debug` with `release` for an optimized build. The existing CMake
bootstrap builds nxdk when its required libraries/tools are missing.

Outputs include:

- `build/debug/nx3d/libnx3d.lib`
- `build/debug/example/xbe/xbe_file/default.xbe`
- `build/debug/example/xiso/example/example.iso`

## Using the library

In an nxdk CMake project using this repository's NXDK package module, add the
`nx3d` subdirectory and link your target against `nx3d`. Its include directory
and NXDK dependency are propagated to consumers. The public header is
`nx3d/include/nx3d.h` and is usable from C and C++.

Set the video mode before `nx3d_init()`. The library owns pbkit for its lifetime;
do not separately initialize it or interleave raw pbkit drawing/state changes.
Call the API from one thread.

```c
(void)XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);
if (nx3d_init() != 0)
    return nx3d_get_error();
```

Check each fallible operation's return value in application code; the example
handles initialization failures. Success returns zero; failure returns -1 and preserves
the first outstanding error. `nx3d_get_error()` returns and clears that error.

See the included example for sample scene rendering code.

## Credit
* The [XboxDev](https://github.com/XboxDev) team, including the information on [xboxdevwiki](https://xboxdevwiki.net) for providing the sample code which was borrowed for this project's sample
* [nxdk_cmake_template](https://github.com/abaire/nxdk_cmake_template) by abaire was utilized for this project