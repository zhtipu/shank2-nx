# Third-party notices

This port is built on the work of other projects. Their licenses apply to their code and to the parts of this
repository derived from it. The rest of this repository (`port/`, `nsp/` except the loader, `tools/`, `docs/`,
`patches/`) is GPL-3.0 (`LICENSE`).

## Projects this port is derived from

| Project | Used for | License |
|---|---|---|
| [sa2-nx](https://github.com/ChanseyIsTheBest/sa2-nx) by ChanseyIsTheBest | The base of this port: the SDK fork (`tools/sa2-nx/sdk`), Renut's renderer with its deko3d back end (`port/src/renut_engine`), the analysis and shader scripts in `port/tools`, the Switch toolchain file. Used as a submodule at commit `adf3a4eee524`, with this port's changes in `patches/sa2-nx.patch` | GPL-3.0; BSD-3-Clause for the SDK |
| [nfsmw-nx](https://github.com/StevensND/nfsmw-nx) by StevensND | The Horizon (Switch) layer of the SDK, the NVK driver build and the method sa2-nx follows | GPL-3.0; BSD-3-Clause for the SDK parts |
| [NFSMW Recompiled](https://github.com/madelrandel-blip/NFSMW-Recompiled) and [masseffect-nx](https://github.com/nebadasSwifty/masseffect-nx) | Where sa2-nx's structure and many of its scripts come from | GPL-3.0 |
| [ReXGlue SDK](https://github.com/rexglue/rexglue-sdk) v0.10.0 by Tom Clay, with portions from [Xenia](https://xenia.jp) | The code generator, Xbox 360 kernel and runtime, XMA audio, virtual file system (inside sa2-nx's `sdk/`) | BSD-3-Clause |
| [reNut](https://github.com/masterspike52/reNut) by masterspike52, as banjo-nx and sa2-nx carry it | `port/src/renut_engine`: the Direct3D-level renderer, retargeted to Shank 2's Direct3D | reNut has no license file; sa2-nx states its author permits reuse in ports. The banjo-nx, sa2-nx and Shank 2 parts: GPL-3.0 |
| [plume](https://github.com/renderbag/plume), with banjo-nx's deko3d back end | The graphics API layer the renderer draws through (taken from sa2-nx's `app/thirdparty/plume`; this port adds a D3D12 debug-layer option, in `patches/sa2-nx.patch`) | MIT |
| [XenosRecomp](https://github.com/hedge-dev/XenosRecomp) (reblue fork, with reNut's and banjo-nx's changes) | The shader translator the shader cache is built with (a build tool) | MIT and the licenses of its `thirdparty/` |
| [uam](https://github.com/devkitPro/uam) by fincs, with SwitchWakerHD's and banjo-nx's patches | The deko3d shader compiler, in the NRO and as a PC tool | zlib, MIT (Mesa's files), MPL-2.0 (the patches) |
| [mesa-switch](https://github.com/danfromtico/mesa-switch) on top of [Mesa](https://mesa3d.org), as nfsmw-nx builds it | The Vulkan driver (NVK) linked into the NRO for the Xenos emulation path | MIT (Mesa's licenses per file) |
| [nx-hbloader](https://github.com/switchbrew/nx-hbloader) and [Carafe](https://github.com/carafe-nx/carafe)'s loader based on it | `nsp/loader`: starts the NRO from the NSP's RomFS (`nsp/loader/LICENSE`) | ISC |

The libraries the SDK and the NRO use (FFmpeg and libmspack under the LGPL, fmt, spdlog, toml++, Dear ImGui, libnx,
Zstandard and others) are listed with their licenses in sa2-nx's
[THIRD_PARTY_NOTICES.md](https://github.com/ChanseyIsTheBest/sa2-nx/blob/adf3a4eee524dcca5cd7c1b42454ddb8a09124a1/THIRD_PARTY_NOTICES.md).
This port changes none of them. Everything needed to rebuild and relink the NRO with a modified version of an LGPL
library is in this repository, sa2-nx and [docs/building.md](docs/building.md).

## The installer page

The installer page is its own repository, [shank2-nx-installer](https://github.com/zhtipu/shank2-nx-installer), based
on [sa2-nx-installer](https://github.com/ChanseyIsTheBest/sa2-nx-installer) and
[nfsmw-nx-installer](https://github.com/StevensND/nfsmw-nx-installer). Its notices list its WebAssembly tools.

## Trademarks

Shank and Shank 2 are trademarks of Klei Entertainment; the icon (`icon.ico`) is based on the game's artwork. Nintendo
Switch is a trademark of Nintendo. Xbox 360 and Xbox Live Arcade are trademarks of Microsoft. This project is not
affiliated with or endorsed by any of them.
