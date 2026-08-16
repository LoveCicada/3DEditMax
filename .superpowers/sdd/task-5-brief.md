### Task 5: D3D11Renderer clear + cube (Phase 0 GPU)

**Files:**
- Create: `src/render/D3D11Renderer.h`
- Create: `src/render/D3D11Renderer.cpp`
- Create: `src/render/MeshGpu.h`
- Create: `src/render/MeshGpu.cpp`
- Create: `src/render/ShaderSet.h`
- Create: `src/render/ShaderSet.cpp`
- Create: `assets/shaders/unlit.hlsl`
- Create: `src/teach/Transforms.h`
- Create: `src/teach/Transforms.cpp`
- Modify: `src/render/RenderThread.cpp` to own `D3D11Renderer` and handle Init/Resize/ReloadShader
- Modify: `CMakeLists.txt` 鈥?app links `d3d11 dxgi d3dcompiler`; copy `assets/shaders` next to exe; add teach/render sources to both targets as needed

**Interfaces:**
- Consumes: `HWND`, `StateSnapshot`, `BuildWorld` / `BuildView` / `BuildProjection` (implement minimal versions here; Task 8 expands tests)
- Produces:
  - `bool D3D11Renderer::initialize(HWND hwnd, int w, int h, bool wantDebug, FeedbackQueue* fb)`
  - `void D3D11Renderer::shutdown()`
  - `bool D3D11Renderer::resize(int w, int h)`
  - `void D3D11Renderer::render(const StateSnapshot& snap)`
  - `bool D3D11Renderer::reloadShaders(FeedbackQueue* fb)`
  - `MeshGpu::createCube(ID3D11Device*)`
  - `ShaderSet::compileFromFile(device, pathVsEntry, pathPsEntry, FeedbackQueue*)`

- [ ] **Step 1: Write `unlit.hlsl`**

```hlsl
cbuffer FrameCB : register(b0) {
  float4x4 world;
  float4x4 view;
  float4x4 proj;
  float4x4 worldViewProj;
};

struct VSIn {
  float3 pos : POSITION;
  float3 nrm : NORMAL;
  float2 uv  : TEXCOORD0;
};

struct VSOut {
  float4 pos : SV_POSITION;
  float3 nrm : COLOR0;
  float2 uv  : TEXCOORD0;
};

VSOut vs_main(VSIn i) {
  VSOut o;
  o.pos = mul(float4(i.pos, 1.0), worldViewProj);
  o.nrm = i.nrm;
  o.uv = i.uv;
  return o;
}

float4 ps_main(VSOut i) : SV_TARGET {
  float3 c = 0.35 + 0.65 * saturate(i.nrm * 0.5 + 0.5);
  return float4(c, 1.0);
}
```

HLSL `mul(float4, float4x4)` matches column-major DX11 if the CPU stored XMMATRIX with `XMStoreFloat4x4`.

- [ ] **Step 2: Transforms used by the renderer**

`src/teach/Transforms.h`:

```cpp
#pragma once
#include "core/TeachingState.h"
#include <DirectXMath.h>

DirectX::XMMATRIX BuildWorld(const TransformTRS& trs);
DirectX::XMMATRIX BuildView(float distance, float pitchDeg, float yawDeg);
DirectX::XMMATRIX BuildProjection(const TeachingState& s, float aspect);
```

`src/teach/Transforms.cpp`:

```cpp
#include "teach/Transforms.h"

using namespace DirectX;

XMMATRIX BuildWorld(const TransformTRS& trs) {
  const XMMATRIX s = XMMatrixScaling(trs.scale[0], trs.scale[1], trs.scale[2]);
  const XMMATRIX r = XMMatrixRotationRollPitchYaw(
      XMConvertToRadians(trs.pitchDeg),
      XMConvertToRadians(trs.yawDeg),
      XMConvertToRadians(trs.rollDeg));
  const XMMATRIX t = XMMatrixTranslation(trs.pos[0], trs.pos[1], trs.pos[2]);
  return s * r * t;
}

XMMATRIX BuildView(float distance, float pitchDeg, float yawDeg) {
  const float pitch = XMConvertToRadians(pitchDeg);
  const float yaw = XMConvertToRadians(yawDeg);
  const XMVECTOR eye = XMVectorSet(
      distance * sinf(yaw) * cosf(pitch),
      distance * sinf(pitch),
      distance * cosf(yaw) * cosf(pitch),
      1.f);
  const XMVECTOR at = XMVectorZero();
  const XMVECTOR up = XMVectorSet(0.f, 1.f, 0.f, 0.f);
  return XMMatrixLookAtLH(eye, at, up);
}

XMMATRIX BuildProjection(const TeachingState& s, float aspect) {
  const float a = (aspect > 0.01f) ? aspect : (16.f / 9.f);
  if (s.proj == ProjOrtho) {
    const float h = s.camDistance;
    return XMMatrixOrthographicLH(h * a, h, s.nearZ, s.farZ);
  }
  return XMMatrixPerspectiveFovLH(XMConvertToRadians(s.fovDeg), a, s.nearZ, s.farZ);
}
```

- [ ] **Step 3: MeshGpu cube**

Vertex struct `{ float px,py,pz, nx,ny,nz, u,v; }` 32 bytes. 24 unique cube verts (per-face normals) + 36 indices. `createCube` builds immutable `D3D11_USAGE_DEFAULT` VB/IB. `draw(context)` binds topology triangle list.

Sphere/cylinder can return empty in this task; implement in Task 10.

- [ ] **Step 4: ShaderSet**

