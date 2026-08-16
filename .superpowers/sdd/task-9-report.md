# Task 9 Report: Matrix format + Phase 1 panels

## Status

**DONE**

## Summary

Added `formatMatrix4` (column/row display from `XMStoreFloat4x4` with no extra transpose), `TransformPanel` (World / View / Projection), `MatrixBoardPanel` (M_W M_V M_P MVP), and MainWindow left/right docks plus toolbar 列主序/重置. TDD: RED was missing `MatrixFormat.h` (`C1083`); GREEN CTest 1/1 passed. Local commit only; not pushed.

## Files Created

| File | Purpose |
|------|---------|
| `src/core/MatrixFormat.h` | `MajorOrder` + `formatMatrix4` |
| `src/core/MatrixFormat.cpp` | `%.3f` lines; column = `_11 _21 _31 _41` first |
| `src/ui/TransformPanel.h` | `setState` / `state` / `changed()` |
| `src/ui/TransformPanel.cpp` | Spin boxes, view presets, proj combo |
| `src/ui/MatrixBoardPanel.h` | `setMajorOrder` + `setMatrices` |
| `src/ui/MatrixBoardPanel.cpp` | Four monospace `QPlainTextEdit` blocks |

## Files Modified

| File | Change |
|------|--------|
| `tests/test_transforms.cpp` | Identity + translation-X=2 format checks |
| `src/app/MainWindow.h` | Docks, toolbar, `m_major` |
| `src/app/MainWindow.cpp` | `dockTransforms` / `dockMatrix` split above debug; board from `Build*` + `XMStoreFloat4x4` |
| `CMakeLists.txt` | Panels + `MatrixFormat.cpp` on app; format cpp on tests |

## Files Not Modified

- `src/render/D3D11Renderer.cpp` — GPU still transposes before `XMStoreFloat4x4`; board does not
- `src/ui/Dx11ViewportWidget.*` — `publishState` unchanged (Task 10 orbit)

## TDD Evidence

### RED (Step 1)

Wrote format assertions in `tests/test_transforms.cpp` **before** `MatrixFormat.h` existed.

```
E:\code\private\3DEditMax\tests\test_transforms.cpp(2): fatal error C1083:
无法打开包括文件: “core/MatrixFormat.h”: No such file or directory
```

Failure reason: feature missing, not a typo.

### GREEN (Step 2+)

Implemented `formatMatrix4` with `sprintf` `%.3f`. Column-major line 0 is `_11 _21 _31 _41` (translation X=2 → last number `2.000`).

```
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
| `1002a1a` | Add transform docks and column-major matrix board. |

Local commit only; not pushed. Not amended.

## Self-Review

### Requirements coverage

- [x] `MajorOrder` + `formatMatrix4(..., char out[4][64])`
- [x] Column-major lines = columns; no extra transpose vs `XMStoreFloat4x4`
- [x] `TransformPanel::setState` does not emit `changed()` (`m_block`)
- [x] Presets Front `(0,0)` Side `(0,90)` Top `(89,0)` Iso `(20,45)`
- [x] `MatrixBoardPanel` M_W M_V M_P MVP + `setMajorOrder`
- [x] Left `dockTransforms`, right `dockMatrix` above debug via `splitDockWidget`
- [x] Toolbar 列主序/行主序 flips board only (no `publishState`)
- [x] Reset → `teachingStateDefault()` + panel + viewport + board
- [x] Panels except viewport: no `windows.h` / D3D
- [x] Chinese via `QString::fromUtf8` (hex bytes, same as 视图)

### Concerns

1. **Manual Step 6 not run in-session.** Format is CTest-covered; slider→cube and 列主序-does-not-redraw-viewport were not interactively clicked.
2. **Distance range is 0.5–50** (plan + Task 10 wheel clamp), not brief `0.5–20`.
3. **Left dock may overflow** at 1280×720 (nine World rows + View + Projection, no scroll).
4. **Board aspect** uses viewport widget size when `aspectFollowViewport`; before first show that can differ slightly from the swapchain.

### Deviations from brief

- Distance max 50 (plan / Task 10), not 20.
- Step 7 `git push origin main` omitted (local commit only, stay on `feat/dx11-lab-teach`).

## Next Task Readiness

`TransformPanel::setState` is silent so Task 10 can push orbit camera back into the panel without `changed()` recursion.
