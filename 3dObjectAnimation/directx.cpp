#pragma once
#include "directx.h"

LRESULT wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
  {
  case WM_CLOSE:
    PostQuitMessage(0);
    break;
  }
  return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK mainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  return wndProc(hwnd, msg, wParam, lParam);
}

void Directx::initWindow() {
  // Register class
  WNDCLASS wc;
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = mainWndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = appInstance;
  wc.hIcon = LoadIcon(0, IDI_APPLICATION);
  wc.hCursor = LoadCursor(0, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wc.lpszMenuName = 0;
  wc.lpszClassName = "Main";

  if (!RegisterClass(&wc)) {
    OutputDebugString("cant register class");
    exit(EXIT_FAILURE);
  }

  // Create window
  RECT rc = { 0, 0, Window::WIDTH, Window::HEIGHT };
  AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);
  int width = rc.right - rc.left;
  int height = rc.bottom - rc.top;
  mainWindow = CreateWindow(
    "Main",
    "Main",
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT,
    CW_USEDEFAULT,
    width,
    height,
    0,
    0,
    appInstance,
    0);

  if (!mainWindow)
  {
    __debugbreak();
  }

  ShowWindow(mainWindow, SW_SHOW);
  UpdateWindow(mainWindow);
}

void Directx::initDirectx() {
  {
    D3D_DRIVER_TYPE driverTypes[] =
    {
      D3D_DRIVER_TYPE_HARDWARE,
      D3D_DRIVER_TYPE_WARP,
      D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
      D3D_FEATURE_LEVEL_11_1,
      D3D_FEATURE_LEVEL_11_0,
      D3D_FEATURE_LEVEL_10_1,
      D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    HRESULT hr;
    // Device
    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
      driver_type = driverTypes[driverTypeIndex];
      hr = D3D11CreateDevice(nullptr, driver_type, nullptr, D3D11_CREATE_DEVICE_DEBUG, featureLevels, numFeatureLevels,
        D3D11_SDK_VERSION, &device, &feature_level, &deviceContext);

      if (hr == E_INVALIDARG)
      {
        // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
        hr = D3D11CreateDevice(nullptr, driver_type, nullptr, 0, &featureLevels[1], numFeatureLevels - 1,
          D3D11_SDK_VERSION, &device, &feature_level, &deviceContext);
      }

      if (SUCCEEDED(hr))
        break;
    }
    DX::ThrowIfFailed(hr);

    // Factory
    IDXGIFactory1* dxgiFactory = nullptr;
    {
      IDXGIDevice* dxgiDevice = nullptr;
      hr = device->QueryInterface(IID_PPV_ARGS(&dxgiDevice));
      if (SUCCEEDED(hr))
      {
        IDXGIAdapter* adapter = nullptr;
        hr = dxgiDevice->GetAdapter(&adapter);
        if (SUCCEEDED(hr))
        {
          hr = adapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
          adapter->Release();
        }
        dxgiDevice->Release();
      }
    }
    DX::ThrowIfFailed(hr);

    // Swap chain
    IDXGIFactory2 *dxgiFactory2 = nullptr;
    hr = dxgiFactory->QueryInterface(IID_PPV_ARGS(&dxgiFactory2));
    if (dxgiFactory2)
    {
      // DirectX 11.1 or later
      hr = device->QueryInterface(IID_PPV_ARGS(&device1));
      if (SUCCEEDED(hr))
      {
        (void)deviceContext->QueryInterface(IID_PPV_ARGS(&deviceContext1));
      }

      DXGI_SWAP_CHAIN_DESC1 sd; // TODO: MAKE {}
      ZeroMemory(&sd, sizeof(sd));
      sd.Width = Window::WIDTH;
      sd.Height = Window::HEIGHT;
      sd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
      sd.SampleDesc.Count = 1;
      sd.SampleDesc.Quality = 0;
      sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
      sd.BufferCount = 1;

      hr = dxgiFactory2->CreateSwapChainForHwnd(device, mainWindow, &sd, nullptr, nullptr, &swapChain1);
      if (SUCCEEDED(hr))
      {
        hr = swapChain1->QueryInterface(IID_PPV_ARGS(&swapChain));
      }

      dxgiFactory2->Release();
    }
    else
    {
      // DirectX 11.0 systems
      DXGI_SWAP_CHAIN_DESC sd;
      ZeroMemory(&sd, sizeof(sd));
      sd.BufferCount = 1;
      sd.BufferDesc.Width = Window::WIDTH;
      sd.BufferDesc.Height = Window::HEIGHT;
      sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
      sd.BufferDesc.RefreshRate.Numerator = 60;
      sd.BufferDesc.RefreshRate.Denominator = 1;
      sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
      sd.OutputWindow = mainWindow;
      sd.SampleDesc.Count = 1;
      sd.SampleDesc.Quality = 0;
      sd.Windowed = TRUE;

      hr = dxgiFactory->CreateSwapChain(device, &sd, &swapChain);
    }

    dxgiFactory->MakeWindowAssociation(mainWindow, DXGI_MWA_NO_ALT_ENTER);
    dxgiFactory->Release();

    DX::ThrowIfFailed(hr);

    // Viewport
    D3D11_VIEWPORT vp;
    vp.Width = (float)Window::WIDTH;
    vp.Height = (float)Window::HEIGHT;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    deviceContext->RSSetViewports(1, &vp);
  }

  // Render target view
  DX::ThrowIfFailed(swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)));
  DX::ThrowIfFailed(device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView));

  // Depth stencil view
  D3D11_TEXTURE2D_DESC desc_depth = {};
  desc_depth.Width = Window::WIDTH;
  desc_depth.Height = Window::HEIGHT;
  desc_depth.MipLevels = 1;
  desc_depth.ArraySize = 1;
  desc_depth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  desc_depth.SampleDesc.Count = 1;
  desc_depth.SampleDesc.Quality = 0;
  desc_depth.Usage = D3D11_USAGE_DEFAULT;
  desc_depth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  desc_depth.CPUAccessFlags = 0;
  desc_depth.MiscFlags = 0;

  DX::ThrowIfFailed(device->CreateTexture2D(&desc_depth, nullptr, &depthStencil));
  DX::ThrowIfFailed(device->CreateDepthStencilView(depthStencil, nullptr, &depthStencilView));

  // Sampler state
  D3D11_SAMPLER_DESC sd = {};
  sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
  sd.MinLOD = 0;
  sd.MaxLOD = D3D11_FLOAT32_MAX;
  DX::ThrowIfFailed(device->CreateSamplerState(&sd, &sampler));
}

