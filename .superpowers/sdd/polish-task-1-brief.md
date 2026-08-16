### Task 1: Stop flicker

**Files:**
- Modify: `src/render/D3D11Renderer.cpp` (`resize`)
- Modify: `src/ui/Dx11ViewportWidget.h` / `.cpp` (orbit flag + throttle signal)
- Modify: `src/app/MainWindow.cpp` / `.h` (throttled panel refresh)
- Modify: `src/ui/MatrixBoardPanel.cpp` (fixed min height on matrix edits)

- [ ] **Step 1:** In `D3D11Renderer::resize`, if `w == m_w && h == m_h` and RTV/DSV valid, return true without releasing RTV.
- [ ] **Step 2:** Viewport: set `m_orbiting` on LMB press; clear on release. While orbiting, `commitTeaching` only `publishState`; schedule/emit `teachingEdited` via 50 ms timer (or MainWindow timer). On release, emit immediately.
- [ ] **Step 3:** MainWindow: on throttled/edited path, `setState` panels + `refreshBoard` / `refreshTracker` as today.
- [ ] **Step 4:** Matrix `QPlainTextEdit`s: set a fixed minimum height so digit churn does not resize the central widget by 1 px.
- [ ] **Step 5:** Manual: drag viewport 鈥?no black flash; release 鈥?sliders/matrices match camera. Real window resize still works.
- [ ] **Step 6:** Commit: `Fix orbit flicker by skipping no-op resize and throttling panel refresh.`

---

