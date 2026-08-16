### Task 12: Frustum, axes, tracker (Phase 2)

**Files:**
- Create: `src/render/DebugDraw.h`
- Create: `src/render/DebugDraw.cpp`
- Create: `src/ui/TrackerPanel.h`
- Create: `src/ui/TrackerPanel.cpp`
- Modify: `D3D11Renderer::render` to draw debug after meshes
- Modify: `MainWindow` right dock tracker
- Modify: `assets/shaders` add `line.hlsl` or reuse unlit with a line input layout (pos only)

**Interfaces:**
- `void DebugDraw::create(ID3D11Device*)`
- `void DebugDraw::draw(ID3D11DeviceContext*, const StateSnapshot&, FXMMATRIX view, FXMMATRIX proj)`
- Frustum: 8 corners from inverse projection (LH clip), transform by `inv(view)`, line list 12 edges, color yellow
- Axes at origin: length 1, RGB = XYZ
- Tracker: `TrackPoint` then draw a small octahedron or 3-axis gizmo at **world** position of the point
- `TrackerPanel`: three spin boxes + preset buttons for cube corners `(卤1,卤1,卤1)` + read-only lines for world/view/clip/ndc
- MainWindow on any state change refreshes tracker labels via `TrackPoint`

- [ ] **Step 1: `line.hlsl`** 鈥?VS `mul(float4(pos,1), viewProj)` where `viewProj = view*proj` (world lines already in world). Color in vertex.
- [ ] **Step 2: Implement DebugDraw dynamic VB (Map DISCARD) each frame**
- [ ] **Step 3: TrackerPanel + wiring**
- [ ] **Step 4: Manual** 鈥?near/far sliders resize frustum; tracker point sits on cube corner
- [ ] **Step 5: Commit and push**

```bash
git add src/render/DebugDraw.h src/render/DebugDraw.cpp assets/shaders/line.hlsl src/ui/TrackerPanel.h src/ui/TrackerPanel.cpp src/render/D3D11Renderer.cpp src/app/MainWindow.h src/app/MainWindow.cpp CMakeLists.txt
git commit -m "Draw frustum, axes, and tracked model-space points."
git push origin main
```

---

