#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <string>

class AxisLabels {
public:
  void create(ID3D11Device* device, const std::wstring& shaderDir);
  void reset();
  void draw(ID3D11DeviceContext* context,
            DirectX::FXMMATRIX view,
            DirectX::FXMMATRIX proj);
  bool valid() const {
    return m_vs && m_ps && m_layout && m_vb && m_cb && m_samp && m_blend &&
           m_depth && m_srv[0] && m_srv[1] && m_srv[2];
  }

private:
  bool compile(ID3D11Device* device, const std::wstring& shaderDir);
  bool createGlyphTextures(ID3D11Device* device);

  Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vs;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> m_ps;
  Microsoft::WRL::ComPtr<ID3D11InputLayout> m_layout;
  Microsoft::WRL::ComPtr<ID3D11Buffer> m_vb;
  Microsoft::WRL::ComPtr<ID3D11Buffer> m_cb;
  Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samp;
  Microsoft::WRL::ComPtr<ID3D11BlendState> m_blend;
  Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depth;
  Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_raster;
  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv[3];
};
