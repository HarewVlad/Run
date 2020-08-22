#pragma once
#include "includes.h"

// TODO(vlad): mb do just one type of vertices for all things

struct VertexDynamic
{
  VertexDynamic() {};
  VertexDynamic(float px, float py, float pz, float nx, float ny, float nz, float u, float v)
  {
    pos = { px, py, pz };
    normal = { nx, ny, nz };
    tex = { u, v };
  }

  XMFLOAT3 pos;
  XMFLOAT3 normal;
  XMFLOAT2 tex;
  XMFLOAT3 boneWeights;
  BYTE boneIndices[4];
};

struct VertexStatic {
  VertexStatic() {};
  VertexStatic(float px, float py, float pz, float nx, float ny, float nz, float u, float v)
  {
    pos = { px, py, pz };
    normal = { nx, ny, nz };
    tex = { u, v };
  }

  XMFLOAT3 pos;
  XMFLOAT3 normal;
  XMFLOAT2 tex;
};