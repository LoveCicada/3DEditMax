#pragma once
#include <vector>

struct MeshVertex {
  float px, py, pz;
  float nx, ny, nz;
  float u, v;
};

struct MeshEdge {
  float ax, ay, az;
  float bx, by, bz;
};

void buildCube(std::vector<MeshVertex>* v, std::vector<unsigned short>* i);
void buildSphere(std::vector<MeshVertex>* v, std::vector<unsigned short>* i, int slices, int stacks);
void buildCylinder(std::vector<MeshVertex>* v, std::vector<unsigned short>* i, int slices);
void buildCone(std::vector<MeshVertex>* v, std::vector<unsigned short>* i,
               float radius, float height, int slices);
void buildSilhouetteEdges(const std::vector<MeshVertex>& verts,
                          const std::vector<unsigned short>& indices,
                          std::vector<MeshEdge>* edges,
                          float thresholdDeg);
int cubeVertexCount();
