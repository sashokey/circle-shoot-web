# Iconic Marble Shooter WebAssembly

A browser port of the community reconstruction of the iconic marble shooter 1.0.0, using HD remake graphics. Includes the 76-level Adventure campaign, Gauntlet Practice/Survival, native menus and animations.

## Technical details

Game logic and UI run in C++20/WebAssembly at 100 Hz. The JavaScript host provides Canvas rendering, bitmap font metrics, WebAudio and localStorage saves. The logical viewport is 640 × 480 with uniform scaling. The site is static and has no npm dependencies. A bundled libopenmpt WebAssembly decoder prepares the externally loaded music. HD remake gameplay code and native game binaries are not used.

## Sources

- [CircleShootApp](https://github.com/kyle-sylvestre/CircleShootApp/tree/7a6a24789ce5b622214d60019df0996c4c066973): reconstructed Deluxe engine and native UI.
- [Zuma-Deluxe-HD](https://github.com/19nkn80/Zuma-Deluxe-HD/tree/5bbc256cc431ee56b5e14617b113fc466508e566): HD graphics, audio and level data.
- [tinyzumadeluxe](https://github.com/ngrt/tinyzumadeluxe/tree/20f0e9118339ec94bd3d41286495870bc2ccc413): original font descriptors and resource metadata.
- [Zuma Deluxe (Pogo Games)](https://archive.org/download/zumadeluxe-pogogames/Zuma%20Deluxe%20%28Pogo%20Games%29.zip): supplemental original UI images and alpha masks.

Known source/asset limits: More Games is unimplemented; Space nebula is missing; quake fragments use rectangular HD cutouts.

## Licensing

This is an unofficial port with mixed licensing.

Original contributions to the browser port are licensed under the [MIT License](LICENSE). This license does not cover the reconstructed game code, third-party libraries, game assets or data derived from those assets. These materials retain their respective rights and terms. See [third-party notices](THIRD_PARTY_NOTICES.md) and `licenses/`.
