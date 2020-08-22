#pragma once
#include "includes.h"
#include "vertex.h"
#include "directx.h"
#include "camera.h"
#include "constantBuffers.h"

struct MeshDynamic {
  ID3D11Buffer *vertexBuffer;
  ID3D11Buffer *indexBuffer;
  std::vector<int> indices;
  std::vector<VertexDynamic> vertices;

  ~MeshDynamic() {
    vertexBuffer->Release();
    indexBuffer->Release();
  }
};

struct MeshStatic {
  ID3D11Buffer *vertexBuffer;
  ID3D11Buffer *indexBuffer;
  std::vector<int> indices;
  std::vector<VertexStatic> vertices;

  ~MeshStatic() {
    vertexBuffer->Release();
    indexBuffer->Release();
  }
};

struct GeometryGenerator {
  MeshStatic *createPlaneFromHeightmap(ID3D11Device *device, const std::string &filename);
  MeshStatic *createPlaneDefault(ID3D11Device *device, XMFLOAT3 pos, float width, float depth, UINT m, UINT n);
  MeshStatic *createBoxDefault(ID3D11Device *device, int w, int h, int d);
  MeshStatic *createSphereDefault(ID3D11Device *device, int latLines, int longLines);
};