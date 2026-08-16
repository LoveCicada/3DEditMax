# Task 12 Report: Frustum, axes, tracker (Phase 2)

## Status

**DONE**

## Summary

`DebugDraw` compiles `line.hlsl` (`mul(float4(pos,1), viewProj)` + vertex color), maps a dynamic line VB each frame, and draws after meshes: yellow LH frustum (inv proj → inv view), RGB world axes of length 1, and a magenta octahedron at the `TrackPoint` world position. `TrackerPanel` (Qt only) has model XYZ spins, eight `(±1,±1,±1)` presets, and read-only world/view/clip/ndc labels refreshed from MainWindow via `TrackPoint`. Local commit only; not pushed.

## Files Created

| File | Change |
|------|--------|
| `assets/shaders/line.hlsl` | LineCB `viewProj`; VS `mul(float4, float4x4)`; vertex COLOR |
| `src/render/DebugDraw.h` | `create(device)` + `create(device, shaderDir)`; `draw(ctx, snap, view, proj)` |
| `src/render/DebugDraw.cpp` | Dynamic VB `MAP_WRITE_DISCARD`; frustum 12 edges; axes; octahedron |
| `src/ui/TrackerPanel.h` | Spins, presets, `setTrackResult`, `changed()` |
| `src/ui/TrackerPanel.cpp` | Qt-only; no `windows.h` / D3D |

## Files Modified

| File | Change |
|------|--------|
| `src/render/D3D11Renderer.h/.cpp` | `m_debug`; create after unlit; `draw` after mesh loop; reload line shader |
| `src/app/MainWindow.h/.cpp` | Right dock Tracker; `refreshTracker()` on every teaching change |
| `CMakeLists.txt` | `DebugDraw.cpp`, `TrackerPanel` on app target |

## Files Not Modified

- Post-build `copy_directory assets/shaders` — already copies `line.hlsl`
- `src/teach/Transforms.h` — `TrackPoint` already present
- `src/core/TeachingState.h` — `trackModel[3]` already present

## TDD Evidence

Task 12 brief has no new CTest. Existing `3deditmax_tests` (TrackPoint + mesh counts) stayed green. GPU frustum/axes are visual (Step 4).

## Build & Test

- Branch: `feat/dx11-lab-teach`
- MSVC 14.33 via `D:\soft\vs2022\ide\VC\Auxiliary\Build\vcvars64.bat`
- `QT_DIR`: `D:\soft\qt5152\5.15.2\msvc2019_64`
- `cmake --build build --target 3deditmax_tests 3DEditMax` exit 0
- CTest `3deditmax_tests` **Passed** (1/1)
- Post-build `build/shaders/line.hlsl` present

## Commit

| SHA | Subject |
|-----|---------|
| `fe53bd0` | Draw frustum, axes, and tracked model-space points. |

Local commit only; not pushed. Not amended.

## Self-Review

### Requirements coverage

- [x] `line.hlsl` VS `mul(float4(pos,1), viewProj)`; color in vertex
- [x] CPU `XMMatrixTranspose` before store of `viewProj` (same as unlit)
- [x] `DebugDraw::create` / `draw`; dynamic VB Map DISCARD
- [x] Frustum: 8 LH clip corners (z 0..1), inv(proj) then inv(view), 12 yellow edges
- [x] Axes at origin length 1, RGB = XYZ
- [x] Tracker: `TrackPoint` then octahedron at **world** position
- [x] TrackerPanel: 3 spins, 8 cube-corner presets `(±1,±1,±1)`, read-only world/view/clip/ndc
- [x] MainWindow refreshes labels via `TrackPoint` on any teaching change
- [x] TrackerPanel: no windows.h / D3D
- [x] Debug after meshes
- [x] C++11
- [x] Local commit only (no push)

### Concerns

1. **Manual Step 4 not run in-session.** Near/far / corner-gizmo were not clicked in the running app.
2. **Same-camera frustum is mostly screen-edge lines.** World-space camera frustum projects back to NDC ±1, so near/far resize the world pyramid but not the on-screen silhouette.
3. **Cube mesh is ±0.5, presets are ±1.** Default `trackModel` is `(1,1,1)`; the octahedron sits outside the unit-size cube unless spins are set to ±0.5 or scale is 2.
4. **`create(ID3D11Device*)` cannot find `line.hlsl`.** Added `create(device, shaderDir)`; renderer uses that. Bare `create(device)` is a no-op compile of `/line.hlsl`.

### Deviations from brief

- Extra `create(ID3D11Device*, const std::wstring& shaderDir)` so the shader path matches unlit.
- Also modified `D3D11Renderer.h` (new `m_debug` member).
- Step 5 `git push origin main` omitted (local commit only, stay on `feat/dx11-lab-teach`).

## Next Task Readiness

Frustum, axes, and tracker overlay the Phase 2 mesh pass. Task 13 can add tutorial/demo/JSON without changing the debug line pipeline.
