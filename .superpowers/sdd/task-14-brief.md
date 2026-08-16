### Task 14: Lab panel (Phase 4)

**Files:**
- Create: `src/ui/LabPanel.h`
- Create: `src/ui/LabPanel.cpp`
- Modify: `src/render/D3D11Renderer.cpp` 鈥?apply `LabState` rasterizer/depth; list CB floats into feedback once per change
- Modify: `src/render/ShaderSet.cpp` 鈥?variants `unlit`, `normal`, `checker` (checker may already be CB mode; expose combo anyway)
- Modify: `DebugLogPanel` / `MainWindow` 鈥?Lab dock shows device description string
- Modify: `RenderThread` 鈥?after init, push `FbLog` with adapter name from `DXGI_ADAPTER_DESC::Description` (wide to UTF-8)

**Interfaces:**
- `LabPanel` combo shaderVariant (`unlit`/`normal`/`checker`), fill Solid/Wire, cull None/Back/Front, depth on/off
- Changing panel writes `LabState` and `publishState`
- Renderer: create three rasterizer states and two depth states at init; bind from snapshot each frame
- `CmdReloadShader` from a LabPanel button; UI pushes the command through `Dx11ViewportWidget::reloadShaders()` which pushes `CmdReloadShader`
- CB observer: `LabPanel` has a read-only text box; MainWindow fills it with the same four matrices already computed for the board (do not read back GPU)

- [ ] **Step 1: Implement LabPanel + `reloadShaders` on the widget**
- [ ] **Step 2: Renderer bind RS/DSS from `snap.lab`**
- [ ] **Step 3: Manual** 鈥?wireframe/cull/depth toggles visible; reload shader with a syntax error shows Error in log and keeps old PSO; fix file and reload recovers
- [ ] **Step 4: Commit and push**

```bash
git add src/ui/LabPanel.h src/ui/LabPanel.cpp src/ui/Dx11ViewportWidget.h src/ui/Dx11ViewportWidget.cpp src/render/D3D11Renderer.cpp src/render/ShaderSet.cpp src/render/RenderThread.cpp src/app/MainWindow.h src/app/MainWindow.cpp CMakeLists.txt
git commit -m "Expose rasterizer, depth, shaders, and CB text in the lab dock."
git push origin main
```

---

