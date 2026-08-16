# Task 10 Report: Orbit camera on the viewport (Phase 1)

## Status

**DONE**

## Summary

Left-drag orbits yaw/pitch and wheel dollies distance on `Dx11ViewportWidget`. Each edit writes `m_teaching`, `publishState`, and emits `teachingEdited`. MainWindow applies that to `TransformPanel::setState` (silent) and refreshes the matrix board. TDD: RED was missing `applyOrbitDrag` (`C3861`); GREEN CTest 1/1 passed. Local commit only; not pushed.

## Files Created

None.

## Files Modified

| File | Change |
|------|--------|
| `src/core/TeachingState.h` | `applyOrbitDrag` / `applyDollyWheel` (pitch clamp, yaw wrap, distance clamp) |
| `src/ui/Dx11ViewportWidget.h` | `teachingEdited`, mouse/wheel overrides, `m_lastMouse` |
| `src/ui/Dx11ViewportWidget.cpp` | `setMouseTracking` + `StrongFocus`; left-drag orbit; wheel dolly |
| `src/app/MainWindow.h` | `onTeachingEdited` slot |
| `src/app/MainWindow.cpp` | connect signal → `setState` + `refreshBoard` (no extra `publishState`) |
| `tests/test_transforms.cpp` | orbit deltas, pitch clamps, dolly in/out and [0.5, 50] clamps |

## Files Not Modified

- `src/ui/TransformPanel.*` — `setState` already silent (`m_block`) from Task 9
- No middle-mouse pan (look-at stays origin)

## TDD Evidence

### RED

Wrote orbit/dolly assertions in `tests/test_transforms.cpp` **before** the helpers existed.

```
E:\code\private\3DEditMax\tests\test_transforms.cpp(59): error C3861: “applyOrbitDrag”: 找不到标识符
E:\code\private\3DEditMax\tests\test_transforms.cpp(69): error C3861: “applyDollyWheel”: 找不到标识符
ninja: build stopped: subcommand failed.
```

Failure reason: feature missing, not a typo.

### GREEN

Implemented helpers in `TeachingState.h`. Viewport calls them, then `commitTeaching()`.

```
cmake --build build --target 3deditmax_tests 3DEditMax
ctest --test-dir build -R 3deditmax_tests --output-on-failure
1/1 Test #1: 3deditmax_tests ..................   Passed    0.05 sec
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
| `c73dfae` | Orbit the teaching camera from the DX11 viewport. |

Local commit only; not pushed. Not amended.

## Self-Review

### Requirements coverage

- [x] `signal void teachingEdited(const TeachingState&)` with `Q_OBJECT`
- [x] Left drag: yaw += dx * 0.3, pitch += dy * 0.3, pitch clamped [-89, 89]
- [x] Wheel: `camDistance *= (delta>0 ? 0.9f : 1.1f)`, clamp [0.5, 50]
- [x] No middle-mouse pan
- [x] Mouse tracking + `Qt::StrongFocus`
- [x] `publishState` then emit after writing `m_teaching`
- [x] MainWindow `setState` without `changed` recursion
- [x] Board refresh on camera edit

### Concerns

1. **Manual Step 4 not run in-session.** Math is CTest-covered; drag/wheel/slider sync was not interactively clicked.
2. **Native HWND mouse path.** `WA_NativeWindow` + swapchain may still swallow some Qt mouse/wheel events on some machines; if orbit feels dead, the next fix is a Win32 mouse hook or `nativeEvent`.
3. **Yaw wrap [-180, 180]** is extra vs the brief so `TransformPanel` spinboxes (range ±180) stay in sync after a long orbit.

### Deviations from brief

- Extracted `applyOrbitDrag` / `applyDollyWheel` into `TeachingState.h` for TDD (widget still owns events).
- Yaw wrapped to ±180 (see concern 3).
- Step 5 `git push origin main` omitted (local commit only, stay on `feat/dx11-lab-teach`).
- `TransformPanel` not touched (Task 9 already silent `setState`).

## Next Task Readiness

Viewport can push camera edits into the panel without recursion. Phase 2 (Task 11) can add meshes/layout/shading on the same `TeachingState` path.
