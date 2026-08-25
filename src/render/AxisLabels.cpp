#include "render/AxisLabels.h"
#include <d3dcompiler.h>
#include <cmath>
#include <cstring>
#include <vector>

using Microsoft::WRL::ComPtr;
using namespace DirectX;

namespace {

const int kGlyphSize = 64;
const float kAxisTip = 3.4f;
const float kHalf = 0.28f;

struct SpriteVertex {
  XMFLOAT3 pos;
  XMFLOAT2 uv;
  XMFLOAT4 col;
};

struct SpriteCBCpu {
  XMFLOAT4X4 viewProj;
  XMFLOAT4 colorMul;
};

float distPointSeg2(float px, float py, float ax, float ay, float bx, float by) {
  const float abx = bx - ax;
  const float aby = by - ay;
  const float apx = px - ax;
  const float apy = py - ay;
  const float ab2 = abx * abx + aby * aby;
  float t = 0.f;
  if (ab2 > 1e-8f) {
    t = (apx * abx + apy * aby) / ab2;
    if (t < 0.f) {
      t = 0.f;
    }
    if (t > 1.f) {
      t = 1.f;
    }
  }
  const float dx = px - (ax + abx * t);
  const float dy = py - (ay + aby * t);
  return dx * dx + dy * dy;
}

float softStroke(float d2, float halfWidth, float soft) {
  const float d = sqrtf(d2);
  const float a = 1.f - (d - halfWidth) / soft;
  if (a <= 0.f) {
    return 0.f;
  }
  if (a >= 1.f) {
    return 1.f;
  }
  return a;
}

void rasterGlyph(char letter, unsigned char r, unsigned char g, unsigned char b,
                 std::vector<unsigned char>* outRgba) {
  outRgba->assign(static_cast<size_t>(kGlyphSize * kGlyphSize * 4), 0);
  const float halfW = 4.2f;
  const float soft = 1.8f;
  for (int y = 0; y < kGlyphSize; ++y) {
    for (int x = 0; x < kGlyphSize; ++x) {
      const float px = static_cast<float>(x) + 0.5f;
      const float py = static_cast<float>(y) + 0.5f;
      float cover = 0.f;
      if (letter == 'X') {
        cover = softStroke(distPointSeg2(px, py, 14.f, 14.f, 50.f, 50.f), halfW, soft);
        const float c2 = softStroke(distPointSeg2(px, py, 50.f, 14.f, 14.f, 50.f), halfW, soft);
        if (c2 > cover) {
          cover = c2;
        }
      } else if (letter == 'Y') {
        cover = softStroke(distPointSeg2(px, py, 14.f, 14.f, 32.f, 34.f), halfW, soft);
        float c2 = softStroke(distPointSeg2(px, py, 50.f, 14.f, 32.f, 34.f), halfW, soft);
        if (c2 > cover) {
          cover = c2;
        }
        c2 = softStroke(distPointSeg2(px, py, 32.f, 34.f, 32.f, 52.f), halfW, soft);
        if (c2 > cover) {
          cover = c2;
        }
      } else {
        cover = softStroke(distPointSeg2(px, py, 14.f, 16.f, 50.f, 16.f), halfW, soft);
        float c2 = softStroke(distPointSeg2(px, py, 50.f, 16.f, 14.f, 48.f), halfW, soft);
        if (c2 > cover) {
          cover = c2;
        }
        c2 = softStroke(distPointSeg2(px, py, 14.f, 48.f, 50.f, 48.f), halfW, soft);
        if (c2 > cover) {
          cover = c2;
        }
      }
      if (cover <= 0.f) {
        continue;
      }
      const size_t i = static_cast<size_t>((y * kGlyphSize + x) * 4);
      (*outRgba)[i + 0] = r;
      (*outRgba)[i + 1] = g;
      (*outRgba)[i + 2] = b;
      (*outRgba)[i + 3] = static_cast<unsigned char>(cover * 255.f + 0.5f);
    }
  }
}

bool createOneGlyph(ID3D11Device* device, char letter, unsigned char r, unsigned char g,
                    unsigned char b, ComPtr<ID3D11ShaderResourceView>* outSrv) {
  std::vector<unsigned char> pixels;
  rasterGlyph(letter, r, g, b, &pixels);

  D3D11_TEXTURE2D_DESC td = {};
  td.Width = kGlyphSize;
  td.Height = kGlyphSize;
  td.MipLevels = 1;
  td.ArraySize = 1;
  td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  td.SampleDesc.Count = 1;
  td.Usage = D3D11_USAGE_IMMUTABLE;
  td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

  D3D11_SUBRESOURCE_DATA init = {};
  init.pSysMem = pixels.data();
  init.SysMemPitch = kGlyphSize * 4;

  ComPtr<ID3D11Texture2D> tex;
  if (FAILED(device->CreateTexture2D(&td, &init, &tex))) {
    return false;
  }
  D3D11_SHADER_RESOURCE_VIEW_DESC sd = {};
  sd.Format = td.Format;
  sd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  sd.Texture2D.MipLevels = 1;
  return SUCCEEDED(device->CreateShaderResourceView(tex.Get(), &sd, outSrv->ReleaseAndGetAddressOf()));
}

void writeBillboard(SpriteVertex* out, FXMVECTOR center, FXMVECTOR right, FXMVECTOR up,
                    float half, const XMFLOAT4& col) {
  const XMVECTOR r = XMVectorScale(right, half);
  const XMVECTOR u = XMVectorScale(up, half);
  const XMVECTOR p0 = XMVectorSubtract(XMVectorSubtract(center, r), u);
  const XMVECTOR p1 = XMVectorSubtract(XMVectorAdd(center, r), u);
  const XMVECTOR p2 = XMVectorAdd(XMVectorAdd(center, r), u);
  const XMVECTOR p3 = XMVectorAdd(XMVectorSubtract(center, r), u);
  const XMFLOAT2 uv0(0.f, 1.f);
  const XMFLOAT2 uv1(1.f, 1.f);
  const XMFLOAT2 uv2(1.f, 0.f);
  const XMFLOAT2 uv3(0.f, 0.f);
  XMFLOAT3 a, b, c, d;
  XMStoreFloat3(&a, p0);
  XMStoreFloat3(&b, p1);
  XMStoreFloat3(&c, p2);
  XMStoreFloat3(&d, p3);
  out[0] = {a, uv0, col};
  out[1] = {b, uv1, col};
  out[2] = {c, uv2, col};
  out[3] = {a, uv0, col};
  out[4] = {c, uv2, col};
  out[5] = {d, uv3, col};
}

}  // namespace

