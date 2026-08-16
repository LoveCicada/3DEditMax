### Task 9: Matrix format + Phase 1 panels

**Files:**
- Create: `src/core/MatrixFormat.h`
- Create: `src/core/MatrixFormat.cpp`
- Create: `src/ui/TransformPanel.h`
- Create: `src/ui/TransformPanel.cpp`
- Create: `src/ui/MatrixBoardPanel.h`
- Create: `src/ui/MatrixBoardPanel.cpp`
- Modify: `src/app/MainWindow.*` 鈥?left/right docks, toolbar 鍒椾富搴?閲嶇疆
- Modify: `src/render/D3D11Renderer.cpp` 鈥?already uploads W V P WVP (Task 5)
- Modify: `tests/test_transforms.cpp` 鈥?format tests
- Modify: `CMakeLists.txt`

**Interfaces:**
- Produces:
  - `enum MajorOrder { MajorColumn = 0, MajorRow = 1 };`
  - `void formatMatrix4(const DirectX::XMFLOAT4X4& m, MajorOrder order, char out[4][64])` 鈥?each line 4 floats
  - `TransformPanel` edits `TeachingState` via `void setState(const TeachingState&)` / `TeachingState state() const` / signal `void changed()`
  - `MatrixBoardPanel::setMatrices(const XMFLOAT4X4& w,v,p,wvp, MajorOrder)`

- [ ] **Step 1: Format tests**

Identity stored with `XMStoreFloat4x4`. Column-major display first line is `1 0 0 0` (elements `_11 _21 _31 _41`). Row-major first line is `1 0 0 0` (`_11 _12 _13 _14`). For a translation-X=2 matrix, column-major last line is `2 0 0 1` if we print columns as rows... Specify exactly:

Column-major display prints 4 lines = 4 columns:

- line 0: `_11 _21 _31 _41`
- line 1: `_12 _22 _32 _42`
- line 2: `_13 _23 _33 _43`
- line 3: `_14 _24 _34 _44`

Row-major display:

- line 0: `_11 _12 _13 _14`
- ...

Translation X=2 鈫?`_41=2` 鈫?column-major line 0 last number is `2`.

- [ ] **Step 2: Implement `formatMatrix4` with `sprintf` of `%.3f`**

- [ ] **Step 3: TransformPanel**

`QGroupBox` World / View / Projection. `QDoubleSpinBox` ranges: pos 卤50 step 0.1; angles 卤180 step 1; scale 0.01鈥?0; distance 0.5鈥?0; fov 10鈥?20; near 0.01鈥?0; far 1鈥?000. Preset buttons write Front `(pitch=0,yaw=0)`, Side `(0,90)`, Top `(89,0)`, Iso `(20,45)`. Perspective/Ortho `QComboBox`. Changing any control emits `changed()`.

- [ ] **Step 4: MatrixBoardPanel**

Four `QPlainTextEdit` or `QLabel` monospace blocks titled M_W M_V M_P MVP. `setMatrices` formats with current `MajorOrder` from a setter `setMajorOrder`.

- [ ] **Step 5: MainWindow wiring**

Keep `m_teaching` / `m_lab`. On `TransformPanel::changed`: `m_teaching = panel->state(); m_viewport->publishState(...)`; recompute W/V/P on UI thread with `Build*` and `XMStoreFloat4x4` for the board (same functions as GPU). Toolbar toggle flips `m_major` and refreshes board. Reset sets `teachingStateDefault()` and updates panel + viewport.

Left dock objectName `dockTransforms`. Right dock `dockMatrix` above debug (use `splitDockWidget`).

- [ ] **Step 6: Manual** 鈥?sliders move the cube; board numbers change; 鍒椾富搴?琛屼富搴?only changes board, not the picture.

- [ ] **Step 7: Commit and push**

```bash
git add src/core/MatrixFormat.h src/core/MatrixFormat.cpp src/ui/TransformPanel.h src/ui/TransformPanel.cpp src/ui/MatrixBoardPanel.h src/ui/MatrixBoardPanel.cpp src/app/MainWindow.h src/app/MainWindow.cpp tests/test_transforms.cpp CMakeLists.txt
git commit -m "Add transform docks and column-major matrix board."
git push origin main
```

---

