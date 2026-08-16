# Viewport Polish Task 4 Report

**Status:** DONE  
**Commit:** `234c607` — Apply DX11-Study-inspired Fusion QSS theme.

## What changed

### Fusion + QSS (`src/app/app.qss`, `src/main.cpp`)
- `QApplication::setStyle("Fusion")` before `MainWindow` construction.
- Loads `app.qss` from the executable directory (`applicationDirPath()/app.qss`).
- Palette matches DX11-Study `:root`: bg-deep `#0f0f1a`, bg-panel `#1a1a2e`, bg-section `#16213e`, bg-cell `#0d0d1a`, border `#2a2a4a`, text `#ccd6f6`, text-dim `#8892b0`, highlight `#61dafb`.
- 5px border-radius on buttons, docks, spinboxes, combos, plain-text matrix cells.
- Consolas monospace for `QPlainTextEdit` / matrix boards.
- Dark scrollbars, menus, toolbar, tabs, checkboxes aligned to the same palette.

### CMake deploy (`CMakeLists.txt`)
- POST_BUILD copies `src/app/app.qss` next to `3DEditMax.exe` via `copy_if_different`.

### W/V/P/MVP accents (GroupBox object names + QSS)
- **Transform panel:** `sectionWorld` `#ff6b6b`, `sectionView` `#4ecdc4`, `sectionProjection` `#f7b731` — 3px left bar + tinted titles.
- **Matrix board:** `sectionMatrixW`, `sectionMatrixV`, `sectionMatrixP`, `sectionMVP` (`#a55eea`) with matching accents.

### Demo button (`src/ui/TutorialPanel.cpp`)
- `btnDemo`: gold border/text `#f7b731`; hover and pressed use `#61dafb`.

## Test results

- **Build:** PASS — MSVC via `vcvars64.bat`, Ninja Debug, `QT_DIR=D:\soft\qt5152\5.15.2\msvc2019_64`.
- **Deploy:** PASS — `build/app.qss` present (7337 bytes).
- **ctest:** PASS — `3deditmax_tests` (1/1, 0.03s).
- **Manual visual:** Not run (no GUI session in agent environment). Expected: dark rounded chrome, opaque color-matched docks, W/V/P/MVP accent bars, gold demo button.

## Concerns

- QSS is file-based; if `app.qss` is missing beside the exe, the app falls back to plain Fusion (no error dialog).
- Qt `QGroupBox` title positioning varies slightly by platform/style; left accent bar is the primary visual cue.
- Dock title bars use Fusion subcontrols; float/close buttons are styled but remain platform-dependent.
- Global `QWidget { background-color: #0f0f1a }` may affect native child widgets (e.g. D3D viewport HWND container); viewport area should remain unaffected as HWND paints independently.

## Out of scope (confirmed untouched)

Teaching state machine, row/column-major upload rules, JSON I/O, render-thread model, D3D11 shaders/renderer, MSAA, debug draw, mesh materials.

## Files changed

| File | Change |
|------|--------|
| `src/app/app.qss` | New DX11-Study Fusion QSS theme |
| `src/main.cpp` | Fusion style + load QSS |
| `CMakeLists.txt` | POST_BUILD copy `app.qss` |
| `src/ui/TransformPanel.cpp` | GroupBox object names for W/V/P accents |
| `src/ui/MatrixBoardPanel.cpp` | GroupBox object names for matrix accents |
| `src/ui/TutorialPanel.cpp` | `btnDemo` object name |

Local commit only; not pushed. Report stays untracked.
