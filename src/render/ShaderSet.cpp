#include "render/ShaderSet.h"
#include <d3dcompiler.h>

static void pushCompileError(FeedbackQueue* fb, ID3DBlob* err) {
  if (!fb) {
    return;
  }
  FeedbackItem item;
  item.kind = FbError;
  item.ms = 0.f;
  if (err && err->GetBufferPointer() && err->GetBufferSize() > 0) {
    const char* text = static_cast<const char*>(err->GetBufferPointer());
    item.text.assign(text, text + err->GetBufferSize());
  } else {
    item.text = "HLSL compile failed";
  }
  fb->push(item);
}

bool ShaderSet::compileVariant(ID3D11Device* device, const std::wstring& shaderDir, const char* variant, FeedbackQueue* fb) {
  const char* name = (variant && variant[0]) ? variant : "unlit";
  D3D_SHADER_MACRO macros[2] = {};
  macros[0].Name = "SHADER_VARIANT";
  macros[0].Definition = name;
  const std::wstring path = shaderDir + L"/unlit.hlsl";
  return compileFromFileEx(device, path, fb, macros);
}

bool ShaderSet::compileFromFile(ID3D11Device* device, const std::wstring& path, FeedbackQueue* fb) {
  return compileFromFileEx(device, path, fb, 0);
}

bool ShaderSet::compileFromFileEx(ID3D11Device* device, const std::wstring& path, FeedbackQueue* fb, const void* macros) {
  const D3D_SHADER_MACRO* defs = static_cast<const D3D_SHADER_MACRO*>(macros);
  if (!device) {
    if (fb) {
      FeedbackItem item;
      item.kind = FbError;
      item.text = "ShaderSet: null device";
      item.ms = 0.f;
      fb->push(item);
    }
    return false;
  }

  Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
  Microsoft::WRL::ComPtr<ID3DBlob> psBlob;
  Microsoft::WRL::ComPtr<ID3DBlob> err;

  HRESULT hr = D3DCompileFromFile(
      path.c_str(),
      defs,
      D3D_COMPILE_STANDARD_FILE_INCLUDE,
      "vs_main",
      "vs_5_0",
      0,
      0,
      &vsBlob,
      &err);
  if (FAILED(hr)) {
    pushCompileError(fb, err.Get());
    return false;
  }

  err.Reset();
  hr = D3DCompileFromFile(
      path.c_str(),
      defs,
      D3D_COMPILE_STANDARD_FILE_INCLUDE,
      "ps_main",
      "ps_5_0",
      0,
      0,
      &psBlob,
      &err);
  if (FAILED(hr)) {
    pushCompileError(fb, err.Get());
    return false;
  }

  Microsoft::WRL::ComPtr<ID3D11VertexShader> vs;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> ps;
  Microsoft::WRL::ComPtr<ID3D11InputLayout> layout;

  hr = device->CreateVertexShader(
      vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), 0, &vs);
  if (FAILED(hr)) {
    if (fb) {
      FeedbackItem item;
      item.kind = FbError;
      item.text = "CreateVertexShader failed";
      item.ms = 0.f;
      fb->push(item);
    }
    return false;
  }

  hr = device->CreatePixelShader(
      psBlob->GetBufferPointer(), psBlob->GetBufferSize(), 0, &ps);
  if (FAILED(hr)) {
    if (fb) {
      FeedbackItem item;
      item.kind = FbError;
      item.text = "CreatePixelShader failed";
      item.ms = 0.f;
      fb->push(item);
    }
    return false;
  }

  D3D11_INPUT_ELEMENT_DESC elems[3] = {};
  elems[0].SemanticName = "POSITION";
  elems[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
  elems[0].AlignedByteOffset = 0;
  elems[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  elems[1].SemanticName = "NORMAL";
  elems[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
  elems[1].AlignedByteOffset = 12;
  elems[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  elems[2].SemanticName = "TEXCOORD";
  elems[2].Format = DXGI_FORMAT_R32G32_FLOAT;
  elems[2].AlignedByteOffset = 24;
  elems[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

  hr = device->CreateInputLayout(
      elems,
      3,
      vsBlob->GetBufferPointer(),
      vsBlob->GetBufferSize(),
      &layout);
  if (FAILED(hr)) {
    if (fb) {
      FeedbackItem item;
      item.kind = FbError;
      item.text = "CreateInputLayout failed";
      item.ms = 0.f;
      fb->push(item);
    }
    return false;
  }

  m_vs = vs;
  m_ps = ps;
  m_layout = layout;
  return true;
}
