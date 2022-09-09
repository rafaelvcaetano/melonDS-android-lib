# melonDS Android

Android port of the DS emulator, melonDS.

This is just the core library with the appropriate changes required to make melonDS build for Android. An example of a
working frontend can be found in the [melonDS Android](https://github.com/rafaelvcaetano/melonDS-android) repository.

<p align="center"><img src="https://raw.githubusercontent.com/Arisotura/melonDS/master/res/icon/melon_128x128.png"></p>
<h2 align="center"><b>melonDS</b></h2>
DS emulator, sorta

The goal is to do things right and fast, akin to blargSNES (but hopefully better). But also to, you know, have a fun challenge :)
<hr>

## How to use

Firmware boot (not direct boot) requires a BIOS/firmware dump from an original DS or DS Lite.
DS firmwares dumped from a DSi or 3DS aren't bootable and only contain configuration data, thus they are only suitable when booting games directly.

### Possible firmware sizes

 * 128KB: DSi/3DS DS-mode firmware (reduced size due to lacking bootcode)
 * 256KB: regular DS firmware
 * 512KB: iQue DS firmware

DS BIOS dumps from a DSi or 3DS can be used with no compatibility issues. DSi BIOS dumps (in DSi mode) are not compatible. Or maybe they are. I don't know.

As for the rest, the interface should be pretty straightforward. If you have a question, don't hesitate to ask, though!

## How to build

### Linux:

1. Install dependencies: `sudo apt install cmake libcurl4-gnutls-dev libpcap0.8-dev libsdl2-dev qt5-default libslirp-dev libarchive-dev libepoxy-dev`
2. Download the melonDS repository and prepare:
  ```bash
  git clone https://github.com/Arisotura/melonDS
  cd melonDS
  mkdir build && cd build
  ```
3. Compile:
  ```bash
  cmake ..
  make -j$(nproc --all)
  ```

### Windows:

1. Install [MSYS2](https://www.msys2.org/)
2. Open the **MSYS2 MinGW 64-bit** terminal
3. Update the packages using `pacman -Syu` and reopen the terminal if it asks you to
4. Download the melonDS repository and prepare:
  ```bash
  git clone https://github.com/Arisotura/melonDS
  cd melonDS
  mkdir build && cd build
  ```
#### Dynamic builds (with DLLs)
5. Install dependencies: `pacman -S git make mingw-w64-x86_64-{cmake,mesa,SDL2,toolchain,qt5,libslirp,libarchive,libepoxy}`
6. Compile:
   ```bash
   cmake .. -G "MSYS Makefiles"
   make -j$(nproc --all)
   ../tools/msys-dist.sh
   ```
If everything went well, melonDS and the libraries it needs should now be in the `dist` folder.

#### Static builds (without DLLs, standalone executable)
5. Install dependencies: `pacman -S git make mingw-w64-x86_64-{cmake,mesa,SDL2,toolchain,qt5-static,libslirp,libarchive,libepoxy}`
6. Compile:
   ```bash
   cmake .. -G 'MSYS Makefiles' -DBUILD_STATIC=ON -DQT5_STATIC_DIR=/mingw64/qt5-static
   make -j$(nproc --all)
   mkdir dist && cp melonDS.exe dist
   ```
If everything went well, melonDS should now be in the `dist` folder.

### macOS:
1. Install the [Homebrew Package Manager](https://brew.sh)
2. Install dependencies: `brew install git pkg-config cmake sdl2 qt@6 libslirp libarchive libepoxy`
3. Download the melonDS repository and prepare:
  ```zsh
  git clone https://github.com/Arisotura/melonDS
  cd melonDS
  mkdir build && cd build
  ```
4. Compile:
   ```zsh
   cmake .. -DCMAKE_PREFIX_PATH="$(brew --prefix qt@6);$(brew --prefix libarchive)" -DUSE_QT6=ON -DMACOS_BUNDLE_LIBS=ON
   make -j$(sysctl -n hw.logicalcpu)
   ```
If everything went well, melonDS.app should now be in the current directory.

   
## TODO LIST

 * better DSi emulation
 * better OpenGL rendering
 * better wifi
 * the impossible quest of pixel-perfect 3D graphics
 * support for rendering screens to separate windows
 * emulating some fancy addons
 * other non-core shit (debugger, graphics viewers, etc)

### TODO LIST FOR LATER (low priority)

 * big-endian compatibility (Wii, etc)
 * LCD refresh time (used by some games for blending effects)
 * any feature you can eventually ask for that isn't outright stupid

## Credits

 * Martin for GBAtek, a good piece of documentation
 * Cydrak for the extra 3D GPU research
 * limittox for the icon
 * All of you comrades who have been testing melonDS, reporting issues, suggesting shit, etc

## Licenses

[![GNU GPLv3 Image](https://www.gnu.org/graphics/gplv3-127x51.png)](http://www.gnu.org/licenses/gpl-3.0.en.html)

melonDS is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

### External
* Images used in the Input Config Dialog - see `src/frontend/qt_sdl/InputConfig/resources/LICENSE.md`