void Directx::createVertexShader(LPCWSTR shaderName, LPCSTR shaderMainFunc) {
  ID3D10Blob *compilationMsgs = nullptr;
  ID3D11VertexShader *vertexShader = nullptr;
  ID3D10Blob *shaderBlob = nullptr;

  DX::ThrowIfFailed(D3DCompileFromFile(shaderName, 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, shaderMainFunc, "vs_5_0", D3DCOMPILE_DEBUG, 0, &shaderBlob, &compilationMsgs));
  if (compilationMsgs != 0)
  {
    MessageBoxA(0, (char*)compilationMsgs->GetBufferPointer(), 0, 0);
  }
  DX::ThrowIfFailed(device->CreateVertexShader((shaderBlob)->GetBufferPointer(), (shaderBlob)->GetBufferSize(), NULL, &vertexShader));

  vertexShaders[shaderName] = VertexShader{ vertexShader, shaderBlob };
}

void Directx::createPixelShader(LPCWSTR shaderName, LPCSTR shaderMainFunc) {
  ID3D10Blob *compilationMsgs = nullptr;
  ID3D11PixelShader *pixelShader = nullptr;
  ID3D10Blob *shaderBlob = nullptr;

  DX::ThrowIfFailed(D3DCompileFromFile(shaderName, 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, shaderMainFunc, "ps_5_0", D3DCOMPILE_DEBUG, 0, &shaderBlob, &compilationMsgs));
  if (compilationMsgs != 0)
  {
    MessageBoxA(0, (char*)compilationMsgs->GetBufferPointer(), 0, 0);
  }
  DX::ThrowIfFailed(device->CreatePixelShader((shaderBlob)->GetBufferPointer(), (shaderBlob)->GetBufferSize(), NULL, &pixelShader));

  pixelShaders[shaderName] = PixelShader{ pixelShader, shaderBlob };
}

void Directx::createInputLayout(const std::string &layoutName, D3D11_INPUT_ELEMENT_DESC vertexDesc[], UINT numElements, VertexShader *vertexShader) {
  ID3D11InputLayout *inputLayout = nullptr;

  ID3D10Blob *shader_blob = vertexShader->blob;
  DX::ThrowIfFailed(device->CreateInputLayout(
    vertexDesc,
    numElements,
    shader_blob->GetBufferPointer(), shader_blob->GetBufferSize(),
    &inputLayout
  ));

  layouts[layoutName] = inputLayout;
}

void Directx::createBuffer(const std::string &name, const ConstantBuffer &cb) {
  ID3D11Buffer *buffer = nullptr;

  D3D11_BUFFER_DESC bd = {};
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  bd.ByteWidth = sizeof(ConstantBuffer);
  bd.CPUAccessFlags = 0;

  D3D11_SUBRESOURCE_DATA initData = {};
  initData.pSysMem = &cb;
  DX::ThrowIfFailed(device->CreateBuffer(&bd, &initData, &buffer));

  buffers[name] = buffer;
}

void Directx::createTexture(const std::string &name, unsigned int miscFlags) {
  ID3D11ShaderResourceView *texture = nullptr;

  DX::ThrowIfFailed(CreateDDSTextureFromFileEx(device,
    std::wstring(name.begin(), name.end()).c_str(),
    0, D3D11_USAGE_DEFAULT,
    D3D11_BIND_SHADER_RESOURCE,
    0,
    miscFlags,
    0,
    nullptr,
    &texture));

  textures[name] = texture;
}