# Viewport Polish Task 3 Report

**Status:** DONE  
**Commit:** `29ca7b6` — Align viewport materials and helpers with DX11-Study look.

## What changed

### Clear + blend (`src/render/D3D11Renderer.cpp`)
- Clear color is `#0a0a18`.
- Created `SrcAlpha` / `InvSrcAlpha` blend state and bound it with the mesh pipeline.

### Ground grid + world axes (`src/render/DebugDraw.cpp`)
- 20×20 grid on Y=0, size 20 (lines from −10 to +10). Center lines `#444466`, others `#333355`.
- World axes length 3 with RGB `#ff4444` / `#44ff44` / `#4444ff`.
- Dynamic line VB raised to 256 verts (grid + frustum + axes + tracker).
- `LineCB` gained `colorMul`; existing debug lines use `(1,1,1,1)`.
- Task 2 negative `DepthBias` / `SlopeScaledDepthBias` and depth-write-zero kept.

### Solid Lambert + companions (`assets/shaders/unlit.hlsl`)
- World-space Lambert: ambient 0.4 + directional `(5,10,5)` intensity 0.8 + weak emissive.
- Primary `#4a90d9` α 0.9, emissive `#1a3a5c`.
- LayoutThree companions `#38b889` / `#d98c3f` (α 0.82) with matching emissives.
- Normal and Checker kept; checker uses `#f2f2f2` / `#2c5f96` under the same Lambert.

### Silhouette edges (`src/teach/MeshBuild.cpp`, renderer)
- `buildSilhouetteEdges` keeps unique geometric edges whose face-normal angle exceeds 1° (cube: 12 edges, no triangle diagonals).
- Drawn after each mesh with `#88ccff` (companions `#72f2c6` / `#ffc178`). Hidden in Wire mode.

### Axis cones + Qt legend
- Cone mesh (r=0.15, h=0.36, 8 slices) at each axis tip. No 3D text sprites.
- Native-child `QFrame` overlay at viewport top-left, styled like DX11-Study `#axis-legend`. Mouse-transparent.

### Sphere tessellation
- Renderer sphere is 32×24 (was 16×24). CTest expects 32×24 vertex count and cube edge count 12.

## Test results

- **Build:** PASS — MSVC via `D:\soft\vs2022\ide\VC\Auxiliary\Build\vcvars64.bat`, Ninja Debug, `QT_DIR=D:\soft\qt5152\5.15.2\msvc2019_64`.
- **ctest:** PASS — `3deditmax_tests` (1/1, 0.06s).
- **Manual visual:** Not run (no GUI session in agent environment). Expected: dark `#0a0a18` clear, translucent blue box + cyan edges, 20×20 grid, RGB cone axes, Qt legend; LayoutThree green/orange readable.

## Concerns

- D3D11 line width is 1 px; “thicker” axes come from length-3 lines + cones (same as WebGL `LineBasicMaterial`).
- Axis legend is a native child HWND over the D3D swapchain; stacking can be driver/Qt-version sensitive.
- Translucent meshes still write depth (Three.js default), so objects behind a cube are fully occluded, not softly seen through.
- Sphere 32×24 edge buffers are larger than the old 16×24 mesh; cost is one extra line draw per object.
- Companion edge colors follow DX11-Study wire colors; brief named `#88ccff` for the primary outline.

## Out of scope (confirmed untouched)

Teaching state machine, row/column-major upload rules, JSON I/O, render-thread model (`std::thread` + `CommandQueue`), MSAA sample selection, Task 4 QSS theme.

## Files changed

| File | Change |
|------|--------|
| `assets/shaders/unlit.hlsl` | Lambert + emissive + `baseColor`; world normals |
| `assets/shaders/line.hlsl` | `colorMul` |
| `src/render/D3D11Renderer.cpp` / `.h` | Clear, blend, materials, edges, cones, sphere 32×24 |
| `src/render/DebugDraw.cpp` / `.h` | Grid, length-3 axes, edge VB + `drawLineList` |
| `src/render/MeshGpu.cpp` / `.h` | `createCone` |
| `src/teach/MeshBuild.cpp` / `.h` | `buildSilhouetteEdges`, `buildCone` |
| `src/ui/Dx11ViewportWidget.cpp` / `.h` | Axis legend overlay |
| `tests/test_transforms.cpp` | Sphere 32×24; cube 12 edges; cone non-empty |

Local commit only; not pushed. Report stays untracked.
