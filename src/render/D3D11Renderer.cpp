#ifdef _MSC_VER
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif
#include "render/D3D11Renderer.h"
#include "core/LabState.h"
#include "teach/MeshBuild.h"
#include "teach/Transforms.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <windows.h>

using Microsoft::WRL::ComPtr;
using namespace DirectX;

namespace {

void pushFb(FeedbackQueue* fb, FeedbackKind kind, const char* text) {
  if (!fb) {
    return;
  }
  FeedbackItem item;
  item.kind = kind;
  item.text = text;
  item.ms = 0.f;
  fb->push(item);
}

struct FrameCBCpu {
  XMFLOAT4X4 w;
  XMFLOAT4X4 v;
  XMFLOAT4X4 p;
  XMFLOAT4X4 wvp;
  XMFLOAT4 shadingMode;
  XMFLOAT4 baseColor;
  XMFLOAT4 emissive;
};

const int kSphereSlices = 32;
const int kSphereStacks = 24;

void objectShadeColors(int objectIndex, XMFLOAT4* base, XMFLOAT4* emit) {
  if (objectIndex == 1) {
    *base = XMFLOAT4(0x38 / 255.f, 0xb8 / 255.f, 0x89 / 255.f, 0.82f);
    *emit = XMFLOAT4(0x17 / 255.f, 0x3c / 255.f, 0x31 / 255.f, 0.f);
    return;
  }
  if (objectIndex == 2) {
    *base = XMFLOAT4(0xd9 / 255.f, 0x8c / 255.f, 0x3f / 255.f, 0.82f);
    *emit = XMFLOAT4(0x4f / 255.f, 0x2e / 255.f, 0x12 / 255.f, 0.f);
    return;
  }
  *base = XMFLOAT4(0x4a / 255.f, 0x90 / 255.f, 0xd9 / 255.f, 0.9f);
  *emit = XMFLOAT4(0x1a / 255.f, 0x3a / 255.f, 0x5c / 255.f, 0.f);
}

XMFLOAT4 objectEdgeColor(int objectIndex) {
  if (objectIndex == 1) {
    return XMFLOAT4(0x72 / 255.f, 0xf2 / 255.f, 0xc6 / 255.f, 1.f);
  }
  if (objectIndex == 2) {
    return XMFLOAT4(1.f, 0xc1 / 255.f, 0x78 / 255.f, 1.f);
  }
  return XMFLOAT4(0x88 / 255.f, 0xcc / 255.f, 1.f, 1.f);
}

void writeFrameCb(ID3D11DeviceContext* context, ID3D11Buffer* buf,
                  FXMMATRIX W, FXMMATRIX V, FXMMATRIX P,
                  float shadeX, const XMFLOAT4& base, const XMFLOAT4& emit) {
  FrameCBCpu cb;
  const XMMATRIX WVP = W * V * P;
  XMStoreFloat4x4(&cb.w, XMMatrixTranspose(W));
  XMStoreFloat4x4(&cb.v, XMMatrixTranspose(V));
  XMStoreFloat4x4(&cb.p, XMMatrixTranspose(P));
  XMStoreFloat4x4(&cb.wvp, XMMatrixTranspose(WVP));
  cb.shadingMode = XMFLOAT4(shadeX, 0.f, 0.f, 0.f);
  cb.baseColor = base;
  cb.emissive = emit;
  D3D11_MAPPED_SUBRESOURCE mapped = {};
  if (SUCCEEDED(context->Map(buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
    std::memcpy(mapped.pData, &cb, sizeof(cb));
    context->Unmap(buf, 0);
  }
}

XMMATRIX rotationYToDir(FXMVECTOR dir) {
  const XMVECTOR d = XMVector3Normalize(dir);
  const XMVECTOR y = XMVectorSet(0.f, 1.f, 0.f, 0.f);
  const float dty = XMVectorGetX(XMVector3Dot(y, d));
  if (dty > 0.9999f) {
    return XMMatrixIdentity();
  }
  if (dty < -0.9999f) {
    return XMMatrixRotationX(XM_PI);
  }
  const XMVECTOR axis = XMVector3Normalize(XMVector3Cross(y, d));
  float c = dty;
  if (c > 1.f) {
    c = 1.f;
  }
  if (c < -1.f) {
    c = -1.f;
  }
  return XMMatrixRotationAxis(axis, acosf(c));
}

void pushCbFloats(FeedbackQueue* fb, const FrameCBCpu& cb) {
  if (!fb) {
    return;
  }
  char chunk[64];
  std::string text = "FrameCB";
  const XMFLOAT4X4* mats[4] = { &cb.w, &cb.v, &cb.p, &cb.wvp };
  const char* names[4] = { " W", " V", " P", " WVP" };
  for (int mi = 0; mi < 4; ++mi) {
    text += names[mi];
    const float* f = &mats[mi]->m[0][0];
    for (int i = 0; i < 16; ++i) {
      std::sprintf(chunk, " %.3f", static_cast<double>(f[i]));
      text += chunk;
    }
  }
  std::sprintf(chunk, " shade=%.0f", static_cast<double>(cb.shadingMode.x));
  text += chunk;
  pushFb(fb, FbLog, text.c_str());
}

}  // namespace

D3D11Renderer::D3D11Renderer(const std::wstring& shaderDir)
    : m_shaderDir(shaderDir)
    , m_hwnd(0)
    , m_w(0)
    , m_h(0)
    , m_wantDebug(false)
    , m_initialized(false)
    , m_dead(false)
    , m_fb(0)
    , m_lastLoggedShadeX(-1.f)
    , m_loggedCb(false)
    , m_sampleCount(1)
    , m_sampleQuality(0)
    , m_cubeEdgeVerts(0)
    , m_sphereEdgeVerts(0)
    , m_cylEdgeVerts(0) {
  std::memset(m_shaderVariant, 0, sizeof(m_shaderVariant));
  std::memset(m_triedVariant, 0, sizeof(m_triedVariant));
}

bool D3D11Renderer::initialize(HWND hwnd, int w, int h, bool wantDebug, FeedbackQueue* fb) {
  shutdown();
  m_dead = false;
  m_fb = fb;
  m_hwnd = hwnd;
  m_w = (w > 1) ? w : 1;
  m_h = (h > 1) ? h : 1;
  m_wantDebug = wantDebug;

  HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(m_factory.ReleaseAndGetAddressOf()));
  if (FAILED(hr) || !m_factory) {
    pushFb(fb, FbError, "CreateDXGIFactory1 failed");
    shutdown();
    return false;
  }

  ComPtr<IDXGIAdapter1> adapter;
  for (UINT i = 0; m_factory->EnumAdapters1(i, adapter.ReleaseAndGetAddressOf()) != DXGI_ERROR_NOT_FOUND; ++i) {
    DXGI_ADAPTER_DESC1 desc;
    if (FAILED(adapter->GetDesc1(&desc))) {
      continue;
    }
    if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
      continue;
    }
    m_adapter = adapter;
    break;
  }
  if (!m_adapter) {
    pushFb(fb, FbError, "No hardware DXGI adapter");
    shutdown();
    return false;
  }

