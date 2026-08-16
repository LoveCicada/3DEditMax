# Pan Task 3 Report: Light layout looseness

## Status

**DONE**

## Summary

Default dock sizes are applied on first show via `resizeDocks` (left column ~300px, right ~380px, bottom ~140px). Transform and Object panel `QGroupBox` sections (World / View / Projection / Mesh / Scene) are checkable and collapse their contents when unchecked. Dock/group box QSS padding and margins are slightly reduced while keeping the Fusion dark palette. `docs/ui/01-lab-shell-layout.md` already documents RMB pan — no edit required.

## Checklist

| # | Item | Result | Evidence |
|---|------|--------|----------|
| 1 | Default dock widths/heights via `resizeDocks` | **PASS** | Left 300, right 380, bottom 140; deferred `QTimer::singleShot(0)` |
| 2 | Docks remain movable/closable; View toggles | **PASS** | No change to dock features or View menu |
| 3 | Transform World/View/Projection checkable | **PASS** | `setCheckable(true)` + hide direct children on toggle |
| 4 | Object Mesh/Scene checkable | **PASS** | Same collapsible helper |
| 5 | QSS: less rigid dock/group padding | **PASS** | Dock title 4×6px; group margin 8px, padding 6px |
| 6 | Doc mentions RMB pan | **PASS** | Line 55: 右键平移 |
| 7 | No camera/pan/render core changes | **PASS** | UI/layout/QSS only |
| 8 | CTest | **PASS** | `1/1 Test #1: 3deditmax_tests Passed` |

## Files Modified

| File | Change |
|------|--------|
| `src/app/MainWindow.cpp` | `resizeDocks` defaults for left/right/bottom |
| `src/ui/TransformPanel.cpp` | Collapsible World / View / Projection groups |
| `src/ui/ObjectPanel.cpp` | Collapsible Mesh / Scene groups |
| `src/app/app.qss` | Slightly tighter dock title and group box spacing |

## Files Not Modified

- `docs/ui/01-lab-shell-layout.md` — already has RMB pan
- Camera, pan, render thread, JSON, viewport interaction code

## Build & Test

**Environment**

- Branch: `feat/viewport-pan-layout`
- Compiler: MSVC 14.33 (`vcvars64.bat`)
- `QT_DIR`: `D:\soft\qt5152\5.15.2\msvc2019_64`
- Generator: existing `build/` Ninja Debug

```bat
call D:\soft\vs2022\ide\VC\Auxiliary\Build\vcvars64.bat
set QT_DIR=D:\soft\qt5152\5.15.2\msvc2019_64
cmake --build build --target 3deditmax_tests --clean-first
cmake --build build --target 3DEditMax
ctest --test-dir build --output-on-failure
```

```
1/1 Test #1: 3deditmax_tests ..................   Passed    0.06 sec
100% tests passed, 0 tests failed out of 1
```

## Commit

| SHA | Subject |
|-----|---------|
| `15e6959` | Loosen lab shell docks with defaults and collapsible transform sections. |

Full: `15e6959dc4c59cf33cdc763f7b8a00bd24734ab5`

Local commit only; not pushed.

## Self-Review

- [x] Task 3 scope only (no camera/pan/render changes)
- [x] C++11, Qt 5.15
- [x] View menu dock toggles unchanged
- [ ] Interactive dock resize / collapse smoke not verified by human this pass
