#pragma once
#include "includes.h"

struct ConstantBuffer {
  XMMATRIX world;
  XMMATRIX view;
  XMMATRIX proj;
  XMMATRIX worldViewProj;
  XMMATRIX rotation;
  XMFLOAT3 eye;
  float pad1;
  XMMATRIX boneTransforms[96];
};