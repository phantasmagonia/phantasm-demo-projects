# Phantasm Demo Projects

Example games built on [Phantasm](https://github.com/phantasmagonia/Phantasm),
an open-source C++20 2D game engine with an isometric layer.

| Demo | Status | Shows |
|---|---|---|
| [`scavenger/`](scavenger) | In progress | Isometric tile map, depth sorting, physics movement with wall collision |

## Build

Requirements: CMake 3.26+, Ninja, a C++20 compiler, `glslc`, and `spirv-cross`
(macOS: `brew install cmake ninja shaderc spirv-cross`).

```sh
cmake --preset debug
cmake --build --preset debug
./build/debug/bin/scavenger
```

Scavenger controls: WASD/arrows move (screen-relative), Q/E zoom, Esc quits.

The engine is fetched at a pinned commit (see `CMakeLists.txt`). To build against a local
engine checkout:

```sh
cmake --preset debug -DFETCHCONTENT_SOURCE_DIR_PHANTASM=../Phantasm
```

## Art

The demos currently use flat placeholder shapes generated in code. Real art will be added
per demo, with its license recorded next to it.

## License

Code: [MIT](LICENSE).
