#pragma once
#include "core/StateSnapshot.h"
#include "teach/MeshBuild.h"
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <string>
#include <vector>

class DebugDraw {
public:
  void create(ID3D11Device* device);
  void create(ID3D11Device* device, const std::wstring& shaderDir);
  void reset();
  void draw(ID3D11DeviceContext* context,
            const StateSnapshot& snap,
            DirectX::FXMMATRIX view,
            DirectX::FXMMATRIX proj);
  bool createEdgeVertexBuffer(ID3D11Device* device,
                              const std::vector<MeshEdge>& edges,
                              Microsoft::WRL::ComPtr<ID3D11Buffer>* outVb,
                              UINT* outVertCount);
  void drawLineList(ID3D11DeviceContext* context,
                    ID3D11Buffer* vb,
                    UINT vertexCount,
                    DirectX::FXMMATRIX wvp,
                    const DirectX::XMFLOAT4& color);
  bool valid() const {
    return m_vs && m_ps && m_layout && m_vb && m_cb && m_raster && m_depth;
  }
private:
  bool compileLine(ID3D11Device* device, const std::wstring& shaderDir);
  void bindLinePipeline(ID3D11DeviceContext* context);

  Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vs;
  Microsoft::WRL::ComPtr<ID3D11PixelShader> m_ps;
  Microsoft::WRL::ComPtr<ID3D11InputLayout> m_layout;
  Microsoft::WRL::ComPtr<ID3D11Buffer> m_vb;
  Microsoft::WRL::ComPtr<ID3D11Buffer> m_cb;
  Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_raster;
  Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depth;
  UINT m_vbCapacity = 0;
};
