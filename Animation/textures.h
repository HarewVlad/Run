#pragma once
#include "includes.h"

namespace Textures {
  ID3D11ShaderResourceView *createTextureFromFile(ID3D11Device *device, const std::string &name, unsigned int flags);
}