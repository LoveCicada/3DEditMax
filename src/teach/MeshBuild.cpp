#include "teach/MeshBuild.h"
#include <cmath>
#include <map>
#include <vector>

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

void buildCone(std::vector<MeshVertex>* v, std::vector<unsigned short>* i,
               float radius, float height, int slices) {
  if (!v || !i) {
    return;
  }
  v->clear();
  i->clear();
  if (slices < 3) {
    slices = 3;
  }
  if (radius < 1e-4f) {
    radius = 1e-4f;
  }
  if (height < 1e-4f) {
    height = 1e-4f;
  }

  const float yApex = height * 0.5f;
  const float yBase = -height * 0.5f;
  const float slope = radius / height;
  pushVert(v, 0.f, yApex, 0.f, 0.f, 1.f, 0.f, 0.5f, 0.f);

  for (int sl = 0; sl < slices; ++sl) {
    const float u = static_cast<float>(sl) / static_cast<float>(slices);
    const float phi = u * (2.f * kPi);
    const float x = radius * sinf(phi);
    const float z = radius * cosf(phi);
    float nx = x;
    float ny = radius * slope;
    float nz = z;
    const float nlen = sqrtf(nx * nx + ny * ny + nz * nz);
    if (nlen > 1e-6f) {
      nx /= nlen;
      ny /= nlen;
      nz /= nlen;
    }
    pushVert(v, x, yBase, z, nx, ny, nz, u, 1.f);
  }

  for (int sl = 0; sl < slices; ++sl) {
    const unsigned short a = 0;
    const unsigned short b = static_cast<unsigned short>(1 + sl);
    const unsigned short c = static_cast<unsigned short>(1 + ((sl + 1) % slices));
    i->push_back(a);
    i->push_back(b);
    i->push_back(c);
  }

  const unsigned short baseCenter = static_cast<unsigned short>(v->size());
  pushVert(v, 0.f, yBase, 0.f, 0.f, -1.f, 0.f, 0.5f, 0.5f);
  const unsigned short baseRim = static_cast<unsigned short>(v->size());
  for (int sl = 0; sl <= slices; ++sl) {
    const float u = static_cast<float>(sl) / static_cast<float>(slices);
    const float phi = u * (2.f * kPi);
    const float x = radius * sinf(phi);
    const float z = radius * cosf(phi);
    pushVert(v, x, yBase, z, 0.f, -1.f, 0.f, 0.5f + 0.5f * x / radius, 0.5f + 0.5f * z / radius);
  }
  for (int sl = 0; sl < slices; ++sl) {
    i->push_back(baseCenter);
    i->push_back(static_cast<unsigned short>(baseRim + sl + 1));
    i->push_back(static_cast<unsigned short>(baseRim + sl));
  }
}

