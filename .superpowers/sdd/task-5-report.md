# Task 5 Report: D3D11Renderer clear + cube (Phase 0 GPU)

## Status

**DONE**

## Summary

Wired a D3D11 device/swapchain renderer, immutable cube mesh, runtime `unlit.hlsl` compile, and W/V/P helpers into `RenderThread` for `CmdInit` / `CmdResize` / `CmdReloadShader` / `CmdStop`. App target now links `d3d11 dxgi d3dcompiler` and copies `assets/shaders` next to the exe. Tests stay CPU-only (`D3DEDITMAX_NO_D3D`) and still construct `RenderThread()`. Local commit only.

## Files Created

| File | Purpose |
|------|---------|
| `assets/shaders/unlit.hlsl` | VS/PS unlit; FrameCB W/V/P/WVP; normal-tinted color |
| `src/teach/Transforms.h` | `BuildWorld` / `BuildView` / `BuildProjection` |
| `src/teach/Transforms.cpp` | DirectXMath S*R*T, look-at LH, persp/ortho |
| `src/render/MeshGpu.h` | 32-byte `MeshVertex`; cube/sphere/cylinder |
| `src/render/MeshGpu.cpp` | 24-vert / 36-index cube; sphere/cylinder empty |
| `src/render/ShaderSet.h` | Runtime HLSL compile + input layout |
| `src/render/ShaderSet.cpp` | `D3DCompileFromFile` vs_main/ps_main; keep prior on fail |
| `src/render/D3D11Renderer.h` | Device, swapchain, draw, reload |
| `src/render/D3D11Renderer.cpp` | Init/resize/render/Present + device-lost recover |

## Files Modified

| File | Change |
|------|--------|
| `src/render/RenderThread.h` | `explicit RenderThread(const std::wstring& shaderDir)`; default ctor kept; owns `D3D11Renderer` when not `D3DEDITMAX_NO_D3D` |
| `src/render/RenderThread.cpp` | Dispatch Init/Resize/ReloadShader/Stop; render + `FbFps` every 30 frames; 1 ms sleep only when not presenting |
| `CMakeLists.txt` | App sources + `d3d11 dxgi d3dcompiler`; post-build shader copy; tests get Transforms + `D3DEDITMAX_NO_D3D` |

## Files Not Modified

- `tests/test_queues.cpp` / `tests/test_main.cpp` — still `RenderThread rt;`
- UI / `MainWindow` — no HWND yet (Task 6)

## Build & Test

**Environment**

- Branch: `feat/dx11-lab-teach`
- Compiler: MSVC 14.33 (`cl.exe` via `vcvars64.bat`)
- Generator: existing `build/` Ninja Debug
- `QT_DIR`: `D:\soft\qt5152\5.15.2\msvc2019_64`

