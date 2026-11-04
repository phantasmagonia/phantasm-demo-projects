# AGENTS.md

Example games for the Phantasm engine (https://github.com/phantasmagonia/Phantasm).
Read the engine's AGENTS.md for engine conventions.

- Each demo is a folder with its own `CMakeLists.txt`, `src/`, and `assets/` (data such as maps).
- The engine is pinned by commit in the root `CMakeLists.txt`; bump it deliberately.
- Local engine development: `-DFETCHCONTENT_SOURCE_DIR_PHANTASM=../Phantasm`.
- Art: placeholders are generated in code (`placeholder_art.*`). Any real art must have its
  license recorded next to it.
- C++20, warnings as errors via the engine's `phantasm_warnings` target, conventional commits.
