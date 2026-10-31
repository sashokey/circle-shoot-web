# Third-party notices

## Game source and framework

The reconstructed game and native UI in `src/game`, and adapted application methods in `src/platform/OriginalUI*.cpp`, originate from [kyle-sylvestre/CircleShootApp](https://github.com/kyle-sylvestre/CircleShootApp/tree/7a6a24789ce5b622214d60019df0996c4c066973), based on [alula/CircleShootApp](https://github.com/alula/CircleShootApp). Upstream provides the reconstructed game source as-is and licenses modifications under MIT; this is not a blanket license for the original commercial game.

Math, random-number and buffer components under `src/platform/SexyAppFramework` retain the PopCap Games Framework license. Its complete terms and accompanying notices are in `licenses/PopCap-Framework.txt` and `licenses/PopCap-Third-Party.txt`.

This product includes portions of the PopCap Games Framework, © 2005-2009 PopCap Games, Inc. All rights reserved. (http://popcapframework.sourceforge.net/).

## Resources

HD atlases, glyph textures, sound effects, curve data and base level configuration originate from [19nkn80/Zuma-Deluxe-HD](https://github.com/19nkn80/Zuma-Deluxe-HD/tree/5bbc256cc431ee56b5e14617b113fc466508e566), a fork of GalaxyShad and s4lat's remake. Its source license is retained in `licenses/HD-Remake-MIT.txt`. Upstream attributes the original resources to PopCap and EA Games; its MIT source license does not grant rights to their artwork or audio. The remake's gameplay code is not included.

Font metrics and resource metadata originate from [ngrt/tinyzumadeluxe](https://github.com/ngrt/tinyzumadeluxe/tree/20f0e9118339ec94bd3d41286495870bc2ccc413). Supplemental original UI images and alpha masks, including the three Serpents foreground masks in `content/levels/serpents/_serpents*.gif`, originate from [Zuma Deluxe (Pogo Games)](https://archive.org/download/zumadeluxe-pogogames/Zuma%20Deluxe%20%28Pogo%20Games%29.zip), archive SHA-1 `de39ae3c73e14f9b63bed40d8a5f1edb577bc8a5`. Only image data was extracted.

The original Zuma MO3 track is loaded externally from the pinned Zuma-Deluxe-HD source. `music-cues.json` preserves the eight source subsong selections and durations. The browser renders these cues into temporary WAV audio in memory; no MO3, OGG or WAV music files are distributed in this repository. Decoding does not change the underlying music rights.

Unchanged resource files are loaded by the browser directly from the pinned upstream sources in `asset-sources.json` and are not included in this repository. The Pogo ZIP is accessed using Internet Archive's official `/cors/` endpoint. Only the selected image entries are read; no executable or library from the archive is executed. The adapted level configuration and prepared font metrics remain included. This storage distinction does not change the rights or license terms of either the external or retained resources.

## Music decoder

`vendor/libopenmpt/` contains the unmodified WebAssembly distribution of [libopenmpt 0.8.9](https://lib.openmpt.org/files/libopenmpt/dev/libopenmpt-0.8.9+release.dev.js.tar.gz). It includes no game music. The decoder is BSD-3-Clause licensed; its complete license and bundled dependency notices are retained in `licenses/libopenmpt/`.

## Port and runtime

The browser host, WASI bridge, widget backend and build/server scripts were written for this port. The binary links official WASI SDK 34 libraries; their notices are in `licenses/WASI-LIBC-LICENSE` and `licenses/LLVM-RUNTIME-LICENSE`. No BASS, SDL, downloaded native game executable or native game library is used. No upstream native build script was run.

This is not an official PopCap or EA release. No single license is asserted over the entire repository. The inclusion of third-party resources does not itself grant permission to redistribute them.
