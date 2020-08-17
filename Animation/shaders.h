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

namespace Shaders {
  PixelShader *createPixelShaderDefault(ID3D11Device *device, LPCWSTR shaderName, LPCSTR shaderMainFunc);
  VertexShader *createVertexShaderDefault(ID3D11Device *device, LPCWSTR shaderName, LPCSTR shaderMainFunc);

  ID3D11InputLayout *createInputLayout(ID3D11Device *device, D3D11_INPUT_ELEMENT_DESC vertexDesc[], UINT numElements, VertexShader *vertexShader);
}