  UINT flags = 0;
  if (wantDebug) {
    flags |= D3D11_CREATE_DEVICE_DEBUG;
  }
  const D3D_FEATURE_LEVEL levels[] = {
      D3D_FEATURE_LEVEL_11_1,
      D3D_FEATURE_LEVEL_11_0,
      D3D_FEATURE_LEVEL_10_1,
      D3D_FEATURE_LEVEL_10_0,
  };
  D3D_FEATURE_LEVEL got = D3D_FEATURE_LEVEL_11_0;
  hr = D3D11CreateDevice(
      m_adapter.Get(),
      D3D_DRIVER_TYPE_UNKNOWN,
      0,
      flags,
      levels,
      4,
      D3D11_SDK_VERSION,
      &m_device,
      &got,
      &m_context);
  if (FAILED(hr) && (flags & D3D11_CREATE_DEVICE_DEBUG)) {
    flags &= ~D3D11_CREATE_DEVICE_DEBUG;
    hr = D3D11CreateDevice(
        m_adapter.Get(),
        D3D_DRIVER_TYPE_UNKNOWN,
        0,
        flags,
        levels,
        4,
        D3D11_SDK_VERSION,
        &m_device,
        &got,
        &m_context);
    if (SUCCEEDED(hr)) {
      pushFb(fb, FbWarn, "D3D11 debug runtime missing");
    }
  }
  if (FAILED(hr) || !m_device || !m_context) {
    pushFb(fb, FbError, "D3D11CreateDevice failed");
    shutdown();
    return false;
  }
  attachInfoQueue();
  selectMultisampleLevel();

