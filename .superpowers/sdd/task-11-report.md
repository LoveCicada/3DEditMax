# Task 11 Report: Meshes, layout, shading (Phase 2)

## Status

**DONE**

## Summary

CPU mesh builders live in `src/teach/MeshBuild` (cube / UV sphere / Y-up cylinder with caps). `MeshGpu::createFromCpu` uploads 16-bit indices. The renderer holds cube/sphere/cylinder, draws 1 or 3 objects, and uses a wire rasterizer for `ShadeWire`. `FrameCB` gained `float4 shadingMode`; the pixel shader branches solid / normal / `ps_checker` (`frac(uv*8)`). ObjectPanel sits in a left dock under transforms. Local commit only; not pushed.

## Files Created

| File | Change |
|------|--------|
| `src/teach/MeshBuild.h` | `MeshVertex`, `buildCube` / `buildSphere` / `buildCylinder`, `cubeVertexCount` |
| `src/teach/MeshBuild.cpp` | CPU generation; clockwise outside winding; sphere verts `(stacks+1)*(slices+1)` |
| `src/ui/ObjectPanel.h` | Mesh radios, layout/shading combos, `changed()` |
| `src/ui/ObjectPanel.cpp` | Qt-only; no `windows.h` / D3D |

## Files Modified

| File | Change |
|------|--------|
| `src/render/MeshGpu.h/.cpp` | `createFromCpu`; `createSphere(dev, slices, stacks)`; `createCylinder(dev, slices)`; `DXGI_FORMAT_R16_UINT` |
| `src/render/D3D11Renderer.h/.cpp` | `m_cube` / `m_sphere` / `m_cyl`; wire RS; per-object W/WVP + `shadingMode`; still `XMMatrixTranspose` before store |
| `assets/shaders/unlit.hlsl` | `shadingMode`; `ps_checker`; `ps_main` branches on `cb.mode` (`mul(float4, float4x4)` unchanged) |
| `src/app/MainWindow.h/.cpp` | Left dock split: ObjectPanel under transforms; cross-panel `setState` |
| `tests/test_transforms.cpp` | Cube indices 36; sphere vert count; cylinder non-empty |
| `CMakeLists.txt` | MeshBuild on app + tests; ObjectPanel on app |

## Files Not Modified

- `src/render/ShaderSet.cpp` — second PS not compiled; branch in `ps_main` instead
- `src/core/TeachingState.h` — mesh/layout/shading enums already present

## TDD Evidence

### RED

Wrote MeshBuild assertions in `tests/test_transforms.cpp` **before** `MeshBuild.h` existed.

```
E:\code\private\3DEditMax\tests\test_transforms.cpp(3): fatal error C1083: 无法打开包括文件: “teach/MeshBuild.h”: No such file or directory
ninja: build stopped: subcommand failed.
```

Failure reason: feature missing, not a typo.

### GREEN

Implemented `MeshBuild` + `cubeVertexCount()`. CMake adds `MeshBuild.cpp` to `3deditmax_tests`.

```
cmake --build build --target 3deditmax_tests 3DEditMax
ctest --test-dir build -R 3deditmax_tests --output-on-failure
1/1 Test #1: 3deditmax_tests ..................   Passed    0.06 sec
100% tests passed, 0 tests failed out of 1
```

## Build & Test

- Branch: `feat/dx11-lab-teach`
- MSVC 14.33 via `D:\soft\vs2022\ide\VC\Auxiliary\Build\vcvars64.bat`
- `QT_DIR`: `D:\soft\qt5152\5.15.2\msvc2019_64`
- `cmake --build build --target 3deditmax_tests 3DEditMax` exit 0
- CTest `3deditmax_tests` **Passed** (1/1)

## Commit

| SHA | Subject |
|-----|---------|
| `6f0f545` | Add teaching meshes, three-object layout, and shading modes. |

Local commit only; not pushed. Not amended.

## Self-Review

### Requirements coverage

- [x] `buildCube` / `buildSphere` / `buildCylinder` + `cubeVertexCount` in `src/teach/MeshBuild`
- [x] Cube indices 36; sphere verts `(stacks+1)*(slices+1)` > 0
- [x] `MeshGpu::createFromCpu`; `createSphere(dev, 16, 24)`; `createCylinder(dev, 24)`
- [x] Renderer holds `m_cube`, `m_sphere`, `m_cyl`
- [x] `LayoutThree` draws `objects[0..2]`; `LayoutOne` draws `[0]`
- [x] `ShadeWire` → `D3D11_FILL_WIREFRAME` (LabState fill ignored until Phase 4)
- [x] `FrameCB` `float4 shadingMode` (x=0 solid, 1 normal, 2 checker)
- [x] Checker: `frac(uv*8)` black/white
- [x] HLSL `mul(float4, float4x4)` + CPU `XMMatrixTranspose` before store
- [x] ObjectPanel: 3 mesh radios, layout combo, shading combo, `changed()`; no windows.h/D3D
- [x] MainWindow left dock stacks ObjectPanel under transforms
- [x] C++11

### Concerns

1. **Manual Step 4 not run in-session.** Counts are CTest-covered; three-object / sphere / checker / wire were not clicked in the running app.
2. **Mesh radio edits `objects[0]` only.** LayoutThree keeps objects 1–2 as default cubes unless those slots are changed elsewhere.
3. **LayoutOne draws `objects[0]`**, whose default `pos.x` is `-2.5` (`teachingStateDefault`). Pre-existing; the cube sits left of center until the transform panel is edited.
4. **Sphere/cylinder winding** matches the cube’s clockwise-from-outside convention but was not GPU-smoke-tested (back-face cull could hide a sign error).

### Deviations from brief

- Branched in `ps_main` (calls `ps_checker`) instead of compiling a second pixel shader; `ShaderSet.cpp` unchanged.
- Also added `D3D11Renderer.h` to the commit (new mesh/RS members).
- Step 5 `git push origin main` omitted (local commit only, stay on `feat/dx11-lab-teach`).

## Next Task Readiness

Phase 2 shading and multi-object draw are on `TeachingState`. Task 12 can add frustum/axes/tracker after the same mesh pass.
