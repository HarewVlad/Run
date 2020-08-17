#include "textures.h"

namespace Textures {
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