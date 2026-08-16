### Task 11: Meshes, layout, shading (Phase 2)

**Files:**
- Modify: `src/render/MeshGpu.h/.cpp` 鈥?`createSphere(dev, 16, 24)`, `createCylinder(dev, 24)`
- Modify: `src/render/D3D11Renderer.cpp` 鈥?draw 1 or 3 objects; wire via rasterizer fill; normal/checker in PS
- Create: `assets/shaders/unlit.hlsl` already has normals; add `ps_checker` and compile second PS or branch on `cb.mode`
- Extend `FrameCB` with `float4 shadingMode` (x=0 solid, 1 normal, 2 checker, 3 unused because wire is RS)
- Create: `src/ui/ObjectPanel.h/.cpp`
- Modify: `MainWindow` left dock stack ObjectPanel under transforms

**Interfaces:**
- `MeshGpu createSphere(ID3D11Device* dev, int slices, int stacks)` 鈥?radius 0.5, UV + smooth normals
- `MeshGpu createCylinder(ID3D11Device* dev, int slices)` 鈥?radius 0.5, height 1, Y-up, caps
- Renderer holds `m_cube`, `m_sphere`, `m_cyl`
- For `LayoutThree`, draw `objects[0..2]`; for `LayoutOne`, draw `[0]`
- `ShadeWire`: `D3D11_FILL_WIREFRAME` regardless of LabState fill until Phase 4, when shading wire OR lab fill can set wire (if either is wire, draw wire)
- Checker PS: `frac(uv*8)` black/white
- ObjectPanel: 3 mesh radio, layout combo, shading combo, `changed()`

- [ ] **Step 1: Implement CPU mesh builders; no GPU test 鈥?verify vertex count > 0 in a small CPU-only helper `int cubeVertexCount()` used by tests if you extract generation to `src/teach/MeshBuild.cpp`. Extract:**

`src/teach/MeshBuild.h`:

```cpp
#pragma once
#include <vector>
struct MeshVertex { float px,py,pz,nx,ny,nz,u,v; };
void buildCube(std::vector<MeshVertex>* v, std::vector<unsigned short>* i);
void buildSphere(std::vector<MeshVertex>* v, std::vector<unsigned short>* i, int slices, int stacks);
void buildCylinder(std::vector<MeshVertex>* v, std::vector<unsigned short>* i, int slices);
```

Test: cube indices 36; sphere verts `(stacks+1)*(slices+1)` > 0.

- [ ] **Step 2: GPU upload in `MeshGpu::createFromCpu`**
- [ ] **Step 3: Shader branch + ObjectPanel + MainWindow**
- [ ] **Step 4: Manual** 鈥?three cubes side by side; sphere/cylinder; checker; wire
- [ ] **Step 5: Commit and push**

```bash
git add src/teach/MeshBuild.h src/teach/MeshBuild.cpp src/render/MeshGpu.h src/render/MeshGpu.cpp src/render/D3D11Renderer.cpp src/render/ShaderSet.cpp assets/shaders/unlit.hlsl src/ui/ObjectPanel.h src/ui/ObjectPanel.cpp src/app/MainWindow.h src/app/MainWindow.cpp tests/test_transforms.cpp CMakeLists.txt
git commit -m "Add teaching meshes, three-object layout, and shading modes."
git push origin main
```

---

