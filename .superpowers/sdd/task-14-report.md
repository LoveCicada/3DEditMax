# Task 14 Report: Lab panel (Phase 4)

## Status

**DONE_WITH_CONCERNS**

## Summary

Lab dock now has Device, Shader (unlit/normal/checker + Reload), Rasterizer (Solid/Wire, None/Back/Front, depth on/off), and a read-only CB observer. Panel writes `LabState` and `publishState`. `Dx11ViewportWidget::reloadShaders()` pushes `CmdReloadShader`. Renderer binds fill×cull RS and on/off DSS from `snap.lab` each frame; `ShadeWire` still forces wire. Failed reload keeps the previous PSO. CB observer is filled from the same UI-computed W/V/P/WVP as the matrix board (no GPU readback). After init, RenderThread pushes `FbLog` `Adapter: <utf8>`. Local commit only; not pushed.

## Files Created

| File | Change |
|------|--------|
| `src/ui/LabPanel.h/.cpp` | Qt-only lab controls; `changed` / `reloadShaders`; no windows.h / D3D |
| `tests/test_lab.cpp` | Defaults; fill (ShadeWire OR lab wire); cull clamp; shadeX from variant vs teaching |

## Files Modified

| File | Change |
|------|--------|
| `src/core/LabState.h` | `labSetShaderVariant`, `labEffectiveFillMode`, `labEffectiveCullMode`, `labShadeModeX` |
| `src/ui/Dx11ViewportWidget.h/.cpp` | `reloadShaders()` → `CmdReloadShader` |
| `src/ui/DebugLogPanel.h/.cpp` | Capture `Adapter: ` FbLog; `adapterName()` |
| `src/app/MainWindow.h/.cpp` | Lab+log in DX11 dock; wire panel; fill CB text; copy adapter into Device label |
| `src/render/D3D11Renderer.h/.cpp` | 2×3 RS + 2 DSS; bind from snapshot; variant compile; FrameCB FbLog once per shade change; `adapterNameUtf8` |
| `src/render/ShaderSet.h/.cpp` | `compileVariant` (`SHADER_VARIANT` macro, still `unlit.hlsl`) |
| `src/render/RenderThread.cpp` | After successful `CmdInit`, `FbLog` adapter name (wide→UTF-8) |
| `tests/test_main.cpp` | `runLabTests()` |
| `CMakeLists.txt` | LabPanel on app; `test_lab.cpp` on tests |

## TDD Evidence

### RED

`labEffectiveFillMode` / `labEffectiveCullMode` / `labShadeModeX` were not declared.

```
E:\code\private\3DEditMax\tests\test_lab.cpp(12): error C3861: labEffectiveFillMode: identifier not found
... same for labEffectiveCullMode, labShadeModeX
```

Failure reason: helpers missing, not a typo.

### GREEN

Helpers added in `LabState.h`. Rebuilt tests + app.

```
cmake --build build --target 3deditmax_tests 3DEditMax
...
[14/14] Linking CXX executable 3DEditMax.exe
1/1 Test #1: 3deditmax_tests ..................   Passed    0.02 sec
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
| `6a09b77` | Expose rasterizer, depth, shaders, and CB text in the lab dock. |

Local commit only; not pushed. Not amended.

## Self-Review

### Requirements coverage

- [x] LabPanel combo shaderVariant unlit/normal/checker, fill Solid/Wire, cull None/Back/Front, depth on/off
- [x] Changing panel writes LabState and publishState
- [x] RS/DSS created at init; bound from snapshot each frame
- [x] ShadeWire still forces wire; lab fill/cull/depth apply
- [x] Reload button → `Dx11ViewportWidget::reloadShaders()` → `CmdReloadShader`
- [x] Failed compile keeps previous shaders (assign only on success)
- [x] CB observer: read-only; MainWindow fills from same four board matrices; no GPU readback
- [x] Lab dock shows adapter description (Device label + FbLog)
- [x] RenderThread after init pushes FbLog adapter name (DXGI wide→UTF-8)
- [x] ShaderSet variants unlit/normal/checker (same `unlit.hlsl` + CB mode; combo exposed)
- [x] LabPanel: no windows.h / D3D
- [x] C++11
- [x] Local commit only (no push)

### Concerns

1. **Manual Step 3 not run in-session.** Wire/cull/depth visibility and the syntax-error reload / recover cycle were not exercised in the running app.
2. **Six rasterizer states, not three.** Fill×cull needs 2×3 precreated states plus two DSS. The brief said “three rasterizer states.”
3. **FrameCB FbLog is GPU-upload (transposed) floats**, logged once on first draw and when `shadeX` changes — not every orbit frame. The live CB box is board-order UI matrices.
4. **Variant files are not separate.** `normal`/`checker` compile `unlit.hlsl` with `SHADER_VARIANT` and set CB `shadingMode`; ObjectPanel Normal/Checker still win over the lab combo for PS mode.

### Deviations from brief

- Extra `tests/test_lab.cpp` (fill/cull/shade helpers).
- Also modified `D3D11Renderer.h`, `ShaderSet.h`, `LabState.h`, `DebugLogPanel`, tests (needed; not all in the brief `git add` list).
- Reset toolbar also restores default `LabState`.
- Step 4 `git push origin main` omitted (local commit only, stay on `feat/dx11-lab-teach`).

## Follow-up: failed-compile variant name (Important finding)

`D3D11Renderer::render` no longer calls `copyVariantName` when `compileVariant` / hot-swap fails. `m_shaderVariant` updates only after a successful compile so the next frame retries and UI/GPU cannot silently disagree.

### Re-test (MSVC vcvars64)

- Branch: `feat/dx11-lab-teach`
- MSVC 14.33 via `D:\soft\vs2022\ide\VC\Auxiliary\Build\vcvars64.bat`
- `QT_DIR`: `D:\soft\qt5152\5.15.2\msvc2019_64`
- `cmake --build build --target 3deditmax_tests 3DEditMax` exit 0
- `ctest --test-dir build -R 3deditmax_tests --output-on-failure`

```
1/1 Test #1: 3deditmax_tests ..................   Passed    0.02 sec
100% tests passed, 0 tests failed out of 1
Total Test time (real) =   0.02 sec
```