  DXGI_SWAP_CHAIN_DESC sd = {};
  // Multisampled DISCARD swapchains require BufferCount == 1.
  sd.BufferCount = (m_sampleCount > 1) ? 1 : 2;
  sd.BufferDesc.Width = static_cast<UINT>(m_w);
  sd.BufferDesc.Height = static_cast<UINT>(m_h);
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferDesc.RefreshRate.Numerator = 60;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow = hwnd;
  sd.SampleDesc.Count = m_sampleCount;
  sd.SampleDesc.Quality = m_sampleQuality;
  sd.Windowed = TRUE;
  sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

  hr = m_factory->CreateSwapChain(m_device.Get(), &sd, m_swap.ReleaseAndGetAddressOf());
  if ((FAILED(hr) || !m_swap) && m_sampleCount > 1) {
    m_sampleCount = 1;
    m_sampleQuality = 0;
    sd.BufferCount = 2;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    hr = m_factory->CreateSwapChain(m_device.Get(), &sd, m_swap.ReleaseAndGetAddressOf());
  }
  if (FAILED(hr) || !m_swap) {
    pushFb(fb, FbError, "CreateSwapChain failed");
    shutdown();
    return false;
  }
  m_factory->MakeWindowAssociation(
      hwnd, DXGI_MWA_NO_ALT_ENTER | DXGI_MWA_NO_WINDOW_CHANGES);

  if (!createSizeDependentResources()) {
    pushFb(fb, FbError, "Create RTV/DSV failed");
    shutdown();
    return false;
  }

  if (!createLabStates(fb)) {
    shutdown();
    return false;
  }

  D3D11_BUFFER_DESC cbd = {};
  cbd.ByteWidth = sizeof(FrameCBCpu);
  cbd.Usage = D3D11_USAGE_DYNAMIC;
  cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  hr = m_device->CreateBuffer(&cbd, 0, &m_cb);
  if (FAILED(hr)) {
    pushFb(fb, FbError, "Create FrameCB failed");
    shutdown();
    return false;
  }

  m_cube = MeshGpu::createCube(m_device.Get());
  if (!m_cube.valid()) {
    pushFb(fb, FbError, "createCube failed");
    shutdown();
    return false;
  }
  m_sphere = MeshGpu::createSphere(m_device.Get(), kSphereSlices, kSphereStacks);
  if (!m_sphere.valid()) {
    pushFb(fb, FbError, "createSphere failed");
    shutdown();
    return false;
  }
  m_cyl = MeshGpu::createCylinder(m_device.Get(), 24);
  if (!m_cyl.valid()) {
    pushFb(fb, FbError, "createCylinder failed");
    shutdown();
    return false;
  }
  m_cone = MeshGpu::createCone(m_device.Get(), 0.15f, 0.36f, 8);
  if (!m_cone.valid()) {
    pushFb(fb, FbWarn, "createCone failed");
  }

  copyVariantName("unlit");
  copyTriedName("unlit");
  if (!m_shaders.compileVariant(m_device.Get(), m_shaderDir, m_shaderVariant, fb)) {
    shutdown();
    return false;
  }
  m_debug.create(m_device.Get(), m_shaderDir);
  if (!m_debug.valid()) {
    pushFb(fb, FbWarn, "DebugDraw line.hlsl failed");
  }
  if (!createEdgeBuffers(fb)) {
    pushFb(fb, FbWarn, "silhouette edge buffers failed");
  }

  m_initialized = true;
  pushFb(fb, FbLog, "D3D11 device ready");
  pushFb(fb, FbDeviceOk, "");
  pumpInfoQueue();
  return true;
}

