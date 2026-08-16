#pragma once
#include <vector>

struct MeshVertex {
  float px, py, pz;
  float nx, ny, nz;
  float u, v;
};

void buildCube(std::vector<MeshVertex>* v, std::vector<unsigned short>* i);
void buildSphere(std::vector<MeshVertex>* v, std::vector<unsigned short>* i, int slices, int stacks);
void buildCylinder(std::vector<MeshVertex>* v, std::vector<unsigned short>* i, int slices);
int cubeVertexCount();