```bat
call D:\soft\vs2022\ide\VC\Auxiliary\Build\vcvars64.bat
set QT_DIR=D:\soft\qt5152\5.15.2\msvc2019_64
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

**Result:**

| Artifact / Test | Result |
|-----------------|--------|
| `3DEditMax.exe` linked | PASS (`build/3DEditMax.exe`) |
| Post-build `build/shaders/unlit.hlsl` | PASS |
| `3deditmax_tests.exe` linked (no D3D11Renderer.cpp) | PASS |
| CTest `3deditmax_tests` | **Passed** (1/1, 0.05 s) |
| `100% tests passed, 0 tests failed out of 1` | PASS |
| GPU present / HWND | Not run (Task 6) |

## Commit

| SHA | Subject |
|-----|---------|
| `55bccf9` | Add D3D11 renderer, cube mesh, and unlit shader. |

Local commit only; not pushed (per task constraints). Not amended.

## Self-Review

### Requirements coverage

- [x] `unlit.hlsl` verbatim from brief
- [x] `BuildWorld` / `BuildView` / `BuildProjection` as specified
- [x] `MeshGpu::createCube` 32-byte verts, 24/36, `D3D11_USAGE_DEFAULT`; sphere/cylinder empty
- [x] `ShaderSet::compileFromFile(device, path, fb)`; VS/PS + POSITION/NORMAL/TEXCOORD layout; keep previous on fail
- [x] `D3D11Renderer::initialize/shutdown/resize/render/reloadShaders`; `ComPtr`; debug retry + `FbWarn`
- [x] Hardware adapter (skip `DXGI_ADAPTER_FLAG_SOFTWARE`); swapchain DISCARD; D24S8; FrameCB dynamic 256 bytes
- [x] Render object 0 only; `XMStoreFloat4x4`; `Present(1,0)`; device-lost → `FbDeviceLost` + reinit or `m_dead`
- [x] `RenderThread(const std::wstring& shaderDir)` + default ctor (`shaderDir` empty)
- [x] Command wiring Init/Resize/ReloadShader/Stop; vsync present; 1 ms sleep when not initialized
- [x] App links `Qt5::Widgets d3d11 dxgi d3dcompiler`; shader copy; tests do not compile `D3D11Renderer.cpp`

### Concerns

1. **No HWND this task.** `initialize` / Present are compiled but not runtime-tested. CreateSwapChain will fail until Task 6 owns a window.
2. **`D3DEDITMAX_NO_D3D` on tests.** Required so `3deditmax_tests` can keep `RenderThread.cpp` without linking D3D. The GPU branch of `threadMain` is not compiled in CTest.
3. **Default `shaderDir` is empty.** `RenderThread()` still works for queue tests. Real Init must pass `applicationDirPath()/shaders` from the viewport widget (Task 6).
4. **Cube winding is unverified.** Clockwise-from-outside assumed for default D3D11 front face. Wrong winding would cull the cube once a swapchain exists.
5. **Shader compile failure aborts `initialize`.** Device is shut down; no `FbDeviceOk`. Reload keeps the previous set.
6. **App still has no viewport.** `3DEditMax.exe` links the renderer but UI does not construct `RenderThread` yet.

### Deviations from brief

- Kept `RenderThread()` delegating to `RenderThread(std::wstring())` so existing tests compile without edits.
- Tests define `D3DEDITMAX_NO_D3D` instead of compiling `D3D11Renderer.cpp` (explicitly forbidden on the test target).
- `ShaderSet::compileFromFile` uses one path + hardcoded `vs_main`/`ps_main` (Step 4), not separate vs/ps entry arguments from the interface blurb.
- Step 8 `git push origin main` omitted (local commit only; stay on `feat/dx11-lab-teach`).

## Next Task Readiness

Renderer is ready for Task 6 to create `Dx11ViewportWidget`, pass HWND + `shaderDir`, and send `CmdInit`.

## Review Fix (Important): failed resize still presenting

Failed `resize` reset RTV/DSV then `render` still called `Present`; `RenderThread` ignored the resize return.

**Code**

- `D3D11Renderer::render` early-outs if `!m_rtv || !m_dsv`.
- `ResizeBuffers` `DEVICE_REMOVED` / `DEVICE_RESET` reuses `handlePresentResult`.
- Other resize failures push `FbError`, clear partial views, return false.
- `RenderThread` sets `allowPresent = false` on failed resize/init and requires `viewsValid()` before present/FPS.

### Commands

```bat
call D:\soft\vs2022\ide\VC\Auxiliary\Build\vcvars64.bat
set QT_DIR=D:\soft\qt5152\5.15.2\msvc2019_64
cmake --build build --target 3DEditMax 3deditmax_tests
ctest --test-dir build -R 3deditmax_tests --output-on-failure
```

### Output

```
**********************************************************************
** Visual Studio 2022 Developer Command Prompt v17.3.2
** Copyright (c) 2022 Microsoft Corporation
**********************************************************************
[vcvarsall.bat] Environment initialized for: 'x64'
[1/9] Automatic MOC for target 3deditmax_tests
[2/8] Automatic MOC for target 3DEditMax
[3/7] Building CXX object CMakeFiles\3deditmax_tests.dir\src\render\RenderThread.cpp.obj
[4/7] Building CXX object CMakeFiles\3DEditMax.dir\src\render\RenderThread.cpp.obj
[5/7] Building CXX object CMakeFiles\3DEditMax.dir\src\render\D3D11Renderer.cpp.obj
[6/7] Linking CXX executable 3deditmax_tests.exe
[7/7] Linking CXX executable 3DEditMax.exe
Internal ctest changing into directory: E:/code/private/3DEditMax/build
Test project E:/code/private/3DEditMax/build
    Start 1: 3deditmax_tests
1/1 Test #1: 3deditmax_tests ..................   Passed    0.07 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.07 sec
```

MSVC `/showIncludes` notes omitted (not errors). Exit code 0.

| Artifact / Test | Result |
|-----------------|--------|
| `3DEditMax.exe` | PASS (`build/3DEditMax.exe`) |
| `3deditmax_tests.exe` | PASS |
| CTest `3deditmax_tests` | **Passed** (1/1, 0.07 s) |

**Commit:** `3a154f3` Skip Present after failed D3D11 resize. Local only; not amended; not pushed.