void D3D11Renderer::shutdown() {
  if (m_context) {
    m_context->ClearState();
    m_context->Flush();
  }
  m_shaders = ShaderSet();
  m_debug.reset();
  m_cube = MeshGpu();
  m_sphere = MeshGpu();
  m_cyl = MeshGpu();
  m_cone = MeshGpu();
  m_cubeEdges.Reset();
  m_sphereEdges.Reset();
  m_cylEdges.Reset();
  m_cubeEdgeVerts = 0;
  m_sphereEdgeVerts = 0;
  m_cylEdgeVerts = 0;
  m_cb.Reset();
  m_blend.Reset();
  m_depthOff.Reset();
  m_depthOn.Reset();
  for (int fi = 0; fi < 2; ++fi) {
    for (int ci = 0; ci < 3; ++ci) {
      m_raster[fi][ci].Reset();
    }
  }
  std::memset(m_shaderVariant, 0, sizeof(m_shaderVariant));
  std::memset(m_triedVariant, 0, sizeof(m_triedVariant));
  m_infoQueue.Reset();
  m_lastLoggedShadeX = -1.f;
  m_loggedCb = false;
  m_dsv.Reset();
  m_depth.Reset();
  m_rtv.Reset();
  m_swap.Reset();
  m_context.Reset();
  m_device.Reset();
  m_adapter.Reset();
  m_factory.Reset();
  m_initialized = false;
}

bool D3D11Renderer::resize(int w, int h) {
  if (w <= 0 || h <= 0) {
    return true;
  }
  if (!m_swap || !m_context || !m_device) {
    return false;
  }
  if (w == m_w && h == m_h && m_rtv && m_dsv) {
    return true;
  }
  m_w = w;
  m_h = h;

  // Must release every outstanding back-buffer reference before ResizeBuffers.
  m_context->OMSetRenderTargets(0, 0, 0);
  m_context->ClearState();
  m_context->Flush();
  m_rtv.Reset();
  m_dsv.Reset();
  m_depth.Reset();

  const HRESULT hr = m_swap->ResizeBuffers(
      0,
      static_cast<UINT>(w),
      static_cast<UINT>(h),
      DXGI_FORMAT_UNKNOWN,
      0);
  if (FAILED(hr)) {
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
      handlePresentResult(hr);
      return m_initialized && !m_dead && m_rtv && m_dsv;
    }
    pushFb(m_fb, FbError, "ResizeBuffers failed");
    return false;
  }
  if (!createSizeDependentResources()) {
    m_rtv.Reset();
    m_dsv.Reset();
    m_depth.Reset();
    pushFb(m_fb, FbError, "Create RTV/DSV failed");
    return false;
  }
  return true;
}

