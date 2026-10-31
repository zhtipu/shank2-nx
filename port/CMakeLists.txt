# shank2 - ReXGlue Recompiled Project
#
# SDK boilerplate lives in generated/rexglue.cmake.
#
# PC:      cmake --preset win-amd64-release -DCMAKE_PREFIX_PATH=<installed sa2-nx SDK>
# Switch:  cmake -S . -B out/nx -G Ninja -DCMAKE_BUILD_TYPE=Release
#            -DCMAKE_TOOLCHAIN_FILE=<sa2-nx>/tools/switch/cmake/switch-devkitA64.cmake
#            -DREXSDK_DIR=<sa2-nx>/sdk -DREXGLUE_SWITCH_NVK_SDK=<nvk-sdk>

cmake_minimum_required(VERSION 3.25)
project(shank2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -mearly-ra=none for everything compiled for the console (as sa2-nx, peggle2-nx and banjo-nx do): devkitA64 GCC's
# AArch64 early register allocation pass dropped table lookups of a NEON permute helper and produced garbage on the
# console only. Set before the SDK is added so its libraries and the LTO link get it too.
if(REXGLUE_PLATFORM_SWITCH)
    add_compile_options(-mearly-ra=none)
    add_link_options(-mearly-ra=none)
endif()

include(generated/rexglue.cmake)

# The SDK sources the Renut engine includes from (src/core headers); the PC build links the installed SDK package,
# the Switch build adds the source tree itself (REXSDK_DIR).
set(SHANK2_SDK_SRC "${CMAKE_CURRENT_SOURCE_DIR}/../tools/sa2-nx/sdk" CACHE PATH "sa2-nx SDK source tree (headers)")

# Sources
set(SHANK2_SOURCES
    src/main.cpp
    src/shank2_compat.cpp
    src/shank2_nsp.cpp
)

if(WIN32)
    add_executable(shank2 WIN32 ${SHANK2_SOURCES})
else()
    add_executable(shank2 ${SHANK2_SOURCES})
endif()
target_include_directories(shank2 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src" "${SHANK2_SDK_SRC}/src/core")

# Renut's Direct3D-level renderer (sa2-nx's copy of reNut): no PM4 ring, no EDRAM. Chosen at boot with
# sa2_renderer = "renut" (the cvar keeps its sa2-nx name); without that key the generic Xenos GPU emulation runs.
option(SHANK2_RENUT_RENDERER "Link Renut's Direct3D-level renderer (sa2_renderer = renut)" ON)

if(REXGLUE_PLATFORM_SWITCH)
    # One static NRO: the Xenos GPU emulation plugin is linked in (no dynamic loading on the console).
    rexglue_setup_target(shank2 GPU_PLUGINS xenos)
    target_compile_definitions(rexcore PRIVATE REX_SWITCH_FALLBACK_EXE="/switch/shank2-nx/shank2.nro")
    # The homebrew menu icon must be a 256x256 JPEG (made from icon.ico, opaque).
    rexglue_switch_add_nro(shank2 NAME "Shank 2" AUTHOR "github.com/zhtipu" VERSION "0.1.0"
        ICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.jpg")
else()
    rexglue_setup_target(shank2)
endif()

if(SHANK2_RENUT_RENDERER)
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/shank2_renut_gpu.cmake")
endif()
