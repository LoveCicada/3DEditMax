#include "render/DebugDraw.h"
#include "teach/Transforms.h"
#include <d3dcompiler.h>
#include <cstring>

using Microsoft::WRL::ComPtr;
using namespace DirectX;

namespace {

const UINT kMaxLineVerts = 512;

struct LineVertex {
  XMFLOAT3 pos;
  XMFLOAT4 col;
};

struct LineCBCpu {
  XMFLOAT4X4 viewProj;
  XMFLOAT4 colorMul;
};

void pushLine(LineVertex* verts, UINT* count, UINT cap,
              FXMVECTOR a, FXMVECTOR b, const XMFLOAT4& col);

void pushGlyphX(LineVertex* verts, UINT* count, UINT cap, FXMVECTOR c, float s,
                const XMFLOAT4& col) {
  pushLine(verts, count, cap, XMVectorAdd(c, XMVectorSet(-s, s, 0.f, 0.f)),
           XMVectorAdd(c, XMVectorSet(s, -s, 0.f, 0.f)), col);
  pushLine(verts, count, cap, XMVectorAdd(c, XMVectorSet(-s, -s, 0.f, 0.f)),
           XMVectorAdd(c, XMVectorSet(s, s, 0.f, 0.f)), col);
}

void pushGlyphY(LineVertex* verts, UINT* count, UINT cap, FXMVECTOR c, float s,
                const XMFLOAT4& col) {
  pushLine(verts, count, cap, XMVectorAdd(c, XMVectorSet(-s, s, 0.f, 0.f)),
           XMVectorAdd(c, XMVectorSet(0.f, 0.f, 0.f, 0.f)), col);
  pushLine(verts, count, cap, XMVectorAdd(c, XMVectorSet(s, s, 0.f, 0.f)),
           XMVectorAdd(c, XMVectorSet(0.f, 0.f, 0.f, 0.f)), col);
  pushLine(verts, count, cap, c, XMVectorAdd(c, XMVectorSet(0.f, -s, 0.f, 0.f)), col);
}

void pushGlyphZ(LineVertex* verts, UINT* count, UINT cap, FXMVECTOR c, float s,
                const XMFLOAT4& col) {
  pushLine(verts, count, cap, XMVectorAdd(c, XMVectorSet(-s, s, 0.f, 0.f)),
           XMVectorAdd(c, XMVectorSet(s, s, 0.f, 0.f)), col);
  pushLine(verts, count, cap, XMVectorAdd(c, XMVectorSet(s, s, 0.f, 0.f)),
           XMVectorAdd(c, XMVectorSet(-s, -s, 0.f, 0.f)), col);
  pushLine(verts, count, cap, XMVectorAdd(c, XMVectorSet(-s, -s, 0.f, 0.f)),
           XMVectorAdd(c, XMVectorSet(s, -s, 0.f, 0.f)), col);
}

void pushLine(LineVertex* verts, UINT* count, UINT cap,
              FXMVECTOR a, FXMVECTOR b, const XMFLOAT4& col) {
  if (!verts || !count || *count + 2 > cap) {
    return;
  }
  LineVertex va;
  LineVertex vb;
  XMStoreFloat3(&va.pos, a);
  XMStoreFloat3(&vb.pos, b);
  va.col = col;
  vb.col = col;
  verts[*count] = va;
  verts[*count + 1] = vb;
  *count += 2;
}

void uploadLineCb(ID3D11DeviceContext* context, ID3D11Buffer* cb,
                  FXMMATRIX xform, const XMFLOAT4& colorMul) {
  LineCBCpu cpu;
  XMStoreFloat4x4(&cpu.viewProj, XMMatrixTranspose(xform));
  cpu.colorMul = colorMul;
  D3D11_MAPPED_SUBRESOURCE mapped = {};
  if (SUCCEEDED(context->Map(cb, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
    std::memcpy(mapped.pData, &cpu, sizeof(cpu));
    context->Unmap(cb, 0);
  }
}

}  // namespace

void DebugDraw::create(ID3D11Device* device) {
  create(device, std::wstring());
}

void DebugDraw::create(ID3D11Device* device, const std::wstring& shaderDir) {
  reset();
  if (!device) {
    return;
  }
  if (!compileLine(device, shaderDir)) {
    reset();
    return;
  }

  D3D11_BUFFER_DESC vbd = {};
  vbd.ByteWidth = kMaxLineVerts * sizeof(LineVertex);
  vbd.Usage = D3D11_USAGE_DYNAMIC;
  vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  if (FAILED(device->CreateBuffer(&vbd, 0, &m_vb))) {
    reset();
    return;
  }
  m_vbCapacity = kMaxLineVerts;

  D3D11_BUFFER_DESC cbd = {};
  cbd.ByteWidth = sizeof(LineCBCpu);
  cbd.Usage = D3D11_USAGE_DYNAMIC;
  cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  if (FAILED(device->CreateBuffer(&cbd, 0, &m_cb))) {
    reset();
    return;
  }

  D3D11_RASTERIZER_DESC rd = {};
  rd.FillMode = D3D11_FILL_SOLID;
  rd.CullMode = D3D11_CULL_NONE;
  rd.DepthClipEnable = TRUE;
  // Negative bias pulls debug lines slightly toward the camera under LESS depth.
  rd.DepthBias = -100000;
  rd.DepthBiasClamp = 0.f;
  rd.SlopeScaledDepthBias = -1.f;
  if (FAILED(device->CreateRasterizerState(&rd, &m_raster))) {
    reset();
    return;
  }

  D3D11_DEPTH_STENCIL_DESC dsd = {};
  dsd.DepthEnable = TRUE;
  dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
  dsd.DepthFunc = D3D11_COMPARISON_LESS;
  if (FAILED(device->CreateDepthStencilState(&dsd, &m_depth))) {
    reset();
    return;
  }
}

void DebugDraw::reset() {
  m_vs.Reset();
  m_ps.Reset();
  m_layout.Reset();
  m_vb.Reset();
  m_cb.Reset();
  m_raster.Reset();
  m_depth.Reset();
  m_vbCapacity = 0;
}

bool DebugDraw::compileLine(ID3D11Device* device, const std::wstring& shaderDir) {
  const std::wstring path = shaderDir + L"/line.hlsl";
  ComPtr<ID3DBlob> vsBlob;
  ComPtr<ID3DBlob> psBlob;
  ComPtr<ID3DBlob> err;
  HRESULT hr = D3DCompileFromFile(
      path.c_str(),
      0,
      D3D_COMPILE_STANDARD_FILE_INCLUDE,
      "vs_main",
      "vs_5_0",
      0,
      0,
      &vsBlob,
      &err);
  if (FAILED(hr)) {
    return false;
  }
  err.Reset();
  hr = D3DCompileFromFile(
      path.c_str(),
      0,
      D3D_COMPILE_STANDARD_FILE_INCLUDE,
      "ps_main",
      "ps_5_0",
      0,
      0,
      &psBlob,
      &err);
  if (FAILED(hr)) {
    return false;
  }

  hr = device->CreateVertexShader(
      vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), 0, &m_vs);
  if (FAILED(hr)) {
    return false;
  }
  hr = device->CreatePixelShader(
      psBlob->GetBufferPointer(), psBlob->GetBufferSize(), 0, &m_ps);
  if (FAILED(hr)) {
    return false;
  }

  D3D11_INPUT_ELEMENT_DESC elems[2] = {};
  elems[0].SemanticName = "POSITION";
  elems[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
  elems[0].AlignedByteOffset = 0;
  elems[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  elems[1].SemanticName = "COLOR";
  elems[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
  elems[1].AlignedByteOffset = 12;
  elems[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  hr = device->CreateInputLayout(
      elems,
      2,
      vsBlob->GetBufferPointer(),
      vsBlob->GetBufferSize(),
      &m_layout);
  return SUCCEEDED(hr);
}

void DebugDraw::bindLinePipeline(ID3D11DeviceContext* context) {
  ID3D11Buffer* cbuf = m_cb.Get();
  context->VSSetConstantBuffers(0, 1, &cbuf);
  context->PSSetConstantBuffers(0, 1, &cbuf);
  context->VSSetShader(m_vs.Get(), 0, 0);
  context->PSSetShader(m_ps.Get(), 0, 0);
  context->IASetInputLayout(m_layout.Get());
  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
  context->RSSetState(m_raster.Get());
  context->OMSetDepthStencilState(m_depth.Get(), 0);
}

bool DebugDraw::createEdgeVertexBuffer(ID3D11Device* device,
                                       const std::vector<MeshEdge>& edges,
                                       ComPtr<ID3D11Buffer>* outVb,
                                       UINT* outVertCount) {
  if (!device || !outVb || !outVertCount) {
    return false;
  }
  outVb->Reset();
  *outVertCount = 0;
  if (edges.empty()) {
    return false;
  }

  std::vector<LineVertex> verts(edges.size() * 2);
  const XMFLOAT4 white(1.f, 1.f, 1.f, 1.f);
  for (size_t i = 0; i < edges.size(); ++i) {
    LineVertex a;
    LineVertex b;
    a.pos = XMFLOAT3(edges[i].ax, edges[i].ay, edges[i].az);
    b.pos = XMFLOAT3(edges[i].bx, edges[i].by, edges[i].bz);
    a.col = white;
    b.col = white;
    verts[i * 2] = a;
    verts[i * 2 + 1] = b;
  }

  D3D11_BUFFER_DESC vbd = {};
  vbd.ByteWidth = static_cast<UINT>(verts.size() * sizeof(LineVertex));
  vbd.Usage = D3D11_USAGE_IMMUTABLE;
  vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  D3D11_SUBRESOURCE_DATA srd = {};
  srd.pSysMem = &verts[0];
  if (FAILED(device->CreateBuffer(&vbd, &srd, outVb->ReleaseAndGetAddressOf()))) {
    return false;
  }
  *outVertCount = static_cast<UINT>(verts.size());
  return true;
}

void DebugDraw::drawLineList(ID3D11DeviceContext* context,
                             ID3D11Buffer* vb,
                             UINT vertexCount,
                             FXMMATRIX wvp,
                             const XMFLOAT4& color) {
  if (!context || !valid() || !vb || vertexCount < 2) {
    return;
  }
  uploadLineCb(context, m_cb.Get(), wvp, color);
  bindLinePipeline(context);
  UINT stride = sizeof(LineVertex);
  UINT offset = 0;
  context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
  context->Draw(vertexCount, 0);
}

void DebugDraw::draw(ID3D11DeviceContext* context,
                     const StateSnapshot& snap,
                     FXMMATRIX view,
                     FXMMATRIX proj) {
  if (!context || !valid()) {
    return;
  }

  LineVertex verts[kMaxLineVerts];
  UINT count = 0;

  const XMFLOAT4 gridMajor(0x44 / 255.f, 0x44 / 255.f, 0x66 / 255.f, 1.f);
  const XMFLOAT4 gridMinor(0x33 / 255.f, 0x33 / 255.f, 0x55 / 255.f, 1.f);
  const float half = 10.f;
  const int divs = 20;
  const int center = divs / 2;
  for (int i = 0; i <= divs; ++i) {
    const float t = -half + (2.f * half) * (static_cast<float>(i) / static_cast<float>(divs));
    const XMFLOAT4& col = (i == center) ? gridMajor : gridMinor;
    pushLine(verts, &count, kMaxLineVerts,
             XMVectorSet(-half, 0.f, t, 0.f), XMVectorSet(half, 0.f, t, 0.f), col);
    pushLine(verts, &count, kMaxLineVerts,
             XMVectorSet(t, 0.f, -half, 0.f), XMVectorSet(t, 0.f, half, 0.f), col);
  }

  XMVECTOR detP = XMVectorZero();
  XMVECTOR detV = XMVectorZero();
  const XMMATRIX invP = XMMatrixInverse(&detP, proj);
  const XMMATRIX invV = XMMatrixInverse(&detV, view);
  if (XMVectorGetX(detP) != 0.f && XMVectorGetX(detV) != 0.f) {
    // D3D LH clip/NDC: x,y in [-1,1], z in [0,1].
    static const float kClip[8][3] = {
        {-1.f, -1.f, 0.f}, {1.f, -1.f, 0.f}, {1.f, 1.f, 0.f}, {-1.f, 1.f, 0.f},
        {-1.f, -1.f, 1.f}, {1.f, -1.f, 1.f}, {1.f, 1.f, 1.f}, {-1.f, 1.f, 1.f},
    };
    XMVECTOR corner[8];
    for (int i = 0; i < 8; ++i) {
      const XMVECTOR clip = XMVectorSet(kClip[i][0], kClip[i][1], kClip[i][2], 1.f);
      const XMVECTOR viewP = XMVector3TransformCoord(clip, invP);
      corner[i] = XMVector3TransformCoord(viewP, invV);
    }
    static const int kEdges[12][2] = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0},
        {4, 5}, {5, 6}, {6, 7}, {7, 4},
        {0, 4}, {1, 5}, {2, 6}, {3, 7},
    };
    const XMFLOAT4 yellow(1.f, 1.f, 0.2f, 1.f);
    for (int e = 0; e < 12; ++e) {
      pushLine(verts, &count, kMaxLineVerts, corner[kEdges[e][0]], corner[kEdges[e][1]], yellow);
    }
  }

  const XMVECTOR origin = XMVectorZero();
  const float axisLen = 3.f;
  pushLine(verts, &count, kMaxLineVerts, origin, XMVectorSet(axisLen, 0.f, 0.f, 0.f),
           XMFLOAT4(1.f, 0x44 / 255.f, 0x44 / 255.f, 1.f));
  pushLine(verts, &count, kMaxLineVerts, origin, XMVectorSet(0.f, axisLen, 0.f, 0.f),
           XMFLOAT4(0x44 / 255.f, 1.f, 0x44 / 255.f, 1.f));
  pushLine(verts, &count, kMaxLineVerts, origin, XMVectorSet(0.f, 0.f, axisLen, 0.f),
           XMFLOAT4(0x44 / 255.f, 0x44 / 255.f, 1.f, 1.f));

  const float glyph = 0.18f;
  const XMFLOAT4 red(1.f, 0x44 / 255.f, 0x44 / 255.f, 1.f);
  const XMFLOAT4 green(0x44 / 255.f, 1.f, 0x44 / 255.f, 1.f);
  const XMFLOAT4 blue(0x44 / 255.f, 0x44 / 255.f, 1.f, 1.f);
  pushGlyphX(verts, &count, kMaxLineVerts, XMVectorSet(axisLen + 0.4f, 0.f, 0.f, 0.f), glyph, red);
  pushGlyphY(verts, &count, kMaxLineVerts, XMVectorSet(0.f, axisLen + 0.4f, 0.f, 0.f), glyph, green);
  pushGlyphZ(verts, &count, kMaxLineVerts, XMVectorSet(0.f, 0.f, axisLen + 0.4f, 0.f), glyph, blue);

  const TeachingState& t = snap.teaching;
  const float* op = t.objects[0].trs.pos;
  const XMVECTOR gizmoO = XMVectorSet(op[0], op[1], op[2], 0.f);
  const float gizmoLen = worldAxisGizmoLength();
  pushLine(verts, &count, kMaxLineVerts, gizmoO, XMVectorAdd(gizmoO, XMVectorSet(gizmoLen, 0.f, 0.f, 0.f)), red);
  pushLine(verts, &count, kMaxLineVerts, gizmoO, XMVectorAdd(gizmoO, XMVectorSet(0.f, gizmoLen, 0.f, 0.f)), green);
  pushLine(verts, &count, kMaxLineVerts, gizmoO, XMVectorAdd(gizmoO, XMVectorSet(0.f, 0.f, gizmoLen, 0.f)), blue);

  const XMMATRIX W = BuildWorld(t.objects[0].trs);
  const XMFLOAT3 model(t.trackModel[0], t.trackModel[1], t.trackModel[2]);
  const TrackResult tr = TrackPoint(model, W, view, proj);
  const XMVECTOR wp = XMVectorSet(tr.world.x, tr.world.y, tr.world.z, 0.f);
  const float s = 0.12f;
  const XMVECTOR px = XMVectorAdd(wp, XMVectorSet(s, 0.f, 0.f, 0.f));
  const XMVECTOR nx = XMVectorAdd(wp, XMVectorSet(-s, 0.f, 0.f, 0.f));
  const XMVECTOR py = XMVectorAdd(wp, XMVectorSet(0.f, s, 0.f, 0.f));
  const XMVECTOR ny = XMVectorAdd(wp, XMVectorSet(0.f, -s, 0.f, 0.f));
  const XMVECTOR pz = XMVectorAdd(wp, XMVectorSet(0.f, 0.f, s, 0.f));
  const XMVECTOR nz = XMVectorAdd(wp, XMVectorSet(0.f, 0.f, -s, 0.f));
  const XMFLOAT4 mag(1.f, 0.35f, 1.f, 1.f);
  pushLine(verts, &count, kMaxLineVerts, px, py, mag);
  pushLine(verts, &count, kMaxLineVerts, px, ny, mag);
  pushLine(verts, &count, kMaxLineVerts, px, pz, mag);
  pushLine(verts, &count, kMaxLineVerts, px, nz, mag);
  pushLine(verts, &count, kMaxLineVerts, nx, py, mag);
  pushLine(verts, &count, kMaxLineVerts, nx, ny, mag);
  pushLine(verts, &count, kMaxLineVerts, nx, pz, mag);
  pushLine(verts, &count, kMaxLineVerts, nx, nz, mag);
  pushLine(verts, &count, kMaxLineVerts, py, pz, mag);
  pushLine(verts, &count, kMaxLineVerts, py, nz, mag);
  pushLine(verts, &count, kMaxLineVerts, ny, pz, mag);
  pushLine(verts, &count, kMaxLineVerts, ny, nz, mag);

  if (count == 0) {
    return;
  }

  const XMMATRIX viewProj = view * proj;
  uploadLineCb(context, m_cb.Get(), viewProj, XMFLOAT4(1.f, 1.f, 1.f, 1.f));
  D3D11_MAPPED_SUBRESOURCE mapped = {};
  if (SUCCEEDED(context->Map(m_vb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
    std::memcpy(mapped.pData, verts, count * sizeof(LineVertex));
    context->Unmap(m_vb.Get(), 0);
  }

  bindLinePipeline(context);
  UINT stride = sizeof(LineVertex);
  UINT offset = 0;
  ID3D11Buffer* vb = m_vb.Get();
  context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
  context->Draw(count, 0);
}
