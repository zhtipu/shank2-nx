<div align=center>

<img width="616" height="353" alt="capsule_616x353" src="https://github.com/user-attachments/assets/5d157e9d-41af-422e-876f-14d47c1718de" />


</div>
<h1 align=center>shank2-nx</h1>

A native Nintendo Switch port of **Shank 2** (Klei Entertainment, Xbox Live Arcade, 2012). It is not an emulator:
the game's PowerPC code is statically recompiled to C++, and it draws through a native renderer on deko3d, the
Switch's own graphics API. It runs at 60 fps.

> [!NOTE]
> You need your own copy of the Xbox Live Arcade game: its content package from an Xbox 360
> (`Content/0000000000000000/5841120C/000D0000/`), or the game extracted from it. This repository and its releases
> contain no game data.

## How to install 

<img width="373" height="311" alt="giphy" src="https://github.com/user-attachments/assets/bd1e1649-699a-49f7-be8c-730cfa5d96e0" />


1. Open the installer page: **https://zhtipu.github.io/shank2-nx-installer/**
2. Choose your game (its package file, the `5841120C` folder, or an extracted folder with `default.xex`) and press
   **Create shank2-nx.zip**. Everything runs in your browser and the game files never leave your computer: the page
   checks your copy, downloads the build and makes the shaders and the game pack from your files.
3. Extract `shank2-nx.zip` into `sdmc:/switch/`.
4. Start it:
   - **NSP (recommended):** install `shank2-nx-x.y.z.nsp` from the [latest release](https://github.com/zhtipu/shank2-nx/releases/latest)
     with your installer of choice and start **Shank 2** from the HOME Menu. The NSP holds only the program; it reads
     the game and its shaders from `/switch/shank2-nx/`.
   - **NRO:** start `shank2.nro` from the Homebrew Menu in title takeover mode (hold **R** while launching an
     installed game) or with a 39-bit forwarder. The album applet does not give the game enough memory.

Your `shank2-nx` folder:

```text
/switch/shank2-nx/
  shank2.nro
  LICENSES.txt
  deko3d_glsl.bin           (the game's shaders, made from your copy)
  deko3d_shaders.bin        (the shaders compiled for the console; written as you play)
  game_root.nxpack          (the files of your game, in one file)
  game_root.nxidx
  shank2.toml               (optional settings)
```

> [!CAUTION]
> The first time you play, the console compiles each shader the first time it is drawn, so new places can stutter for
> a moment. The compiled shaders are saved in `deko3d_shaders.bin` and later sessions start smoothly.

**Updating:** open the installer page again and press **Create shank2-nx-update.zip** (only the program and the
shaders), extract it over your folder, and install the new NSP if you use it.

## Controls

The Xbox 360 layout on the Switch buttons by position. Stick clicks are L3 / R3. The port's own menus are on
**L + R** with the D-pad (Up: performance overlay, Right: settings, Down: log console, Left: achievements).

## Known issues

- The first visit to a new effect can stutter for a moment while its shader compiles (see above).
- Some alpha-tested effects use slightly more GPU time than on the development build: the installer's shaders do not
  carry the PC build's early alpha optimisation.

## Building

See [docs/building.md](docs/building.md).

## License

GPL-3.0 (`LICENSE`). Third-party code and its licenses: [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

Shank and Shank 2 are trademarks of Klei Entertainment. This project is not affiliated with or endorsed by Klei
Entertainment, Nintendo or Microsoft. Running homebrew requires custom firmware, which is against Nintendo's terms of
service; use it at your own risk.
