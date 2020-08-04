#pragma once
#include <iostream>
#include <Windows.h>
#include <thread>
#include <chrono>
#include <map>
#include <unordered_map>
#include <string>
#include <random>
#include <math.h>

#include <d3d11_1.h>
#include <dxgi1_2.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <d3dcompiler.h>
#include <DDSTextureLoader.h>
#include <fbxsdk.h>

// #define MANUAL_CONTROL
#define AUTO_CONTROL

using namespace DirectX;

namespace DX
{
  inline void ThrowIfFailed(HRESULT hr)
  {
    if (FAILED(hr))
    {
      // Set a breakpoint on this line to catch DirectX API errors
      throw std::exception();
    }
  }
}

const float SPEED = 0.5f;

namespace Window
{
  enum
  {
    WIDTH = 2560,
    HEIGHT = 1440
  };
}

struct Vertex
{
  XMFLOAT3 pos;
  XMFLOAT3 normal;
  XMFLOAT2 tex;
  XMFLOAT3 boneWeights;
  BYTE boneIndices[4];

  Vertex() {};
  Vertex(float px, float py, float pz, float nx, float ny, float nz, float u, float v)
  {
    pos = { px, py, pz };
    normal = { nx, ny, nz };
    tex = { u, v };
  }
};