void AxisLabels::create(ID3D11Device* device, const std::wstring& shaderDir) {
  reset();
  if (!device) {
    return;
  }
  if (!compile(device, shaderDir)) {
    reset();
    return;
  }
  if (!createGlyphTextures(device)) {
    reset();
    return;
  }

  D3D11_BUFFER_DESC vbd = {};
  vbd.ByteWidth = sizeof(SpriteVertex) * 18;
  vbd.Usage = D3D11_USAGE_DYNAMIC;
  vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  if (FAILED(device->CreateBuffer(&vbd, 0, &m_vb))) {
    reset();
    return;
  }

  D3D11_BUFFER_DESC cbd = {};
  cbd.ByteWidth = sizeof(SpriteCBCpu);
  cbd.Usage = D3D11_USAGE_DYNAMIC;
  cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  if (FAILED(device->CreateBuffer(&cbd, 0, &m_cb))) {
    reset();
    return;
  }

  D3D11_SAMPLER_DESC samp = {};
  samp.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  samp.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
  samp.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
  samp.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
  samp.MaxLOD = D3D11_FLOAT32_MAX;
  if (FAILED(device->CreateSamplerState(&samp, &m_samp))) {
    reset();
    return;
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
  if (FAILED(device->CreateBlendState(&bd, &m_blend))) {
    reset();
    return;
  }

  D3D11_DEPTH_STENCIL_DESC dsd = {};
  dsd.DepthEnable = TRUE;
  dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
  dsd.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
  if (FAILED(device->CreateDepthStencilState(&dsd, &m_depth))) {
    reset();
    return;
  }

  D3D11_RASTERIZER_DESC rd = {};
  rd.FillMode = D3D11_FILL_SOLID;
  rd.CullMode = D3D11_CULL_NONE;
  rd.DepthClipEnable = TRUE;
  if (FAILED(device->CreateRasterizerState(&rd, &m_raster))) {
    reset();
    return;
  }
}

void AxisLabels::reset() {
  m_vs.Reset();
  m_ps.Reset();
  m_layout.Reset();
  m_vb.Reset();
  m_cb.Reset();
  m_samp.Reset();
  m_blend.Reset();
  m_depth.Reset();
  m_raster.Reset();
  m_srv[0].Reset();
  m_srv[1].Reset();
  m_srv[2].Reset();
}

bool AxisLabels::compile(ID3D11Device* device, const std::wstring& shaderDir) {
  const std::wstring path = shaderDir + L"/sprite.hlsl";
  ComPtr<ID3DBlob> vsBlob;
  ComPtr<ID3DBlob> psBlob;
  ComPtr<ID3DBlob> err;
  HRESULT hr = D3DCompileFromFile(path.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE,
                                  "vs_main", "vs_5_0", 0, 0, &vsBlob, &err);
  if (FAILED(hr)) {
    return false;
  }
  err.Reset();
  hr = D3DCompileFromFile(path.c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE,
                          "ps_main", "ps_5_0", 0, 0, &psBlob, &err);
  if (FAILED(hr)) {
    return false;
  }
  if (FAILED(device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), 0,
                                        &m_vs))) {
    return false;
  }
  if (FAILED(device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), 0,
                                       &m_ps))) {
    return false;
  }
  const D3D11_INPUT_ELEMENT_DESC layout[] = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0},
  };
  return SUCCEEDED(device->CreateInputLayout(layout, 3, vsBlob->GetBufferPointer(),
                                            vsBlob->GetBufferSize(), &m_layout));
}

