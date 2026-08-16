# Viewport Polish Task 2 Report

**Status:** DONE  
**Commit:** `0f87071` — Add MSAA swapchain and depth-biased debug lines.

## What changed

### D3D11Renderer (`src/render/D3D11Renderer.cpp`, `.h`)
- Added `selectMultisampleLevel()` called after device creation.
- Probes `CheckMultisampleQualityLevels` for `DXGI_FORMAT_R8G8B8A8_UNORM` in order 4 → 2 → 1; uses first supported count with quality 0.
- Stores `m_sampleCount` / `m_sampleQuality` and applies them to both swapchain `SampleDesc` and depth texture `SampleDesc`.
- Kept `DXGI_SWAP_EFFECT_DISCARD` unchanged.
- Teaching wireframe / lab rasterizer states untouched (Back cull preserved for wire mode).

### DebugDraw (`src/render/DebugDraw.cpp`, `.h`)
- Creates dedicated rasterizer: `DepthBias = 100000`, `SlopeScaledDepthBias = -1.f`, `CullMode = NONE`.
- Creates depth-stencil state: depth test **ON** (`LESS`), `DepthWriteMask = ZERO`.
- Binds both states before line draw so axes, frustum, and tracker crosshair sit slightly in front of mesh depth without writing depth.

## Test results

- **Build:** PASS — MSVC via `D:\soft\vs2022\ide\VC\Auxiliary\Build\vcvars64.bat`, Ninja Debug, `QT_DIR=D:\soft\qt5152\5.15.2\msvc2019_64`.
- **ctest:** PASS — `3deditmax_tests` (1/1, 0.02s).
- **Manual visual:** Not run (no GUI session in agent environment). Expected: smoother silhouettes from MSAA; less z-fighting on world axes / frustum / tracker vs. mesh.

## Concerns

- MSAA level is chosen once at init; device-lost re-init re-probes (OK).
- Depth bias values (`100000` / `-1.f`) are heuristic — may need tuning per GPU or scene scale in a later pass.
- MSAA swapchain increases fill cost vs. 1×; acceptable per plan (no 8×).
- Silhouette edge outlines deliberately **not** implemented (Task 3 scope).

## Out of scope (confirmed untouched)

Teaching state machine, row/column-major rules, JSON I/O, render-thread model, wireframe cull mode, Task 3 outline path.
