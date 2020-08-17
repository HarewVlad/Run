#pragma once
#include "includes.h"

struct ConstantBufferPlayer {
  XMMATRIX world;
  XMMATRIX view;
  XMMATRIX proj;
  XMMATRIX worldViewProj;
  XMMATRIX rotation;
  XMFLOAT3 eye;
  float pad1;
  XMMATRIX boneTransforms[256];
};

struct ConstantBufferDefault {
  XMMATRIX world;
  XMMATRIX view;
  XMMATRIX proj;
  XMMATRIX worldViewProj;
  XMFLOAT3 eye;
  float pad1;
};

struct ConstantBufferLightning {
  XMFLOAT3 dir;
  float pad1;
  XMFLOAT4 ambient;
  XMFLOAT4 diffuse;
};

namespace ConstantBuffer {
  ID3D11Buffer *createBuffer(ID3D11Device *device, const void *b, size_t size);
}