#pragma once
#include "includes.h"

struct VertexShader {
  ID3D11VertexShader *vs;
  ID3D10Blob *blob;
};

struct PixelShader {
  ID3D11PixelShader *ps;
  ID3D10Blob *blob;
};

struct ConstantBuffer {
  XMMATRIX world;
  XMMATRIX view;
  XMMATRIX proj;
  XMMATRIX worldViewProj;
  XMFLOAT3 eye;
  float pad1;
  XMMATRIX boneTransforms[96];
};

struct Directx {
  // Main data
  HINSTANCE appInstance;
  HWND mainWindow;
  D3D_DRIVER_TYPE driver_type = D3D_DRIVER_TYPE_NULL;
  D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_11_0;
  ID3D11Device *device;
  ID3D11Device1 *device1;
  ID3D11DeviceContext *deviceContext;
  IDXGISwapChain *swapChain;
  IDXGISwapChain1 *swapChain1;
  ID3D11DeviceContext1 *deviceContext1;
  ID3D11Texture2D *backBuffer;
  ID3D11RenderTargetView *renderTargetView;
  ID3D11Texture2D *depthStencil;
  ID3D11DepthStencilView *depthStencilView;
  ID3D11SamplerState *sampler;

  // Shaders
  std::unordered_map<LPCWSTR, VertexShader> vertexShaders;
  std::unordered_map<LPCWSTR, PixelShader> pixelShaders;

  // Input layouts
  std::unordered_map<std::string, ID3D11InputLayout *> layouts;

  // Buffers
  std::unordered_map<std::string, ID3D11Buffer *> buffers;

  // Textures
  std::unordered_map<std::string, ID3D11ShaderResourceView *> textures;

  void initWindow();
  void initDirectx();

  void createVertexShader(LPCWSTR shaderName, LPCSTR shaderMainFunc);
  void createPixelShader(LPCWSTR shaderName, LPCSTR shaderMainFunc);
  void createInputLayout(const std::string &layoutName, D3D11_INPUT_ELEMENT_DESC vertexDesc[], UINT numElements, VertexShader *vertexShader);
  void createBuffer(const std::string &name, const ConstantBuffer &cb);
  void createTexture(const std::string &name, unsigned int miscFlags);
};