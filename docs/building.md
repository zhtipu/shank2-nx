# Building shank2-nx

Everything below runs on Windows 11 from PowerShell 7 (`pwsh`). It needs your own copy of the game: the code
generator reads `default.xex`, and the shader steps read the game's effect files. Nothing built from them may be
shared, except the NRO and the NSP made with `SHANK2_RENUT_RELEASE_SHADERS=ON` (no translated game shader in them but
the movie player's three, which the game compiles at run time and are in no game file).

## 1. Tools

- Visual Studio 2022 Build Tools (C++ workload, Windows SDK), LLVM (clang 20+), CMake, Ninja, Python 3 with
  `pip install xxhash pillow`, Git.
- devkitPro with devkitA64 and libnx at `C:\devkitPro`.
- For the NVK driver the NRO links (the Xenos emulation path): an NVK build of mesa-switch, assembled into an SDK folder
  (`lib/libvulkan.a` + `include/`).
- For the deko3d shader steps: SPIRV-Cross and `uam` built for the PC.
- For the NSP: hacBrewPack and your own `prod.keys` (never part of this repository).

`tools/vsenv.ps1` loads the VS 2022 environment into the current PowerShell (and strips a stray quote some machines
have in PATH). Dot-source it before every build; do not run CMake from Git Bash.

## 2. Set up the tree

```powershell
git clone --recursive https://github.com/zhtipu/shank2-nx
cd shank2-nx
pwsh -File tools/setup.ps1
```

`setup.ps1` checks out the SDK base at the pinned commit (`tools/sa2-nx`), applies `patches/sa2-nx.patch` (the SDK's
multi-stream XMA fix, Switch sockets, NSP paths, plume's D3D12 debug layer), copies the base's `app/thirdparty` to
`port/thirdparty` and downloads DXC 1.9.2609.

Put the extracted game (`default.xex`, `data/`, `data-360/`, `ArcadeInfo.xml`) in `extracted/` next to `port/`. The
installer page reads the package directly; to extract it on the PC, use `tools/sa2-nx/tools/stfs_extract.py` (or
ryzendew/XBLA-Extract).

## 3. Code generation

Build the SDK's code generator (`tools/sa2-nx/sdk`, preset `win-amd64`, target `rexglue`), then from `port/`:

```powershell
. ../tools/vsenv.ps1
& ../tools/sa2-nx/sdk/out/win-amd64/Release/rexglue.exe codegen shank2_manifest.toml --ignore-stamp
```

About 22,000 functions, about 100 MB of C++ in `port/generated/default`. `overrides.toml` already holds every
analysis correction the port needed.

## 4. PC build (needed for the shader table)

```powershell
cmake --preset win-amd64 -DSHANK2_RENUT_RENDERER=ON
cmake --build --preset win-amd64-release
```

Run it with `--game_data_root="<extracted>" --license_mask=1 --sa2_renderer=renut --renut_gpu_backend=d3d12`.

## 5. Shaders for the Switch

From `port/`:

```powershell
python tools/shank2_dump_shaders.py --game ../extracted --out out/renut_shader_dump
$env:BANJO_DEKO3D_SPIRV_DIR='out/deko3d_shaders/spirv'; $env:BANJO_HLSL_OVERRIDE='out/build/pc-renut/renut_gpu/early_alpha_hlsl'
out/build/pc-renut/xenos_recomp/build/XenosRecomp/XenosRecomp.exe out/renut_shader_dump out/deko3d_shaders/spirv_pass.cpp thirdparty/reblue-XenosRecomp/XenosRecomp/shader_common.h
python tools/deko3d_shaders.py --spirv out/deko3d_shaders/spirv --out out/deko3d_shaders/glsl --no-uam
python tools/deko3d_glsl_opt.py --in out/deko3d_shaders/glsl/glsl --out out/deko3d_shaders/glsl_opt --check
python tools/deko3d_shader_package.py --glsl out/deko3d_shaders/glsl_opt --out out/renut_switch
python tools/renut_switch_shader_cache.py --pc out/build/pc-renut
```

The movie player's three shaders (HLSL text in `default.xex`, compiled by the game at run time) are only found by a
run of the game: add `out/renut_shader_dump` files captured from a PC run if they are missing.

For a build that can be shared, the files without translated game shaders:

```powershell
python tools/shank2_dump_shaders.py --game ../extracted --out out/renut_shader_static
python tools/renut_release_shaders.py --glsl out/deko3d_shaders/glsl_opt --cache out/renut_switch/shader_cache_switch.cpp --out out/renut_switch_release --static-dump out/renut_shader_static
```

## 6. Switch build

```powershell
. ../tools/vsenv.ps1
$env:DEVKITPRO = 'C:/devkitPro'
$sa2 = "$PWD/../tools/sa2-nx"
cmake -S . -B out/nx-release -G Ninja -DCMAKE_BUILD_TYPE=Release `
  "-DCMAKE_TOOLCHAIN_FILE=$sa2/tools/switch/cmake/switch-devkitA64.cmake" "-DREXSDK_DIR=$sa2/sdk" `
  "-DREXGLUE_SWITCH_NVK_SDK=<your nvk-sdk folder>" -DDEVKITPRO=C:/devkitPro `
  -DSHANK2_RENUT_RENDERER=ON -DSHANK2_RENUT_RELEASE_SHADERS=ON
cmake --build out/nx-release -j 10
```

Without `-DSHANK2_RENUT_RELEASE_SHADERS=ON` (build folder `out/nx`) every translated shader is built in: a personal
build that needs no `deko3d_glsl.bin` on the SD card, and that must not be shared.

## 7. NSP

```powershell
pwsh -File nsp/build_nsp.ps1 -Release -Version 1.0.0 -Keys <your prod.keys>
```

`-Release` packs the loader and `out/nx-release/shank2.nro` only (the shareable NSP: the game comes from the SD
card). Without it, the NSP also carries your game pack in its RomFS: a personal, self-contained install.

## 8. LGPL libraries

FFmpeg and libmspack are built from the SDK's `thirdparty` folder by the steps above. To relink the NRO with a
modified version, change their sources under `tools/sa2-nx/sdk/thirdparty` and run step 6 again.
