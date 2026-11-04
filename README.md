# melonDS Android core — UD fork

This is the core library used by the [UD fork of melonDS Android](https://github.com/Umberto-DEV/melonDS-android).
It builds into the app's APK. Users install the app once; they do not download
or install this library separately. Both fork repositories use `main`, and the
app fixes its core dependency to a specific Git commit.

## Changes from the official Android core

Compared with the official Nightly core `431ab4bd`, this fork includes:

- **Safer save replacement.** A failed NAND save replacement could leave the
  old generation inconsistent. The new path stages the replacement, preserves
  the previous generation, restores it on failure and allows retry.
  [Before/after tests](https://github.com/Umberto-DEV/melonDS-android/blob/main/docs/maintenance/2026-11-03-nand-rename-fault/README.md).
- **Correct JIT memory handling.** The JIT converts emulated instructions into
  host code. Teardown and invalidation fixes prevent use of expired memory and
  prevent Thumb literal values from remaining cached when their writes cannot
  invalidate that code. [Baseline and tests](https://github.com/Umberto-DEV/melonDS-android/blob/main/docs/maintenance/2026-10-29-nightly-comparison/VALIDATION.md);
  [Thumb regression](https://github.com/Umberto-DEV/melonDS-android/blob/main/docs/maintenance/2026-11-02-core-literal-thumb/README.md).
- **Bounded cartridge and peripheral access.** Malformed ROMs, transfer lengths
  and peripheral commands could access memory outside native buffers. The
  corrected paths check the complete range before access and reject invalid
  restored values. [Individual fixes and evidence](https://github.com/Umberto-DEV/melonDS-android/blob/main/docs/maintenance/2026-11-03-security-followup/FINDING-DETAILS.md).
- **Safer saved states.** Invalid callbacks, queue positions, dimensions and
  indices are checked before use. The paired app restores the previous state
  when a load is rejected. [State-validation report](https://github.com/Umberto-DEV/melonDS-android/blob/main/docs/maintenance/2026-11-03-security-followup/README.md).
- **Bounded Wi-Fi frames and cheat execution.** DS/DSi frame lengths are checked
  against buffer capacity. Incomplete Action Replay commands are rejected;
  expensive cheats share a frame budget and deferred cheats resume on a later
  frame. [DS frames](https://github.com/Umberto-DEV/melonDS-android/blob/main/docs/maintenance/2026-11-03-wifi-ds-length/README.md),
  [DSi frames](https://github.com/Umberto-DEV/melonDS-android/blob/main/docs/maintenance/2026-11-03-dsi-wmi-length/README.md),
  [cheat regression tests](https://github.com/Umberto-DEV/melonDS-android/blob/main/docs/maintenance/2026-11-03-action-replay-length/README.md).
- **DS/DSi and rendering fixes.** Register initialization, access rules, Wi-Fi
  power saving and redundant renderer work are corrected. Existing OpenGL,
  Compute and high-resolution 3D support remain upstream features.
  [Changes and original contributors](https://github.com/Umberto-DEV/melonDS-android/blob/main/docs/maintenance/2026-10-28-refinement/INTEGRATION-REPORT.md).

## Testing and compatibility

The [validation report](https://github.com/Umberto-DEV/melonDS-android/blob/main/docs/maintenance/2026-11-03-security-followup/README.md)
separates host sanitizer tests, Android native execution on ARM64 4 KiB/16 KiB
page sizes, app tests and real-app checks. Each finding states its proof level;
a passing suite does not certify every game. DSi HOME/quota, physical-device
driver behavior and the compatibility of changed network rules remain limited
or pending. No measured CPU, GPU, battery or all-game improvement is claimed.

The [app downloads](https://github.com/Umberto-DEV/melonDS-android/releases)
carry their own package, signing and build identities. A fork-signed APK cannot
replace the official author's signed APK in place. Source changes can be
reviewed and built by the author with the official signing identity. ROMs,
BIOS, firmware, NAND and saves are not distributed with the application.

## Building and contributing

Use the [app repository's build instructions](https://github.com/Umberto-DEV/melonDS-android#building)
for the Android APK and initialize submodules recursively. A plain source ZIP
does not include submodule contents. For the core itself, see [BUILD.md](BUILD.md)
and [CONTRIBUTING.md](CONTRIBUTING.md). Original authorship, credits and licenses
are preserved below.

## Upstream melonDS information

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
See [BUILD.md](./BUILD.md) for build instructions.

## TODO LIST

 * better DSi emulation
 * better OpenGL rendering
 * netplay
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
