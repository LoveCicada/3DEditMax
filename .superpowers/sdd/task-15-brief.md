### Task 15: Phase 1鈥? acceptance

**Files:** none required

- [ ] **Step 1: Run `ctest --test-dir build --output-on-failure` 鈥?all PASS**
- [ ] **Step 2: Manual full pass**

1. Column-major board matches a PIX/Graphics Debugger CB dump of `worldViewProj` (or at least self-consistent with `TrackPoint` NDC vs on-screen).
2. JSON save/load restores TRS and camera.
3. Tutorial 8 steps all apply.
4. Demo starts/stops.
5. Three objects + four shadings.
6. Frustum tracks near/far.
7. Lab fill/cull/depth.
8. Close joins the render thread.

- [ ] **Step 3: If README build instructions drifted, update and push**

```bash
git add README.md
git commit -m "Align README with the finished lab-shell build."
git push origin main
```

Only commit this step if `README.md` actually changed.

---

## Self-review

**Spec coverage:**

| Spec section | Tasks |
|--------------|-------|
| Phase 0 window/thread/cube/log | 1鈥? |
| HWND rules / no QPainter | 6 |
| std::thread, poll-only feedback | 3鈥? |
| DirectXMath, column-major display | 5, 8, 9 |
| TeachingState / LabState / snapshot | 2鈥? |
| Transforms, TrackPoint, CTest | 8, 11 mesh counts, 13 |
| Phase 1 sliders + board + orbit | 9鈥?0 |
| Phase 2 meshes/shading/frustum/tracker | 11鈥?2 |
| Phase 3 script/demo/JSON | 13 |
| Phase 4 shader/RS/CB | 14 |
| Error: debug runtime, device lost, shader fail | 5, 14 |
| No Scene/Node, no QThread, no invokeMethod | global + tasks 4鈥? |
| Git push per task | every task Step commit |

**Placeholders:** none. Web RH toggle remains out of scope (spec 搂4.3). Middle-mouse pan is explicitly skipped in Task 10.

**Types:** `CommandType`/`CmdInit`鈥? `FeedbackKind`/`FbLog`鈥? `TeachingState`, `LabState`, `StateSnapshot`, `RenderThread::start` / `requestStopAndJoin`, `TrackPoint` / `TrackResult`, `MajorOrder`, `tutorialStepAt`, `teachingToJson` are named consistently across tasks.
