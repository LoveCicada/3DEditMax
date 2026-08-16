#include "render/MeshGpu.h"

static_assert(sizeof(MeshVertex) == 32, "MeshVertex is 32 bytes");

MeshGpu MeshGpu::createFromCpu(ID3D11Device* device,
                               const std::vector<MeshVertex>& verts,
                               const std::vector<unsigned short>& indices) {
  MeshGpu mesh;
  if (!device || verts.empty() || indices.empty()) {
    return mesh;
  }

  D3D11_BUFFER_DESC vbd = {};
  vbd.ByteWidth = static_cast<UINT>(verts.size() * sizeof(MeshVertex));
  vbd.Usage = D3D11_USAGE_DEFAULT;
  vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  D3D11_SUBRESOURCE_DATA vrd = {};
  vrd.pSysMem = &verts[0];
  if (FAILED(device->CreateBuffer(&vbd, &vrd, &mesh.m_vb))) {
    return MeshGpu();
  }

  D3D11_BUFFER_DESC ibd = {};
  ibd.ByteWidth = static_cast<UINT>(indices.size() * sizeof(unsigned short));
  ibd.Usage = D3D11_USAGE_DEFAULT;
  ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
  D3D11_SUBRESOURCE_DATA ird = {};
  ird.pSysMem = &indices[0];
  if (FAILED(device->CreateBuffer(&ibd, &ird, &mesh.m_ib))) {
    return MeshGpu();
  }

  mesh.m_indexCount = static_cast<UINT>(indices.size());
  return mesh;
}

MeshGpu MeshGpu::createCube(ID3D11Device* device) {
  std::vector<MeshVertex> verts;
  std::vector<unsigned short> indices;
  buildCube(&verts, &indices);
  return createFromCpu(device, verts, indices);
}

MeshGpu MeshGpu::createSphere(ID3D11Device* device, int slices, int stacks) {
  std::vector<MeshVertex> verts;
  std::vector<unsigned short> indices;
  buildSphere(&verts, &indices, slices, stacks);
  return createFromCpu(device, verts, indices);
}

MeshGpu MeshGpu::createCylinder(ID3D11Device* device, int slices) {
  std::vector<MeshVertex> verts;
  std::vector<unsigned short> indices;
  buildCylinder(&verts, &indices, slices);
  return createFromCpu(device, verts, indices);
}

void MeshGpu::draw(ID3D11DeviceContext* context) const {
  if (!context || !valid()) {
    return;
  }
  UINT stride = sizeof(MeshVertex);
  UINT offset = 0;
  ID3D11Buffer* vb = m_vb.Get();
  context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
  context->IASetIndexBuffer(m_ib.Get(), DXGI_FORMAT_R16_UINT, 0);
  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  context->DrawIndexed(m_indexCount, 0, 0);
}
