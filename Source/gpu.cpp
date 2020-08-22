#include "gpu.h"

namespace GPU {
  // Shaders
  PixelShader *createPixelShaderDefault(ID3D11Device *device, LPCWSTR shaderName, LPCSTR shaderMainFunc) {
    ID3D10Blob *compilationMsgs = nullptr;
    ID3D11PixelShader *pixelShader = nullptr;
    ID3D10Blob *shaderBlob = nullptr;

    DX::ThrowIfFailed(D3DCompileFromFile(shaderName, 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, shaderMainFunc, "ps_5_0", D3DCOMPILE_DEBUG, 0, &shaderBlob, &compilationMsgs));
    if (compilationMsgs != 0)
    {
      MessageBoxA(0, (char*)compilationMsgs->GetBufferPointer(), 0, 0);
    }
    DX::ThrowIfFailed(device->CreatePixelShader((shaderBlob)->GetBufferPointer(), (shaderBlob)->GetBufferSize(), NULL, &pixelShader));

    return new PixelShader{ pixelShader, shaderBlob };
  }

  VertexShader *createVertexShaderDefault(ID3D11Device *device, LPCWSTR shaderName, LPCSTR shaderMainFunc) {
    ID3D10Blob *compilationMsgs = nullptr;
    ID3D11VertexShader *vertexShader = nullptr;
    ID3D10Blob *shaderBlob = nullptr;

    DX::ThrowIfFailed(D3DCompileFromFile(shaderName, 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, shaderMainFunc, "vs_5_0", D3DCOMPILE_DEBUG, 0, &shaderBlob, &compilationMsgs));
    if (compilationMsgs != 0)
    {
      MessageBoxA(0, (char*)compilationMsgs->GetBufferPointer(), 0, 0);
    }
    DX::ThrowIfFailed(device->CreateVertexShader((shaderBlob)->GetBufferPointer(), (shaderBlob)->GetBufferSize(), NULL, &vertexShader));

    return new VertexShader{ vertexShader, shaderBlob };
  }
  
  // Input layout
  ID3D11InputLayout *createInputLayout(ID3D11Device *device, D3D11_INPUT_ELEMENT_DESC vertexDesc[], UINT numElements, VertexShader *vertexShader) {
    ID3D11InputLayout *inputLayout = nullptr;

    ID3D10Blob *shader_blob = vertexShader->blob;
    DX::ThrowIfFailed(device->CreateInputLayout(
      vertexDesc,
      numElements,
      shader_blob->GetBufferPointer(), shader_blob->GetBufferSize(),
      &inputLayout
    ));

    return inputLayout;
  }

  // Textures
  ID3D11ShaderResourceView *createTextureFromFile(ID3D11Device *device, const std::string &name, unsigned int flags) {
    ID3D11ShaderResourceView *texture = nullptr;

    DX::ThrowIfFailed(CreateDDSTextureFromFileEx(device,
      std::wstring(name.begin(), name.end()).c_str(),
      0, D3D11_USAGE_DEFAULT,
      D3D11_BIND_SHADER_RESOURCE,
      0,
      flags,
      0,
      nullptr,
      &texture));

    return texture;
  }
}