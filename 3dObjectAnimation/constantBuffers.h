#pragma once
#include "includes.h"

struct ConstantBuffer {
  XMMATRIX world;
  XMMATRIX view;
  XMMATRIX proj;
  XMMATRIX worldViewProj;
  XMFLOAT3 eye;
  float pad1;
  // XMMATRIX boneTransforms[96];
};