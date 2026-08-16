#pragma once
#include "render/FeedbackQueue.h"
#include <d3d11.h>
#include <wrl/client.h>
#include <string>

class ShaderSet {
public:
  bool compileFromFile(ID3D11Device* device, const std::wstring& path, FeedbackQueue* fb);
  bool compileVariant(ID3D11Device* device, const std::wstring& shaderDir, const char* variant, FeedbackQueue* fb);
  ID3D11VertexShader* vs() const { return m_vs.Get(); }
  ID3D11PixelShader* ps() const { return m_ps.Get(); }
  ID3D11InputLayout* layout() const { return m_layout.Get(); }
  bool valid() const { return m_vs && m_ps && m_layout; }
private:
  bool compileFromFileEx(ID3D11Device* device, const std::wstring& path, FeedbackQueue* fb, const void* macros);
  Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vs;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> m_ps;
  Microsoft::WRL::ComPtr<ID3D11InputLayout> m_layout;
};
