# Pan Task 1 Report: Stop drag flicker

## Status

**DONE**

## Summary

Left-drag orbit no longer emits `teachingEdited` while the mouse is down. Each move only calls `publishState` so the render thread updates the camera; matrix boards and dock layout stay untouched until LMB release, when `teachingEdited` fires once for MainWindow to sync Transform / Matrix / Tracker. The 50 ms throttle timer was removed as unnecessary. Existing no-op resize skip, matrix min height, `grabMouse`, and orbit math are unchanged. MainWindow was not modified.

## Checklist

| # | Item | Result | Evidence |
|---|------|--------|----------|
| 1 | Interaction state (`m_orbiting`) | **PASS** | LMB press sets `m_orbiting`; release clears it |
| 2 | Drag: `publishState` only, no emit | **PASS** | `commitTeaching()` skips `emit teachingEdited` when `m_orbiting` |
| 3 | Release: single `teachingEdited` | **PASS** | `mouseReleaseEvent` publishes then emits once |
| 4 | Remove 50 ms throttle | **PASS** | `QTimer`, `flushTeachingEdited`, timer wiring removed |
| 5 | Keep no-op resize + matrix min height | **PASS** | No edits to `D3D11Renderer::resize` or `MatrixBoardPanel.cpp` |
| 6 | Keep `grabMouse` + orbit math | **PASS** | LMB press/release grab unchanged; `applyOrbitDrag` unchanged |
| 7 | Manual drag smoke | **SKIPPED** | No interactive orbit session this pass |
| 8 | CTest | **PASS** | `1/1 Test #1: 3deditmax_tests Passed` |

## Files Modified

| File | Change |
|------|--------|
| `src/ui/Dx11ViewportWidget.h` | Removed `QTimer*`, `flushTeachingEdited` |
| `src/ui/Dx11ViewportWidget.cpp` | Drag silence: no timer, no emit while orbiting; emit on release only |

## Files Not Modified

- `src/app/MainWindow.cpp` / `.h` — existing `onTeachingEdited` path is sufficient
- `src/render/D3D11Renderer.cpp` — no-op resize already present from polish Task 1
- `src/ui/MatrixBoardPanel.cpp` — min height unchanged
- Render thread, teaching state machine, JSON, tests

## Build & Test

**Environment**

- Branch: `feat/viewport-pan-layout`
- Compiler: MSVC 14.33 (`vcvars64.bat`)
- `QT_DIR`: `D:\soft\qt5152\5.15.2\msvc2019_64`
- Generator: existing `build/` Ninja Debug

```bat
call D:\soft\vs2022\ide\VC\Auxiliary\Build\vcvars64.bat
set QT_DIR=D:\soft\qt5152\5.15.2\msvc2019_64
cmake --build build --target 3DEditMax 3deditmax_tests
ctest --test-dir build --output-on-failure
```

```
1/1 Test #1: 3deditmax_tests ..................   Passed    0.09 sec
100% tests passed, 0 tests failed out of 1
```

Note: initial link failed with `LNK1168` because `3DEditMax.exe` was running; killed process and rebuild succeeded.

## Commit

| SHA | Subject |
|-----|---------|
| `826adae` | Stop matrix board refresh during viewport drag to prevent flicker. |

Local commit only; not pushed.

## Self-Review

- [x] Task 1 scope only (no pan / layout work)
- [x] C++11, Qt 5.15
- [x] Wheel dolly still emits immediately (not orbiting)
- [ ] Interactive “no black flash” not verified by human this pass
