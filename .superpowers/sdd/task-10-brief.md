### Task 10: Orbit camera on the viewport (Phase 1)

**Files:**
- Modify: `src/ui/Dx11ViewportWidget.h`
- Modify: `src/ui/Dx11ViewportWidget.cpp`
- Modify: `src/app/MainWindow.h/.cpp` 鈥?connect camera edits back to `TransformPanel`

**Interfaces:**
- Produces: `signal void teachingEdited(const TeachingState&)`
- Left drag: yaw += dx * 0.3, pitch += dy * 0.3, pitch clamped [-89, 89]
- Wheel: `camDistance *= (delta>0 ? 0.9f : 1.1f)`, clamp [0.5, 50]
- Middle drag: optional skip in this task (do not implement pan; keep look-at origin)

- [ ] **Step 1: Enable mouse tracking, `setFocusPolicy(Qt::StrongFocus)`**
- [ ] **Step 2: Implement `mousePressEvent` / `mouseMoveEvent` / `wheelEvent` writing `m_teaching` then `publishState` and emit `teachingEdited`**
- [ ] **Step 3: MainWindow `setState` on TransformPanel without extra `changed` recursion** 鈥?`TransformPanel::setState` must not emit `changed`
- [ ] **Step 4: Manual** 鈥?drag orbits, wheel dolly, sliders stay in sync
- [ ] **Step 5: Commit and push**

```bash
git add src/ui/Dx11ViewportWidget.h src/ui/Dx11ViewportWidget.cpp src/ui/TransformPanel.h src/ui/TransformPanel.cpp src/app/MainWindow.h src/app/MainWindow.cpp
git commit -m "Orbit the teaching camera from the DX11 viewport."
git push origin main
```

---

