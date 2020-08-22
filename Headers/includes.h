#pragma once
#include <iostream>
#include <Windows.h>
#include <windowsx.h>
#include <thread>
#include <chrono>
#include <map>
#include <unordered_map>
#include <string>
#include <random>
#include <math.h>
#include <fstream>
#include <exception>
#include <limits>

#include <d3d11_1.h>
#include <dxgi1_2.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <d3dcompiler.h>
#include <DDSTextureLoader.h>
#include <fbxsdk.h>
#include <ppl.h>

using namespace DirectX;

#define internal static

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
const XMMATRIX MATRIX_IDENTITY = XMMatrixIdentity();

namespace Window
{
  enum
  {
    WIDTH = 2560,
    HEIGHT = 1440
  };
}