void D3D11Renderer::render(const StateSnapshot& snap) {
  if (m_dead || !m_initialized || !m_context || !m_swap) {
    return;
  }
  if (!m_rtv || !m_dsv) {
    return;
  }
  if (m_w <= 0 || m_h <= 0) {
    return;
  }

  ID3D11RenderTargetView* rtv = m_rtv.Get();
  m_context->OMSetRenderTargets(1, &rtv, m_dsv.Get());

  D3D11_VIEWPORT vp = {};
  vp.Width = static_cast<float>(m_w);
  vp.Height = static_cast<float>(m_h);
  vp.MinDepth = 0.f;
  vp.MaxDepth = 1.f;
  m_context->RSSetViewports(1, &vp);

  const float clear[4] = {0x0a / 255.f, 0x0a / 255.f, 0x18 / 255.f, 1.f};
  m_context->ClearRenderTargetView(m_rtv.Get(), clear);
  m_context->ClearDepthStencilView(m_dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

  const TeachingState& t = snap.teaching;
  if (std::strcmp(snap.lab.shaderVariant, m_triedVariant) != 0) {
    ShaderSet next;
    if (next.compileVariant(m_device.Get(), m_shaderDir, snap.lab.shaderVariant, m_fb)) {
      m_shaders = next;
      copyVariantName(snap.lab.shaderVariant);
    }
    copyTriedName(snap.lab.shaderVariant);
  }
  const float aspect = t.aspectFollowViewport
      ? (static_cast<float>(m_w) / static_cast<float>(m_h))
      : t.aspect;
  const XMMATRIX V = BuildView(t.camDistance, t.camPitchDeg, t.camYawDeg, t.camTarget);
  const XMMATRIX P = BuildProjection(t, aspect);
  const int count = (t.layout == LayoutThree) ? 3 : 1;
  const float shadeX = labShadeModeX(snap.lab.shaderVariant, static_cast<int>(t.shading));

  bindMeshPipeline(snap);

  for (int oi = 0; oi < count; ++oi) {
    const TeachingObject& obj = t.objects[oi];
    const XMMATRIX W = BuildWorld(obj.trs);
    XMFLOAT4 base;
    XMFLOAT4 emit;
    objectShadeColors(oi, &base, &emit);
    writeFrameCb(m_context.Get(), m_cb.Get(), W, V, P, shadeX, base, emit);
    if (!m_loggedCb || shadeX != m_lastLoggedShadeX) {
      m_loggedCb = true;
      m_lastLoggedShadeX = shadeX;
      FrameCBCpu logCb;
      const XMMATRIX WVP = W * V * P;
      XMStoreFloat4x4(&logCb.w, XMMatrixTranspose(W));
      XMStoreFloat4x4(&logCb.v, XMMatrixTranspose(V));
      XMStoreFloat4x4(&logCb.p, XMMatrixTranspose(P));
      XMStoreFloat4x4(&logCb.wvp, XMMatrixTranspose(WVP));
      logCb.shadingMode = XMFLOAT4(shadeX, 0.f, 0.f, 0.f);
      logCb.baseColor = base;
      logCb.emissive = emit;
      pushCbFloats(m_fb, logCb);
    }

    const MeshGpu* mesh = &m_cube;
    ID3D11Buffer* edgeVb = m_cubeEdges.Get();
    UINT edgeVerts = m_cubeEdgeVerts;
    if (obj.mesh == MeshSphere) {
      mesh = &m_sphere;
      edgeVb = m_sphereEdges.Get();
      edgeVerts = m_sphereEdgeVerts;
    } else if (obj.mesh == MeshCylinder) {
      mesh = &m_cyl;
      edgeVb = m_cylEdges.Get();
      edgeVerts = m_cylEdgeVerts;
    }
    bindMeshPipeline(snap);
    mesh->draw(m_context.Get());

    if (t.shading != ShadeWire && edgeVb && edgeVerts > 0 && m_debug.valid()) {
      const XMMATRIX WVP = W * V * P;
      m_debug.drawLineList(m_context.Get(), edgeVb, edgeVerts, WVP, objectEdgeColor(oi));
    }
  }

  m_debug.draw(m_context.Get(), snap, V, P);
  drawAxisCones(snap, V, P, DirectX::XMVectorZero(), 3.f);
  const float* op = t.objects[0].trs.pos;
  drawAxisCones(snap, V, P, DirectX::XMVectorSet(op[0], op[1], op[2], 0.f),
                worldAxisGizmoLength());

  // Sync interval 0: callers may hold a mutex shared with the UI resize path;
  // never block here on vsync or the UI thread can deadlock.
  const HRESULT hr = m_swap->Present(0, 0);
  handlePresentResult(hr);
  pumpInfoQueue();
}

bool D3D11Renderer::reloadShaders(FeedbackQueue* fb) {
  if (!m_device) {
    return false;
  }
  std::memset(m_triedVariant, 0, sizeof(m_triedVariant));
  const char* variant = m_shaderVariant[0] ? m_shaderVariant : "unlit";
  ShaderSet next;
  if (!next.compileVariant(m_device.Get(), m_shaderDir, variant, fb)) {
    return false;
  }
  m_shaders = next;
  DebugDraw debugNext;
  debugNext.create(m_device.Get(), m_shaderDir);
  if (debugNext.valid()) {
    m_debug = debugNext;
  }
  return true;
}

std::string D3D11Renderer::adapterNameUtf8() const {
  if (!m_adapter) {
    return std::string();
  }
  DXGI_ADAPTER_DESC desc;
  if (FAILED(m_adapter->GetDesc(&desc))) {
    return std::string();
  }
  const wchar_t* w = desc.Description;
  if (!w || !w[0]) {
    return std::string();
  }
  const int n = WideCharToMultiByte(CP_UTF8, 0, w, -1, 0, 0, 0, 0);
  if (n <= 1) {
    return std::string();
  }
  std::string out(static_cast<size_t>(n - 1), '\0');
  WideCharToMultiByte(CP_UTF8, 0, w, -1, &out[0], n, 0, 0);
  return out;
}

bool D3D11Renderer::createLabStates(FeedbackQueue* fb) {
  const D3D11_FILL_MODE fills[2] = { D3D11_FILL_SOLID, D3D11_FILL_WIREFRAME };
  const D3D11_CULL_MODE culls[3] = { D3D11_CULL_NONE, D3D11_CULL_FRONT, D3D11_CULL_BACK };
  for (int fi = 0; fi < 2; ++fi) {
    for (int ci = 0; ci < 3; ++ci) {
      D3D11_RASTERIZER_DESC rd = {};
      rd.FillMode = fills[fi];
      rd.CullMode = culls[ci];
      rd.DepthClipEnable = TRUE;
      const HRESULT hr = m_device->CreateRasterizerState(&rd, &m_raster[fi][ci]);
      if (FAILED(hr)) {
        pushFb(fb, FbError, "CreateRasterizerState failed");
        return false;
      }
    }
  }

  D3D11_DEPTH_STENCIL_DESC dsd = {};
  dsd.DepthEnable = TRUE;
  dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  dsd.DepthFunc = D3D11_COMPARISON_LESS;
  HRESULT hr = m_device->CreateDepthStencilState(&dsd, &m_depthOn);
  if (FAILED(hr)) {
    pushFb(fb, FbError, "CreateDepthStencilState failed");
    return false;
  }
  dsd.DepthEnable = FALSE;
  dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
  hr = m_device->CreateDepthStencilState(&dsd, &m_depthOff);
  if (FAILED(hr)) {
    pushFb(fb, FbError, "CreateDepthStencilState off failed");
    return false;
  }

  D3D11_BLEND_DESC bd = {};
  bd.RenderTarget[0].BlendEnable = TRUE;
  bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
  bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
  bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
  bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
  bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
  bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
  bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
  hr = m_device->CreateBlendState(&bd, &m_blend);
  if (FAILED(hr)) {
    pushFb(fb, FbError, "CreateBlendState failed");
    return false;
  }
  return true;
}

bool D3D11Renderer::createEdgeBuffers(FeedbackQueue* fb) {
  (void)fb;
  m_cubeEdges.Reset();
  m_sphereEdges.Reset();
  m_cylEdges.Reset();
  m_cubeEdgeVerts = 0;
  m_sphereEdgeVerts = 0;
  m_cylEdgeVerts = 0;
  if (!m_device || !m_debug.valid()) {
    return false;
  }

  std::vector<MeshVertex> verts;
  std::vector<unsigned short> indices;
  std::vector<MeshEdge> edges;
  bool ok = true;

  buildCube(&verts, &indices);
  buildSilhouetteEdges(verts, indices, &edges, 1.f);
  if (!m_debug.createEdgeVertexBuffer(m_device.Get(), edges, &m_cubeEdges, &m_cubeEdgeVerts)) {
    ok = false;
  }

  buildSphere(&verts, &indices, kSphereSlices, kSphereStacks);
  buildSilhouetteEdges(verts, indices, &edges, 1.f);
  if (!m_debug.createEdgeVertexBuffer(m_device.Get(), edges, &m_sphereEdges, &m_sphereEdgeVerts)) {
    ok = false;
  }

  buildCylinder(&verts, &indices, 24);
  buildSilhouetteEdges(verts, indices, &edges, 1.f);
  if (!m_debug.createEdgeVertexBuffer(m_device.Get(), edges, &m_cylEdges, &m_cylEdgeVerts)) {
    ok = false;
  }
  return ok;
}

void D3D11Renderer::bindLabStates(const StateSnapshot& snap) {
  const int fill = labEffectiveFillMode(snap.lab.fillMode, snap.teaching.shading == ShadeWire);
  const int cull = labEffectiveCullMode(snap.lab.cullMode);
  const int fi = (fill == 2) ? 1 : 0;
  int ci = cull - 1;
  if (ci < 0 || ci > 2) {
    ci = 2;
  }
  m_context->RSSetState(m_raster[fi][ci].Get());
  m_context->OMSetDepthStencilState(snap.lab.depthEnable ? m_depthOn.Get() : m_depthOff.Get(), 0);
  const float blendFactor[4] = {0.f, 0.f, 0.f, 0.f};
  m_context->OMSetBlendState(m_blend.Get(), blendFactor, 0xffffffff);
}

void D3D11Renderer::bindMeshPipeline(const StateSnapshot& snap) {
  ID3D11Buffer* cbuf = m_cb.Get();
  m_context->VSSetConstantBuffers(0, 1, &cbuf);
  m_context->PSSetConstantBuffers(0, 1, &cbuf);
  m_context->VSSetShader(m_shaders.vs(), 0, 0);
  m_context->PSSetShader(m_shaders.ps(), 0, 0);
  m_context->IASetInputLayout(m_shaders.layout());
  bindLabStates(snap);
}

void D3D11Renderer::drawAxisCones(const StateSnapshot& snap,
                                 FXMMATRIX view,
                                 FXMMATRIX proj,
                                 FXMVECTOR origin,
                                 float axisLen) {
  (void)snap;
  if (!m_context || !m_cone.valid() || !m_cb) {
    return;
  }

  ID3D11Buffer* cbuf = m_cb.Get();
  m_context->VSSetConstantBuffers(0, 1, &cbuf);
  m_context->PSSetConstantBuffers(0, 1, &cbuf);
  m_context->VSSetShader(m_shaders.vs(), 0, 0);
  m_context->PSSetShader(m_shaders.ps(), 0, 0);
  m_context->IASetInputLayout(m_shaders.layout());
  m_context->RSSetState(m_raster[0][0].Get());
  m_context->OMSetDepthStencilState(m_depthOn.Get(), 0);
  const float blendFactor[4] = {0.f, 0.f, 0.f, 0.f};
  m_context->OMSetBlendState(m_blend.Get(), blendFactor, 0xffffffff);

  const XMVECTOR dirs[3] = {
      XMVectorSet(1.f, 0.f, 0.f, 0.f),
      XMVectorSet(0.f, 1.f, 0.f, 0.f),
      XMVectorSet(0.f, 0.f, 1.f, 0.f),
  };
  const XMFLOAT4 colors[3] = {
      XMFLOAT4(1.f, 0x44 / 255.f, 0x44 / 255.f, 1.f),
      XMFLOAT4(0x44 / 255.f, 1.f, 0x44 / 255.f, 1.f),
      XMFLOAT4(0x44 / 255.f, 0x44 / 255.f, 1.f, 1.f),
  };

  for (int i = 0; i < 3; ++i) {
    const XMMATRIX rot = rotationYToDir(dirs[i]);
    const XMMATRIX T = XMMatrixTranslationFromVector(
        XMVectorAdd(origin, XMVectorScale(dirs[i], axisLen)));
    const XMMATRIX W = rot * T;
    const XMFLOAT4 emit(colors[i].x * 0.35f, colors[i].y * 0.35f, colors[i].z * 0.35f, 0.f);
    writeFrameCb(m_context.Get(), m_cb.Get(), W, view, proj, 0.f, colors[i], emit);
    m_cone.draw(m_context.Get());
  }
}

void D3D11Renderer::copyVariantName(const char* name) {
  std::memset(m_shaderVariant, 0, sizeof(m_shaderVariant));
  if (!name) {
    return;
  }
  for (int i = 0; name[i] && i < 31; ++i) {
    m_shaderVariant[i] = name[i];
  }
}

void D3D11Renderer::copyTriedName(const char* name) {
  std::memset(m_triedVariant, 0, sizeof(m_triedVariant));
  if (!name) {
    return;
  }
  for (int i = 0; name[i] && i < 31; ++i) {
    m_triedVariant[i] = name[i];
  }
}

void D3D11Renderer::selectMultisampleLevel() {
  m_sampleCount = 1;
  m_sampleQuality = 0;
  if (!m_device) {
    return;
  }
  static const UINT kCandidates[] = { 4, 2, 1 };
  for (size_t i = 0; i < sizeof(kCandidates) / sizeof(kCandidates[0]); ++i) {
    const UINT count = kCandidates[i];
    UINT levels = 0;
    m_device->CheckMultisampleQualityLevels(
        DXGI_FORMAT_R8G8B8A8_UNORM, count, &levels);
    if (levels > 0) {
      m_sampleCount = count;
      m_sampleQuality = 0;
      return;
    }
  }
}

void D3D11Renderer::attachInfoQueue() {
#ifdef _DEBUG
  m_infoQueue.Reset();
  if (!m_device) {
    return;
  }
  m_device.As(&m_infoQueue);
#else
  m_infoQueue.Reset();
#endif
}

void D3D11Renderer::pumpInfoQueue() {
#ifdef _DEBUG
  if (!m_infoQueue) {
    return;
  }
  const UINT64 stored = m_infoQueue->GetNumStoredMessages();
  const UINT64 n = (stored > 64ull) ? 64ull : stored;
  for (UINT64 i = 0; i < n; ++i) {
    SIZE_T bytes = 0;
    m_infoQueue->GetMessage(i, 0, &bytes);
    if (bytes == 0) {
      continue;
    }
    std::vector<char> buf(bytes);
    D3D11_MESSAGE* msg = reinterpret_cast<D3D11_MESSAGE*>(&buf[0]);
    if (FAILED(m_infoQueue->GetMessage(i, msg, &bytes))) {
      continue;
    }
    FeedbackKind kind = FbLog;
    if (msg->Severity == D3D11_MESSAGE_SEVERITY_CORRUPTION ||
        msg->Severity == D3D11_MESSAGE_SEVERITY_ERROR) {
      kind = FbError;
    } else if (msg->Severity == D3D11_MESSAGE_SEVERITY_WARNING) {
      kind = FbWarn;
    }
    if (msg->pDescription && msg->DescriptionByteLength > 0) {
      std::string text(msg->pDescription, msg->pDescription + msg->DescriptionByteLength);
      while (!text.empty()) {
        const char c = text[text.size() - 1];
        if (c != '\0' && c != '\n' && c != '\r') {
          break;
        }
        text.resize(text.size() - 1);
      }
      if (!text.empty()) {
        pushFb(m_fb, kind, text.c_str());
      }
    }
  }
  m_infoQueue->ClearStoredMessages();
#endif
}

bool D3D11Renderer::createSizeDependentResources() {
  if (!m_device || !m_swap) {
    return false;
  }
  m_rtv.Reset();
  m_dsv.Reset();
  m_depth.Reset();

  ComPtr<ID3D11Texture2D> back;
  HRESULT hr = m_swap->GetBuffer(
      0,
      __uuidof(ID3D11Texture2D),
      reinterpret_cast<void**>(back.ReleaseAndGetAddressOf()));
  if (FAILED(hr) || !back) {
    return false;
  }
  hr = m_device->CreateRenderTargetView(back.Get(), 0, m_rtv.ReleaseAndGetAddressOf());
  back.Reset();
  if (FAILED(hr) || !m_rtv) {
    return false;
  }

  D3D11_TEXTURE2D_DESC dd = {};
  dd.Width = static_cast<UINT>(m_w);
  dd.Height = static_cast<UINT>(m_h);
  dd.MipLevels = 1;
  dd.ArraySize = 1;
  dd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  dd.SampleDesc.Count = m_sampleCount;
  dd.SampleDesc.Quality = m_sampleQuality;
  dd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  hr = m_device->CreateTexture2D(&dd, 0, m_depth.ReleaseAndGetAddressOf());
  if (FAILED(hr) || !m_depth) {
    m_rtv.Reset();
    return false;
  }
  hr = m_device->CreateDepthStencilView(m_depth.Get(), 0, m_dsv.ReleaseAndGetAddressOf());
  if (FAILED(hr) || !m_dsv) {
    m_rtv.Reset();
    m_depth.Reset();
    return false;
  }
  return true;
}

void D3D11Renderer::handlePresentResult(HRESULT hr) {
  if (hr != DXGI_ERROR_DEVICE_REMOVED && hr != DXGI_ERROR_DEVICE_RESET) {
    return;
  }
  pushFb(m_fb, FbDeviceLost, "");
  // Do not recreate the swapchain on the render thread — HWND ownership is on
  // the UI thread. Mark dead and wait for a UI-side reinit.
  shutdown();
  m_dead = true;
}
