### Task 2: MSAA + depth bias for debug lines

**Files:**
- Modify: `src/render/D3D11Renderer.cpp` / `.h` (sample count, depth buffer match)
- Modify: `src/render/DebugDraw.cpp` / `.h` (rasterizer / depth-stencil for lines)

- [ ] **Step 1:** After device create, `CheckMultisampleQualityLevels` for `R8G8B8A8_UNORM`; pick 4, else 2, else 1. Set swapchain + depth `SampleDesc` alike. Keep `DXGI_SWAP_EFFECT_DISCARD`.
- [ ] **Step 2:** DebugDraw: bind a rasterizer with `DepthBias` / negative `SlopeScaledDepthBias`, depth test on, `DepthWriteMask = ZERO`.
- [ ] **Step 3:** Teaching wireframe stays Back cull; outline path (Task 3) uses true edges, not triangle diagonals.
- [ ] **Step 4:** Manual: oblique view 鈥?smoother silhouettes; axes/grid less z-fight.
- [ ] **Step 5:** Commit: `Add MSAA swapchain and depth-biased debug lines.`

---

