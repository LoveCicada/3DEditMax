### Task 3: Viewport look (DX11-Study scene cues)

**Files:**
- Modify: `assets/shaders/unlit.hlsl`
- Modify: `src/render/D3D11Renderer.cpp` / `.h`
- Modify: `src/render/DebugDraw.cpp` / `.h`
- Modify: `src/teach/MeshBuild.cpp` / `.h` (sphere tessellation; optional edge list helper)
- Modify: `src/ui/Dx11ViewportWidget` or `MainWindow` (axis legend overlay)
- Modify: `tests/test_transforms.cpp` or mesh tests if sphere vertex counts change

- [ ] **Step 1:** Clear color `#0a0a18`.
- [ ] **Step 2:** Ground grid 20脳20 at Y=0; major `#444466`, minor `#333355` (match `GridHelper` in scene.js).
- [ ] **Step 3:** Solid: Lambert + weak emissive; primary `#4a90d9` alpha ~0.9; LayoutThree companions `#38b889` / `#d98c3f`. Enable SrcAlpha / InvSrcAlpha blend.
- [ ] **Step 4:** After mesh, draw **silhouette edges** (cube: 12 edges only), color `#88ccff`.
- [ ] **Step 5:** World axes: thicker RGB lines + small cones, length ~3. Qt top-left axis legend (like `#axis-legend`); no 3D text sprites this pass.
- [ ] **Step 6:** Keep Normal / Checker / Wire. Sphere ~24脳32; update CTest expectations.
- [ ] **Step 7:** Manual: translucent blue box + bright edges + grid + cone axes; three-object green/orange readable.
- [ ] **Step 8:** Commit: `Align viewport materials and helpers with DX11-Study look.`

---

