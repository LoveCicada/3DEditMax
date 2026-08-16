#pragma once
#include <d3d11.h>
#include <wrl/client.h>

struct MeshVertex {
  float px, py, pz;
  float nx, ny, nz;
  float u, v;
};

class MeshGpu {
public:
  static MeshGpu createCube(ID3D11Device* device);
  static MeshGpu createSphere(ID3D11Device* device);
  static MeshGpu createCylinder(ID3D11Device* device);
  void draw(ID3D11DeviceContext* context) const;
  bool valid() const { return m_vb && m_ib && m_indexCount > 0; }
private:
  Microsoft::WRL::ComPtr<ID3D11Buffer> m_vb;
  Microsoft::WRL::ComPtr<ID3D11Buffer> m_ib;
  UINT m_indexCount = 0;
};