`compileFromFile(ID3D11Device* dev, const std::wstring& path, FeedbackQueue* fb)`:

- `D3DCompileFromFile(path, 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, "vs_main", "vs_5_0", 0, 0, &vsBlob, &err)`
- same for `ps_main` / `ps_5_0`
- create VS/PS
- input layout: POSITION R32G32B32, NORMAL R32G32B32, TEXCOORD R32G32
- on failure: if `err` blob, push `FbError` with blob text; return false; caller keeps previous shaders

Resolve shader path: `QCoreApplication::applicationDirPath()` is Qt 鈥?renderer must not use Qt. Pass absolute path from UI via `ReloadShader` later; for Init, `RenderThread` receives a `std::wstring shaderDir` in its constructor, set by the widget from `QCoreApplication::applicationDirPath() + "/shaders"`.

Add to `RenderThread`:

```cpp
explicit RenderThread(const std::wstring& shaderDir);
```

CMake post-build:

```cmake
add_custom_command(TARGET 3DEditMax POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets/shaders"
    "$<TARGET_FILE_DIR:3DEditMax>/shaders")
```

- [ ] **Step 5: D3D11Renderer**

`initialize`:

1. `CreateDXGIFactory1`
2. Enum adapters, skip `DXGI_ADAPTER_FLAG_SOFTWARE`, take first hardware
3. `UINT flags = 0`; if `wantDebug` then `flags |= D3D11_CREATE_DEVICE_DEBUG`
4. `D3D11CreateDevice` feature levels 11_1, 11_0, 10_1, 10_0
5. If debug create fails, clear debug flag, retry, `fb->push({FbWarn, "D3D11 debug runtime missing", 0})`
6. Swapchain desc: `BufferCount=2`, `Format=DXGI_FORMAT_R8G8B8A8_UNORM`, `OutputWindow=hwnd`, `Windowed=TRUE`, `SwapEffect=DXGI_SWAP_EFFECT_DISCARD`, `BufferUsage=DXGI_USAGE_RENDER_TARGET_OUTPUT`, size max(w,1) x max(h,1)
7. `CreateSwapChain`, `MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER)`
8. Create RTV from buffer 0; create `DXGI_FORMAT_D24_UNORM_S8_UINT` depth; DSV
9. Default rasterizer solid/back; depth state enabled
10. Create `FrameCB` dynamic constant buffer `sizeof(float)*16*4`
11. `createCube`; compile `shaderDir + L"/unlit.hlsl"`
12. `fb->push({FbLog, "D3D11 device ready", 0})` and `{FbDeviceOk, "", 0}`

`resize`: if w<=0 or h<=0 return true without touching buffers. Else release RTV/DSV, `ResizeBuffers`, recreate views.

`render`:

- if w<=0 or h<=0 return
- OMSet RTV+DSV, viewport, clear color `(0.08, 0.10, 0.14, 1)`, clear depth 1
- aspect = follow ? w/h : teaching.aspect
- W = `BuildWorld(objects[0].trs)` (phase 0 draws object 0 only)
- V = `BuildView(...)`, P = `BuildProjection(...)`
- WVP = W*V*P
- `XMStoreFloat4x4` into a `struct FrameCBCpu { XMFLOAT4X4 w,v,p,wvp; }`
- Map CB, memcpy, unmap, VSSet/PSSet b0
- set shaders, layout, rasterizer, depth
- `cube.draw`
- `Present(1, 0)`
- if `Present` returns `DXGI_ERROR_DEVICE_REMOVED` or `DXGI_ERROR_DEVICE_RESET`: push `FbDeviceLost`, try `shutdown` + `initialize` with stored hwnd/size; if fail, set `m_dead=true` and skip further presents

`shutdown`: release all COM in reverse order.

Use `Microsoft::WRL::ComPtr`.

- [ ] **Step 6: Wire commands in `RenderThread::threadMain`**

Members: `D3D11Renderer m_renderer;` (or `unique_ptr` via `make_unique` after first Init), `HWND m_hwnd`, `int m_w, m_h`, `std::wstring m_shaderDir`.

On `CmdInit`: `m_renderer.initialize(cmd.hwnd, cmd.width, cmd.height, true, &m_feedback)`  
On `CmdResize`: `m_renderer.resize`  
On `CmdReloadShader`: `m_renderer.reloadShaders(&m_feedback)`  
On `CmdStop`: `m_renderer.shutdown(); break`  
Each loop after commands: if initialized and not dead, `m_renderer.render(m_snapshots.consume())`, then push `FbFps` every 30 frames with ms.

Sleep 1ms only when not initialized; when presenting, rely on `Present(1,0)` vsync.

- [ ] **Step 7: Link libraries on `3DEditMax`**

```cmake
target_link_libraries(3DEditMax PRIVATE Qt5::Widgets d3d11 dxgi d3dcompiler)
```

Add all new sources to `3DEditMax`. Tests keep Transforms + queues; they do not need D3D.

- [ ] **Step 8: Manual GPU check is Task 6** (needs HWND). Commit renderer even if unused.

```bash
git add assets/shaders/unlit.hlsl src/teach/Transforms.h src/teach/Transforms.cpp src/render/D3D11Renderer.h src/render/D3D11Renderer.cpp src/render/MeshGpu.h src/render/MeshGpu.cpp src/render/ShaderSet.h src/render/ShaderSet.cpp src/render/RenderThread.h src/render/RenderThread.cpp CMakeLists.txt
git commit -m "Add D3D11 renderer, cube mesh, and unlit shader."
git push origin main
```

---