namespace {

struct QPos {
  int x;
  int y;
  int z;
  bool operator<(const QPos& o) const {
    if (x != o.x) {
      return x < o.x;
    }
    if (y != o.y) {
      return y < o.y;
    }
    return z < o.z;
  }
};

struct QEdge {
  QPos a;
  QPos b;
  bool operator<(const QEdge& o) const {
    if (a < o.a) {
      return true;
    }
    if (o.a < a) {
      return false;
    }
    return b < o.b;
  }
};

int quantizeAxis(float v) {
  return static_cast<int>(floorf(v * 4096.f + (v >= 0.f ? 0.5f : -0.5f)));
}

QPos makePos(float x, float y, float z) {
  QPos p;
  p.x = quantizeAxis(x);
  p.y = quantizeAxis(y);
  p.z = quantizeAxis(z);
  return p;
}

QEdge makeEdge(QPos a, QPos b) {
  QEdge e;
  if (b < a) {
    e.a = b;
    e.b = a;
  } else {
    e.a = a;
    e.b = b;
  }
  return e;
}

struct EdgeRec {
  float ax, ay, az;
  float bx, by, bz;
  int face0;
  int face1;
  int nfaces;
};

void addEdgeFace(std::map<QEdge, EdgeRec>* edges,
                 const MeshVertex& va,
                 const MeshVertex& vb,
                 int face) {
  const QEdge key = makeEdge(makePos(va.px, va.py, va.pz), makePos(vb.px, vb.py, vb.pz));
  std::map<QEdge, EdgeRec>::iterator it = edges->find(key);
  if (it == edges->end()) {
    EdgeRec rec;
    rec.ax = va.px;
    rec.ay = va.py;
    rec.az = va.pz;
    rec.bx = vb.px;
    rec.by = vb.py;
    rec.bz = vb.pz;
    rec.face0 = face;
    rec.face1 = -1;
    rec.nfaces = 1;
    (*edges)[key] = rec;
    return;
  }
  if (it->second.nfaces == 1) {
    it->second.face1 = face;
  }
  it->second.nfaces += 1;
}

void faceNormal(const MeshVertex& a, const MeshVertex& b, const MeshVertex& c,
                float* nx, float* ny, float* nz) {
  const float e1x = b.px - a.px;
  const float e1y = b.py - a.py;
  const float e1z = b.pz - a.pz;
  const float e2x = c.px - a.px;
  const float e2y = c.py - a.py;
  const float e2z = c.pz - a.pz;
  float x = e1y * e2z - e1z * e2y;
  float y = e1z * e2x - e1x * e2z;
  float z = e1x * e2y - e1y * e2x;
  const float len = sqrtf(x * x + y * y + z * z);
  if (len > 1e-8f) {
    x /= len;
    y /= len;
    z /= len;
  }
  *nx = x;
  *ny = y;
  *nz = z;
}

}  // namespace

void buildSilhouetteEdges(const std::vector<MeshVertex>& verts,
                          const std::vector<unsigned short>& indices,
                          std::vector<MeshEdge>* edges,
                          float thresholdDeg) {
  if (!edges) {
    return;
  }
  edges->clear();
  if (verts.empty() || indices.size() < 3) {
    return;
  }
  if (thresholdDeg < 0.f) {
    thresholdDeg = 0.f;
  }

  const size_t nfaces = indices.size() / 3;
  std::vector<float> nx(nfaces, 0.f);
  std::vector<float> ny(nfaces, 0.f);
  std::vector<float> nz(nfaces, 0.f);
  std::map<QEdge, EdgeRec> unique;

  for (size_t f = 0; f < nfaces; ++f) {
    const unsigned short i0 = indices[f * 3 + 0];
    const unsigned short i1 = indices[f * 3 + 1];
    const unsigned short i2 = indices[f * 3 + 2];
    if (i0 >= verts.size() || i1 >= verts.size() || i2 >= verts.size()) {
      continue;
    }
    faceNormal(verts[i0], verts[i1], verts[i2], &nx[f], &ny[f], &nz[f]);
    addEdgeFace(&unique, verts[i0], verts[i1], static_cast<int>(f));
    addEdgeFace(&unique, verts[i1], verts[i2], static_cast<int>(f));
    addEdgeFace(&unique, verts[i2], verts[i0], static_cast<int>(f));
  }

  const float threshRad = thresholdDeg * (kPi / 180.f);
  const float threshDot = cosf(threshRad);

  for (std::map<QEdge, EdgeRec>::const_iterator it = unique.begin(); it != unique.end(); ++it) {
    const EdgeRec& rec = it->second;
    bool keep = rec.nfaces == 1;
    if (rec.nfaces >= 2 && rec.face0 >= 0 && rec.face1 >= 0) {
      const float dot = nx[static_cast<size_t>(rec.face0)] * nx[static_cast<size_t>(rec.face1)] +
                        ny[static_cast<size_t>(rec.face0)] * ny[static_cast<size_t>(rec.face1)] +
                        nz[static_cast<size_t>(rec.face0)] * nz[static_cast<size_t>(rec.face1)];
      keep = dot <= threshDot;
    }
    if (!keep) {
      continue;
    }
    MeshEdge e;
    e.ax = rec.ax;
    e.ay = rec.ay;
    e.az = rec.az;
    e.bx = rec.bx;
    e.by = rec.by;
    e.bz = rec.bz;
    edges->push_back(e);
  }
}

int cubeVertexCount() {
  std::vector<MeshVertex> v;
  std::vector<unsigned short> i;
  buildCube(&v, &i);
  return static_cast<int>(v.size());
}
