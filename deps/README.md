# Dependencies

This directory holds optional vendored trees and internal VertexNova libraries.

## Layout

- **external/** – Optional local overrides for FetchContent deps (see [external/README.md](external/README.md)).
- **internal/** – Optional VertexNova libraries (vnecommon, vnelogging).

CMake modules (**vnecmake**) live at **cmake/vnecmake** and are a **required** git submodule.

## Git submodules (`.gitmodules`)

| Submodule | Path | Required |
|-----------|------|----------|
| vnecmake | `cmake/vnecmake` | yes |
| vnecommon | `deps/internal/vnecommon` | no |
| vnelogging | `deps/internal/vnelogging` | no |

From the project root:

```bash
git submodule update --init --recursive
```

## FetchContent (configure time)

Shader compiler and JSON dependencies are **not** submodules. They are declared in `cmake/Dependencies.cmake` and downloaded when you run `cmake` (unless you pass a local `VNE_SC_*_DIR` override).

Tests use **googletest** via FetchContent when system GTest is not installed (`tests/CMakeLists.txt`).

CI and release workflows still use `submodules: recursive` so **vnecmake** is present; Khronos and googletest are fetched during the CMake configure step on the runner (network required on cache miss).
