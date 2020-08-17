#include "constantBuffers.h"

namespace ConstantBuffer {
  ID3D11Buffer *createBuffer(ID3D11Device *device, const void *b, size_t size) {
    ID3D11Buffer *buffer = nullptr;

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.ByteWidth = size;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = b;
    DX::ThrowIfFailed(device->CreateBuffer(&bd, &initData, &buffer));

    return buffer;
  }
}