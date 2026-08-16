#include "teach/MeshBuild.h"
#include <cmath>

namespace {

const float kPi = 3.14159265358979323846f;

void pushVert(std::vector<MeshVertex>* v,
              float px, float py, float pz,
              float nx, float ny, float nz,
              float u, float vv) {
  MeshVertex m;
  m.px = px;
  m.py = py;
  m.pz = pz;
  m.nx = nx;
  m.ny = ny;
  m.nz = nz;
  m.u = u;
  m.v = vv;
  v->push_back(m);
}

}  // namespace

void buildCube(std::vector<MeshVertex>* v, std::vector<unsigned short>* i) {
  if (!v || !i) {
    return;
  }
  v->clear();
  i->clear();

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
  static const unsigned short kIdx[36] = {
      0,  1,  2,  0,  2,  3,  4,  5,  6,  4,  6,  7,
      8,  9,  10, 8,  10, 11, 12, 13, 14, 12, 14, 15,
      16, 17, 18, 16, 18, 19, 20, 21, 22, 20, 22, 23,
  };
  v->insert(v->end(), kVerts, kVerts + 24);
  i->insert(i->end(), kIdx, kIdx + 36);
}

void buildSphere(std::vector<MeshVertex>* v, std::vector<unsigned short>* i, int slices, int stacks) {
  if (!v || !i) {
    return;
  }
  v->clear();
  i->clear();
  if (slices < 3) {
    slices = 3;
  }
  if (stacks < 2) {
    stacks = 2;
  }

  const float r = 0.5f;
  for (int st = 0; st <= stacks; ++st) {
    const float vv = static_cast<float>(st) / static_cast<float>(stacks);
    const float theta = vv * kPi;
    const float y = r * cosf(theta);
    const float ringR = r * sinf(theta);
    for (int sl = 0; sl <= slices; ++sl) {
      const float u = static_cast<float>(sl) / static_cast<float>(slices);
      const float phi = u * (2.f * kPi);
      // phi=0 at +Z, increasing toward +X so u grows rightward on the +Z face.
      const float x = ringR * sinf(phi);
      const float z = ringR * cosf(phi);
      pushVert(v, x, y, z, x / r, y / r, z / r, u, vv);
    }
  }

  const int stride = slices + 1;
  for (int st = 0; st < stacks; ++st) {
    for (int sl = 0; sl < slices; ++sl) {
      const unsigned short i00 = static_cast<unsigned short>(st * stride + sl);
      const unsigned short i01 = static_cast<unsigned short>(i00 + 1);
      const unsigned short i10 = static_cast<unsigned short>((st + 1) * stride + sl);
      const unsigned short i11 = static_cast<unsigned short>(i10 + 1);
      i->push_back(i00);
      i->push_back(i01);
      i->push_back(i11);
      i->push_back(i00);
      i->push_back(i11);
      i->push_back(i10);
    }
  }
}

void buildCylinder(std::vector<MeshVertex>* v, std::vector<unsigned short>* i, int slices) {
  if (!v || !i) {
    return;
  }
  v->clear();
  i->clear();
  if (slices < 3) {
    slices = 3;
  }

  const float r = 0.5f;
  const float yTop = 0.5f;
  const float yBot = -0.5f;

  for (int sl = 0; sl <= slices; ++sl) {
    const float u = static_cast<float>(sl) / static_cast<float>(slices);
    const float phi = u * (2.f * kPi);
    const float x = r * sinf(phi);
    const float z = r * cosf(phi);
    const float nx = x / r;
    const float nz = z / r;
    pushVert(v, x, yTop, z, nx, 0.f, nz, u, 0.f);
    pushVert(v, x, yBot, z, nx, 0.f, nz, u, 1.f);
  }

  for (int sl = 0; sl < slices; ++sl) {
    const unsigned short t0 = static_cast<unsigned short>(sl * 2);
    const unsigned short b0 = static_cast<unsigned short>(t0 + 1);
    const unsigned short t1 = static_cast<unsigned short>((sl + 1) * 2);
    const unsigned short b1 = static_cast<unsigned short>(t1 + 1);
    i->push_back(t0);
    i->push_back(t1);
    i->push_back(b1);
    i->push_back(t0);
    i->push_back(b1);
    i->push_back(b0);
  }

  const unsigned short topCenter = static_cast<unsigned short>(v->size());
  pushVert(v, 0.f, yTop, 0.f, 0.f, 1.f, 0.f, 0.5f, 0.5f);
  const unsigned short topRim = static_cast<unsigned short>(v->size());
  for (int sl = 0; sl <= slices; ++sl) {
    const float u = static_cast<float>(sl) / static_cast<float>(slices);
    const float phi = u * (2.f * kPi);
    const float x = r * sinf(phi);
    const float z = r * cosf(phi);
    pushVert(v, x, yTop, z, 0.f, 1.f, 0.f, 0.5f + 0.5f * x / r, 0.5f + 0.5f * z / r);
  }
  for (int sl = 0; sl < slices; ++sl) {
    i->push_back(topCenter);
    i->push_back(static_cast<unsigned short>(topRim + sl));
    i->push_back(static_cast<unsigned short>(topRim + sl + 1));
  }

  const unsigned short botCenter = static_cast<unsigned short>(v->size());
  pushVert(v, 0.f, yBot, 0.f, 0.f, -1.f, 0.f, 0.5f, 0.5f);
  const unsigned short botRim = static_cast<unsigned short>(v->size());
  for (int sl = 0; sl <= slices; ++sl) {
    const float u = static_cast<float>(sl) / static_cast<float>(slices);
    const float phi = u * (2.f * kPi);
    const float x = r * sinf(phi);
    const float z = r * cosf(phi);
    pushVert(v, x, yBot, z, 0.f, -1.f, 0.f, 0.5f + 0.5f * x / r, 0.5f + 0.5f * z / r);
  }
  for (int sl = 0; sl < slices; ++sl) {
    i->push_back(botCenter);
    i->push_back(static_cast<unsigned short>(botRim + sl + 1));
    i->push_back(static_cast<unsigned short>(botRim + sl));
  }
}

int cubeVertexCount() {
  std::vector<MeshVertex> v;
  std::vector<unsigned short> i;
  buildCube(&v, &i);
  return static_cast<int>(v.size());
}
