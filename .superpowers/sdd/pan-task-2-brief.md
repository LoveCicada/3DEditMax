### Task 2: Right-button pan

Plan: docs/superpowers/plans/2026-08-16-viewport-pan-layout.md Task 2

- camTarget[3] default 0,0,0
- BuildView(distance,pitch,yaw,target)
- applyPanDrag along camera right/up
- RMB pan + grabMouse; release syncs UI (same as Task 1 — no teachingEdited during drag)
- LMB orbits about current target; wheel dolly
- JSON camTarget optional (missing = origin)
- Transform View: Target X/Y/Z preferred
- Update all BuildView call sites (MainWindow, D3D11Renderer, tests)
- Commit: Add camera target and right-button pan for the viewport.
- Local only; build MSVC+Ninja Debug; ctest
