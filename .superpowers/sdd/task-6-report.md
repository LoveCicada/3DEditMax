# Task 6 Report: Viewport widget, docks, debug log (Phase 0 UI)

## Status

**DONE**

## Summary

Embedded D3D11 in a Qt central viewport. `Dx11ViewportWidget` owns `RenderThread` (start on `showEvent`, `requestStopAndJoin` on `hideEvent` / destructor), publishes `StateSnapshot` defaults, and exposes `feedback()`. Right dock `DX11 Lab / Debug` hosts a read-only `DebugLogPanel` drained by a 100 ms `QTimer` (no `QMetaObject::invokeMethod`). View menu toggles the dock. Local commit only.

## Files Created

| File | Purpose |
|------|---------|
| `src/ui/Dx11ViewportWidget.h` | Native HWND widget; `publishState` / `feedback` |
| `src/ui/Dx11ViewportWidget.cpp` | WA_* attributes; `paintEngine()=0`; Init/Resize/Stop |
| `src/ui/DebugLogPanel.h` | `drain` + `appendFromQueue` |
| `src/ui/DebugLogPanel.cpp` | Read-only `QPlainTextEdit`; `[Error]`/`[Warn]`/`[Log]`; skip `FbFps` |

## Files Modified

| File | Change |
|------|--------|
| `src/app/MainWindow.h` | Viewport, log, poll timer, teaching/lab state |
| `src/app/MainWindow.cpp` | Central widget, right debug dock, 视图 menu, 100 ms poll |
| `CMakeLists.txt` | App sources + headers for AUTOMOC |

## Files Not Modified

- Render / teach / tests — already consume `RenderThread` + snapshot defaults
- Other `src/ui` panels — none exist; `DebugLogPanel` does not include D3D or `windows.h`

## Build & Test

**Environment**

- Branch: `feat/dx11-lab-teach`
- Compiler: MSVC 14.33 (`cl.exe` via `vcvars64.bat`)
- Generator: existing `build/` Ninja Debug
- `QT_DIR`: `D:\soft\qt5152\5.15.2\msvc2019_64` (env had `...\bin`; CMake strips it)

```bat
call D:\soft\vs2022\ide\VC\Auxiliary\Build\vcvars64.bat
cmake -S . -B build
cmake --build build --target 3DEditMax 3deditmax_tests
ctest --test-dir build --output-on-failure
```

**Result:**

| Artifact / Test | Result |
|-----------------|--------|
| `3DEditMax.exe` linked | PASS (`build/3DEditMax.exe`) |
| Post-build `build/shaders/unlit.hlsl` | PASS |
| CTest `3deditmax_tests` | **Passed** (1/1, 0.02 s) |
| `100% tests passed, 0 tests failed out of 1` | PASS |
| GUI smoke (3 s then kill) | Process stayed up (`pid=23268`); no interactive cube/log inspect |

First compile failed `C2668` on unqualified `make_unique` (ADL vs `std::make_unique` from `std::wstring`). Fixed with `::make_unique`. Rebuild exit 0.

## Commit

| SHA | Subject |
|-----|---------|
| `0d8a85d` | Embed D3D11 swapchain in a Qt viewport with debug log polling. |

Local commit only; not pushed (per task constraints). Not amended.

## Self-Review

### Requirements coverage

- [x] Viewport attributes: `WA_NativeWindow`, `WA_DontCreateNativeAncestors`, `WA_OpaquePaintEvent`, `WA_NoSystemBackground`; no auto-fill; min 320×180
- [x] `paintEngine()` returns 0; `paintEvent` empty
- [x] `hwnd()` = `reinterpret_cast<HWND>(winId())`
- [x] `startRenderer` after `showEvent` + `winId()`; shader dir `applicationDirPath()/shaders`; `CmdInit` + `publishState`
- [x] `publishState` copies members; snapshot `viewportW/H` = widget size
- [x] `resizeEvent` pushes `CmdResize` then `publishState` if thread exists
- [x] `stopRenderer` = `requestStopAndJoin` + `reset`; hide before `QWidget::hideEvent`; destructor
- [x] Only `Dx11ViewportWidget` among UI files includes `<windows.h>` (cpp); header pulls it via `RenderThread.h` as specified
- [x] `DebugLogPanel` has no D3D / `windows.h`; `drain` formats Error/Warn/Log; ignores `FbFps`
- [x] `appendFromQueue` aliases `drain` (Produces name + Step 2/3 `drain`)
- [x] MainWindow central + right dock `dockDebug` + 100 ms `QTimer` poll; 视图 + `toggleViewAction`
- [x] `core/MakeUnique.h` / `::make_unique`; no `QMetaObject::invokeMethod`
- [x] New sources listed on `3DEditMax` for AUTOMOC

### Concerns

1. **No visual GPU checklist.** Smoke test only proved the process stayed alive. Cube, clear color, and log line `D3D11 device ready` were not inspected. Task 7 should confirm on a display.
2. **`hideEvent` tears down the device.** Minimize / hide / dock shuffle may `Stop`+`join` and drop unread feedback. `startRenderer` no-ops if already running to avoid a double device.
3. **`feedback()` while stopped.** After hide, `m_thread` is null. Returns a function-local static empty queue so the 100 ms poll cannot dereference null. Hidden-session messages are discarded with the thread.
4. **`FbDeviceLost` / `FbDeviceOk` are not printed.** Drain only tags Error/Warn/Log; those kinds are skipped (DeviceOk text is empty anyway).
5. **MainWindow transitively sees `windows.h`** via `Dx11ViewportWidget.h` → `RenderThread.h`. Allowed (`src/app`, not another UI panel).
6. **`::make_unique` required.** Unqualified `make_unique(std::wstring)` is ambiguous with `std::make_unique` under MSVC.

### Deviations from brief

- `::make_unique<RenderThread>(...)` instead of unqualified `make_unique` (C2668).
- `DebugLogPanel::appendFromQueue` added as a one-line alias of `drain` so both the Produces name and Step 3 `drain` call exist.
- `feedback()` / `publishState` / `startRenderer` null-thread guards (not in the header sketch).
- Step 5 `git push origin main` omitted (local commit only; stay on `feat/dx11-lab-teach`).

## Next Task Readiness

HWND + `CmdInit` now reach `D3D11Renderer`. Task 7 can run the Phase 0 visual checklist (clear color, cube, debug log, resize, clean exit).
