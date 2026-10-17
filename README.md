# Iconic Marble Shooter WebAssembly

A browser port of the community reconstruction of the iconic marble shooter 1.0.0, using HD remake graphics. Includes the 76-level Adventure campaign, Gauntlet Practice/Survival, native menus and animations.

## Technical details

Game logic and UI run in C++20/WebAssembly at 100 Hz. The JavaScript host provides Canvas rendering, bitmap font metrics, WebAudio and localStorage saves. The logical viewport is 640 × 480 with uniform scaling. The site is static and has no npm dependencies. A bundled libopenmpt WebAssembly decoder prepares the externally loaded music. HD remake gameplay code and native game binaries are not used.

## External resources

The browser loads 198 resource files from the external sources listed in `asset-sources.json`: the existing 197 resources and the original `zuma.mo3` music module from the pinned Zuma-Deluxe-HD commit. Each external file is checked against its SHA-256 digest before use. No music files are included in this repository.

The eight music cues are rendered from the external MO3 module in a Web Worker using the included official libopenmpt 0.8.9 WebAssembly decoder. `music-cues.json` preserves their original subsong selections and durations. The resulting WAV audio exists only in browser memory and is played through Blob URLs. No backend, conversion service or game rebuild is required.

Original UI images from the Pogo archive use Internet Archive's official CORS endpoint; the ZIP is downloaded once per page load and only requested image entries are decompressed in memory. The adapted `content/levels/levels.xml` and prepared `font-metrics.json` remain included. The game module and game logic are unchanged.

Use a current browser with WebAssembly, Web Crypto, Web Workers and `DecompressionStream('deflate-raw')` support. Serve over HTTPS, or localhost for development. External sources must be reachable; if a download fails or its contents change, the loading screen reports the source error. Images are decoded from fetched bytes so Canvas pixel reads and custom cursors remain available.

## Run and build

Run `node tools/serve.mjs` and open `http://127.0.0.1:8080/`.

The compiled `game.wasm` is included. To rebuild, install [WASI SDK 34](https://github.com/WebAssembly/wasi-sdk/releases/tag/wasi-sdk-34), set `WASI_SDK_PATH` to its directory and run `node tools/build.mjs`. The build uses standard WebAssembly exception handling and the SDK's C++ runtime/unwinder.

Run `npm test` with Node.js 24 or newer and internet access. The check downloads the external resources into memory, verifies their hashes and references, and checks every campaign layout, tunnel path and foreground mask for missing resources, placeholder definitions and invalid image bounds. The music check renders all eight subsongs and verifies their start orders, durations and non-silent PCM output. No original resource files are saved to the repository. Serpents uses the original foreground alpha masks over the HD background; this resource fix does not require rebuilding `game.wasm`.

## Sources

- [CircleShootApp](https://github.com/kyle-sylvestre/CircleShootApp/tree/7a6a24789ce5b622214d60019df0996c4c066973): reconstructed Deluxe engine and native UI.
- [Zuma-Deluxe-HD](https://github.com/19nkn80/Zuma-Deluxe-HD/tree/5bbc256cc431ee56b5e14617b113fc466508e566): HD graphics, audio and level data.
- [tinyzumadeluxe](https://github.com/ngrt/tinyzumadeluxe/tree/20f0e9118339ec94bd3d41286495870bc2ccc413): original font descriptors and resource metadata.
- [Zuma Deluxe (Pogo Games)](https://archive.org/download/zumadeluxe-pogogames/Zuma%20Deluxe%20%28Pogo%20Games%29.zip): supplemental original UI images and alpha masks.

Known source/asset limits: More Games is unimplemented; Space nebula is missing; quake fragments use rectangular HD cutouts.

## Licensing

This is an unofficial port with mixed licensing.

Original contributions to the browser port are licensed under the [MIT License](LICENSE). This license does not cover the reconstructed game code, third-party libraries, game assets or data derived from those assets. These materials retain their respective rights and terms. See [third-party notices](THIRD_PARTY_NOTICES.md) and `licenses/`.
