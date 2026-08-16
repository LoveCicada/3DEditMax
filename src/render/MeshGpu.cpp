#include "render/MeshGpu.h"

static_assert(sizeof(MeshVertex) == 32, "MeshVertex is 32 bytes");

MeshGpu MeshGpu::createCube(ID3D11Device* device) {
  MeshGpu mesh;
  if (!device) {
    return mesh;
  }

  // Clockwise winding when viewed from outside (D3D11 default front face).
  static const MeshVertex kVerts[24] = {
      // +Z
      {-0.5f, 0.5f, 0.5f, 0.f, 0.f, 1.f, 0.f, 0.f},
      {0.5f, 0.5f, 0.5f, 0.f, 0.f, 1.f, 1.f, 0.f},
      {0.5f, -0.5f, 0.5f, 0.f, 0.f, 1.f, 1.f, 1.f},
      {-0.5f, -0.5f, 0.5f, 0.f, 0.f, 1.f, 0.f, 1.f},
      // -Z
      {0.5f, 0.5f, -0.5f, 0.f, 0.f, -1.f, 0.f, 0.f},
      {-0.5f, 0.5f, -0.5f, 0.f, 0.f, -1.f, 1.f, 0.f},
      {-0.5f, -0.5f, -0.5f, 0.f, 0.f, -1.f, 1.f, 1.f},
      {0.5f, -0.5f, -0.5f, 0.f, 0.f, -1.f, 0.f, 1.f},
      // +X
      {0.5f, 0.5f, 0.5f, 1.f, 0.f, 0.f, 0.f, 0.f},
      {0.5f, 0.5f, -0.5f, 1.f, 0.f, 0.f, 1.f, 0.f},
      {0.5f, -0.5f, -0.5f, 1.f, 0.f, 0.f, 1.f, 1.f},
      {0.5f, -0.5f, 0.5f, 1.f, 0.f, 0.f, 0.f, 1.f},
      // -X
      {-0.5f, 0.5f, -0.5f, -1.f, 0.f, 0.f, 0.f, 0.f},
      {-0.5f, 0.5f, 0.5f, -1.f, 0.f, 0.f, 1.f, 0.f},
      {-0.5f, -0.5f, 0.5f, -1.f, 0.f, 0.f, 1.f, 1.f},
      {-0.5f, -0.5f, -0.5f, -1.f, 0.f, 0.f, 0.f, 1.f},
      // +Y
      {-0.5f, 0.5f, -0.5f, 0.f, 1.f, 0.f, 0.f, 0.f},
      {0.5f, 0.5f, -0.5f, 0.f, 1.f, 0.f, 1.f, 0.f},
      {0.5f, 0.5f, 0.5f, 0.f, 1.f, 0.f, 1.f, 1.f},
      {-0.5f, 0.5f, 0.5f, 0.f, 1.f, 0.f, 0.f, 1.f},
      // -Y
      {-0.5f, -0.5f, 0.5f, 0.f, -1.f, 0.f, 0.f, 0.f},
      {0.5f, -0.5f, 0.5f, 0.f, -1.f, 0.f, 1.f, 0.f},
      {0.5f, -0.5f, -0.5f, 0.f, -1.f, 0.f, 1.f, 1.f},
      {-0.5f, -0.5f, -0.5f, 0.f, -1.f, 0.f, 0.f, 1.f},
  };
  static const UINT kIdx[36] = {
      0,  1,  2,  0,  2,  3,  4,  5,  6,  4,  6,  7,
      8,  9,  10, 8,  10, 11, 12, 13, 14, 12, 14, 15,
      16, 17, 18, 16, 18, 19, 20, 21, 22, 20, 22, 23,
  };

  D3D11_BUFFER_DESC vbd = {};
  vbd.ByteWidth = sizeof(kVerts);
  vbd.Usage = D3D11_USAGE_DEFAULT;
  vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  D3D11_SUBRESOURCE_DATA vrd = {};
  vrd.pSysMem = kVerts;
  if (FAILED(device->CreateBuffer(&vbd, &vrd, &mesh.m_vb))) {
    return MeshGpu();
  }

  D3D11_BUFFER_DESC ibd = {};
  ibd.ByteWidth = sizeof(kIdx);
  ibd.Usage = D3D11_USAGE_DEFAULT;
  ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
  D3D11_SUBRESOURCE_DATA ird = {};
  ird.pSysMem = kIdx;
  if (FAILED(device->CreateBuffer(&ibd, &ird, &mesh.m_ib))) {
    return MeshGpu();
  }

  mesh.m_indexCount = 36;
  return mesh;
}

MeshGpu MeshGpu::createSphere(ID3D11Device*) {
  return MeshGpu();
}

MeshGpu MeshGpu::createCylinder(ID3D11Device*) {
  return MeshGpu();
}

void MeshGpu::draw(ID3D11DeviceContext* context) const {
  if (!context || !valid()) {
    return;
  }
  UINT stride = sizeof(MeshVertex);
  UINT offset = 0;
  ID3D11Buffer* vb = m_vb.Get();
  context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
  context->IASetIndexBuffer(m_ib.Get(), DXGI_FORMAT_R32_UINT, 0);
  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  context->DrawIndexed(m_indexCount, 0, 0);
}
