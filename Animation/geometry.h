#pragma once
#include "includes.h"
#include "vertex.h"
#include "directx.h"
#include "camera.h"

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

struct Object {
  MeshStatic *mesh;
  XMMATRIX world;
};

struct GeometryConstructor {
  MeshStatic *createPlane(ID3D11Device *device, const std::string &name);
  MeshStatic *createBox(ID3D11Device *device, float w, float h, float d);
  MeshStatic *createSphere(ID3D11Device *device, int sliceCount, int stackCount);
};