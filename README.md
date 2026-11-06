# melonDS Android core — UD fork

This is the core library used by the [UD fork of melonDS Android](https://github.com/Umberto-DEV/melonDS-android).
It builds into the app's APK. Users install the app once; they do not download
or install this library separately. Both fork repositories use `main`, and the
app fixes its core dependency to a specific Git commit.

<a id="changes-from-the-official-android-core"></a>
## What's new

The comparison is against official Android core `431ab4bd`, still the upstream
`master` checked on 6 November 2026. Native corrections are at `ac5e6ba0`;
`2bb701ff` added the public overview. The app pins an exact core revision.

- **Optional hostname answers for a frontend.** UD adds a disabled-by-default
  hostname-to-IPv4 override table to the built-in DNS responder. It supplies
  endpoint addresses, not a replacement system DNS resolver. The Android
  firmware Wi-Fi editor does not use this API.
  [Implementation](https://github.com/Umberto-DEV/melonDS-android-lib/commit/f950b5e6)
  · [Packet tests and limits](https://github.com/Umberto-DEV/melonDS-android/blob/main/docs/maintenance/2026-10-29-nightly-comparison/VALIDATION.md)
- **Shared fixes used by UD Nightly.** NAND recovery, JIT ownership,
  instruction/data bounds and work budgets below compile into the Android app.
  Existing DS/DSi emulation, renderers and high-resolution 3D are upstream work.
  No separate core APK is needed.

The B7 save-page correction adapts Arisotura's
[merged cartridge refactor #2595](https://github.com/melonDS-emu/melonDS/pull/2595).
The JIT teardown research retains credit to maximxlss and RSDuck from
[#2537](https://github.com/melonDS-emu/melonDS/issues/2537).
Other existing authorship, licenses and credits remain intact.
[Integration/provenance register](https://github.com/Umberto-DEV/melonDS-android/blob/main/docs/maintenance/2026-10-28-refinement/INTEGRATION-REPORT.md#3-cosa-contiene-la-libreria-core)

## Bug fixes

Each linked dossier states the caller, original failure, correction, exact
source/tests and limits. **High/Medium** reproduce the linked security record's
rating. Other impact labels describe a functional failure without assigning a
security score. Component tests do not establish full-game or driver acceptance.

| Correction | Impact | Evidence and remaining limits |
| --- | --- | --- |
| Stage NAND replacements, retain the previous generation and recover after failed renames before retry. | Save loss risk | [One-shot fault sweep: host sanitizers and Android ARM64 4/16 KiB RED/GREEN](https://github.com/Umberto-DEV/melonDS-android/blob/main/docs/maintenance/2026-11-03-nand-rename-fault/README.md#correction-and-verification). Persistent hardware faults, power loss, ART/SAF and DSi HOME remain outside that result. |
| Reset fast-memory mappings before ARM9 teardown; keep DTCM aliases within configured windows. | Expired memory, wrong mapping | [Teardown/mapping controls](https://github.com/Umberto-DEV/melonDS-android/blob/main/docs/maintenance/2026-10-29-nightly-comparison/VALIDATION.md). Host and Android component results are distinct from physical gameplay. |
| Fold Thumb literals only when writes can invalidate the generated block, on AArch64 and x86-64. | Stale generated code | [AArch64 reproduction](https://github.com/Umberto-DEV/melonDS-android/blob/main/docs/maintenance/2026-11-02-core-literal-thumb/README.md) · [Linux x86-64 RED/GREEN](https://github.com/Umberto-DEV/melonDS-android/blob/main/docs/maintenance/2026-11-03-security-followup/FINDING-DETAILS.md#causa-extra-scan-non-contata). Linux checks do not establish Android x86-64 runtime acceptance. |
| Check ROM headers, cartridge transfers, NAND save pages and GBA flash accesses before reading/writing native buffers. | High: native memory bounds | [Cartridge image](https://github.com/Umberto-DEV/melonDS-android/blob/main/docs/maintenance/2026-11-03-security-followup/FINDING-DETAILS.md#occ_bf9ea27628798833dda7cf7b) · [NAND page RED/GREEN](https://github.com/Umberto-DEV/melonDS-android/blob/main/docs/maintenance/2026-11-03-nand-cart-read/README.md#final-correction-verification-and-limits) · [GBA flash](https://github.com/Umberto-DEV/melonDS-android/blob/main/docs/maintenance/2026-11-03-security-followup/FINDING-DETAILS.md#occ_0f5f2f5c076ab2aa0094b993). Per-case proof levels differ. |
| Validate savestate sections, scheduler parameters, FIFO positions, graphics/audio dimensions and peripheral state before use. | High/Medium: native memory bounds or nontermination | [Section parsing](https://github.com/Umberto-DEV/melonDS-android/blob/main/docs/maintenance/2026-11-03-security-followup/FINDING-DETAILS.md#occ_2be586dc4a21fbc0f60e65fd) · [Callbacks/indices](https://github.com/Umberto-DEV/melonDS-android/blob/main/docs/maintenance/2026-11-03-security-followup/FINDING-DETAILS.md#occ_2993485bc81b37540bda66df) · [Scheduler follow-up](https://github.com/Umberto-DEV/melonDS-android/blob/main/docs/maintenance/2026-11-03-security-followup/FINDING-DETAILS.md#occ_f332a69bdae9b2e0d1dac524). Paired-app load rollback is separate; arbitrary old savestates are not certified. |
| Bound DS/DSi Wi-Fi frame conversion and restored multiplayer lengths to the destination capacity. | Native buffer bounds | [DS host/Android RED/GREEN](https://github.com/Umberto-DEV/melonDS-android/blob/main/docs/maintenance/2026-11-03-wifi-ds-length/README.md#correction-and-independent-verification) · [DSi host/Android RED/GREEN](https://github.com/Umberto-DEV/melonDS-android/blob/main/docs/maintenance/2026-11-03-dsi-wmi-length/README.md#correction-tests-and-limits). Real multiplayer/server interoperability remains separate. |
| Reject truncated Action Replay instructions; share a frame work budget and resume deferred cheats. | Out-of-bounds read; Medium: emulation hang | [Instruction-length RED/GREEN](https://github.com/Umberto-DEV/melonDS-android/blob/main/docs/maintenance/2026-11-03-action-replay-length/README.md#correction-and-verification) · [Work-budget case](https://github.com/Umberto-DEV/melonDS-android/blob/main/docs/maintenance/2026-11-03-security-followup/FINDING-DETAILS.md#occ_95b372be5c7a53614199db50). This does not prove every cheat correct or every workload fast. |
| Validate firmware ranges, DSP descriptors/commands and G.711 transfers before dispatch. | High: native buffer access; Medium: stalled or invalid DSP execution | [Firmware](https://github.com/Umberto-DEV/melonDS-android/blob/main/docs/maintenance/2026-11-03-security-followup/FINDING-DETAILS.md#occ_a558a5b137d2433c0269cf8e) · [DSP descriptors](https://github.com/Umberto-DEV/melonDS-android/blob/main/docs/maintenance/2026-11-03-security-followup/FINDING-DETAILS.md#occ_9e7a80234cd3366f50ce8d90) · [G.711 follow-up](https://github.com/Umberto-DEV/melonDS-android/blob/main/docs/maintenance/2026-11-03-security-followup/FINDING-DETAILS.md#occ_86c50141e68ec4d1fcff078f). Source review and focused suites are not individual runtime reproductions of every card. |
| Bound DNS parsing, release resolver allocations and restrict guest access to private/loopback endpoints. | Medium: native read, leak or access to host services | [DNS parsing](https://github.com/Umberto-DEV/melonDS-android/blob/main/docs/maintenance/2026-11-03-security-followup/FINDING-DETAILS.md#occ_6b9635717a6a6448125d3010) · [Network policy](https://github.com/Umberto-DEV/melonDS-android/blob/main/docs/maintenance/2026-11-03-security-followup/FINDING-DETAILS.md#occ_46d778442f1d34d3992b75b7). Private-network services and the changed LAN representation need compatibility review. |
| Apply DS/DSi SCFG access rules and Wi-Fi automatic power-down only in the requested mode. | Incorrect emulated hardware behavior | [Source changes, provenance and test scope](https://github.com/Umberto-DEV/melonDS-android/blob/main/docs/maintenance/2026-10-28-refinement/INTEGRATION-REPORT.md#3-cosa-contiene-la-libreria-core). No general C-Gear, DSi HOME or all-game closure follows. |
| Skip unchanged Compute renderer configuration, unused edge-data work and a false cache-rejection log. | Redundant work, misleading diagnostic | [Source-level comparison](https://github.com/Umberto-DEV/melonDS-android/blob/main/docs/maintenance/2026-10-29-nightly-comparison/README.md). No measured FPS, CPU, GPU or battery improvement is claimed. |

The [complete per-occurrence register](https://github.com/Umberto-DEV/melonDS-android/blob/main/docs/maintenance/2026-11-03-security-followup/FINDING-DETAILS.md#index)
covers both repositories: 97 code-corrected occurrences, two repository
corrections with external controls unverified, one Android-not-applicable case
and one informational reconciliation. The 101 occurrences are not 101
independent bugs, and the app's release controls are not core runtime fixes.

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
