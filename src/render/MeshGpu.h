#pragma once
#include "teach/MeshBuild.h"
#include <d3d11.h>
#include <wrl/client.h>
#include <vector>

class MeshGpu {
public:
  static MeshGpu createFromCpu(ID3D11Device* device,
                               const std::vector<MeshVertex>& verts,
                               const std::vector<unsigned short>& indices);
  static MeshGpu createCube(ID3D11Device* device);
  static MeshGpu createSphere(ID3D11Device* device, int slices, int stacks);
  static MeshGpu createCylinder(ID3D11Device* device, int slices);
  void draw(ID3D11DeviceContext* context) const;
  bool valid() const { return m_vb && m_ib && m_indexCount > 0; }
private:
  Microsoft::WRL::ComPtr<ID3D11Buffer> m_vb;
  Microsoft::WRL::ComPtr<ID3D11Buffer> m_ib;
  UINT m_indexCount = 0;
};