bool AxisLabels::createGlyphTextures(ID3D11Device* device) {
  if (!createOneGlyph(device, 'X', 0xef, 0x47, 0x6f, &m_srv[0])) {
    return false;
  }
  if (!createOneGlyph(device, 'Y', 0x06, 0xd6, 0xa0, &m_srv[1])) {
    return false;
  }
  if (!createOneGlyph(device, 'Z', 0x4c, 0xc9, 0xf0, &m_srv[2])) {
    return false;
  }
  return true;
}

void AxisLabels::draw(ID3D11DeviceContext* context, FXMMATRIX view, FXMMATRIX proj) {
  if (!context || !valid()) {
    return;
  }

  XMFLOAT4X4 vf;
  XMStoreFloat4x4(&vf, view);
  XMVECTOR right = XMVector3Normalize(XMVectorSet(vf._11, vf._21, vf._31, 0.f));
  XMVECTOR up = XMVector3Normalize(XMVectorSet(vf._12, vf._22, vf._32, 0.f));

  const XMFLOAT4 white(1.f, 1.f, 1.f, 1.f);
  SpriteVertex verts[18];
  writeBillboard(verts + 0, XMVectorSet(kAxisTip, 0.f, 0.f, 0.f), right, up, kHalf, white);
  writeBillboard(verts + 6, XMVectorSet(0.f, kAxisTip, 0.f, 0.f), right, up, kHalf, white);
  writeBillboard(verts + 12, XMVectorSet(0.f, 0.f, kAxisTip, 0.f), right, up, kHalf, white);

  D3D11_MAPPED_SUBRESOURCE mapped = {};
  if (FAILED(context->Map(m_vb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
    return;
  }
  std::memcpy(mapped.pData, verts, sizeof(verts));
  context->Unmap(m_vb.Get(), 0);

  SpriteCBCpu cb;
  XMStoreFloat4x4(&cb.viewProj, XMMatrixTranspose(view * proj));
  cb.colorMul = XMFLOAT4(1.f, 1.f, 1.f, 1.f);
  if (FAILED(context->Map(m_cb.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
    return;
  }
  std::memcpy(mapped.pData, &cb, sizeof(cb));
  context->Unmap(m_cb.Get(), 0);

  context->IASetInputLayout(m_layout.Get());
  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  UINT stride = sizeof(SpriteVertex);
  UINT offset = 0;
  ID3D11Buffer* vb = m_vb.Get();
  context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
  ID3D11Buffer* cbuf = m_cb.Get();
  context->VSSetConstantBuffers(0, 1, &cbuf);
  context->PSSetConstantBuffers(0, 1, &cbuf);
  context->VSSetShader(m_vs.Get(), 0, 0);
  context->PSSetShader(m_ps.Get(), 0, 0);
  context->PSSetSamplers(0, 1, m_samp.GetAddressOf());
  context->RSSetState(m_raster.Get());
  context->OMSetDepthStencilState(m_depth.Get(), 0);
  const float blendFactor[4] = {0.f, 0.f, 0.f, 0.f};
  context->OMSetBlendState(m_blend.Get(), blendFactor, 0xffffffff);

  for (int i = 0; i < 3; ++i) {
    ID3D11ShaderResourceView* srv = m_srv[i].Get();
    context->PSSetShaderResources(0, 1, &srv);
    context->Draw(6, static_cast<UINT>(i * 6));
  }
  ID3D11ShaderResourceView* nullSrv = 0;
  context->PSSetShaderResources(0, 1, &nullSrv);
}
