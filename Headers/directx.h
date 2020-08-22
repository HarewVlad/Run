#pragma once
#include "includes.h"

struct Directx {
  Directx();
  void initDirectx();

  LRESULT mainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

  virtual void onMouseDown(WPARAM btnState, int x, int y) = 0;
  virtual void onMouseUp(WPARAM btnState, int x, int y) = 0;
  virtual void onMouseMove(WPARAM btnState, int x, int y) = 0;

  static Directx *dx;
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
  ID3D11RasterizerState *rastState;
  ID3D11DepthStencilState *depthStencilState;
};

LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
