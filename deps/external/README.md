# External dependencies

Third-party dependencies for **vnesc** are resolved at **CMake configure** time via `cmake/Dependencies.cmake` (FetchContent), not git submodules.

Pinned versions (Khronos): tag `vulkan-sdk-1.3.296.0`. nlohmann/json: `v3.11.3`.

| Dependency | When fetched | Override |
|------------|--------------|----------|
| SPIRV-Cross | always | `-DVNE_SC_SPIRV_CROSS_DIR=` |
| SPIRV-Headers, glslang | `VNE_SC_GLSLANG=ON` (default ON) | `-DVNE_SC_SPIRV_HEADERS_DIR=`, `-DVNE_SC_GLSLANG_DIR=` |
| SPIRV-Tools | `VNE_SC_SPIRVTOOLS=ON` | `-DVNE_SC_SPIRV_TOOLS_DIR=` |
| nlohmann/json | `VNE_SC_JSON=ON` (default ON) | `-DVNE_SC_JSON_DIR=` |
| Dawn/Tint | `VNE_SC_TINT=ON` (default OFF) | FetchContent only |

**Google Test** (when tests are enabled): `tests/CMakeLists.txt` uses system GTest if found, otherwise FetchContent `v1.17.0`. Optional override: `-DVNE_SC_GOOGLETEST_DIR=`.

Local checkouts under `deps/external/*` are optional; if you set a `VNE_SC_*_DIR` cache variable, it must point at a tree that contains `CMakeLists.txt`.

CI and release builds run `git submodule update --init --recursive` only for entries in `.gitmodules` (e.g. **vnecmake**, optional **vnecommon** / **vnelogging**). Shader toolchain sources are downloaded into `build/<lib_type>/_deps/` during `cmake` configure.
