#pragma once
#include "geometry.h"

internal void computeAverageNormals(int start, int end, int numFaces,
  const std::vector<XMFLOAT3> &tempNormals,
  const std::vector<int> &indices,
  std::vector<VertexStatic> &vertices) {

  XMVECTOR sum = {};
  int facesUsing = 0;
  float tX, tY, tZ;

  for (int i = start; i < end; i++) {
    for (int j = 0; j < numFaces; j++) {
      if (indices[j * 3] == i
        || indices[j * 3 + 1] == i
        || indices[j * 3 + 2] == i) {
        tX = XMVectorGetX(sum) + tempNormals[j].x;
        tY = XMVectorGetY(sum) + tempNormals[j].y;
        tZ = XMVectorGetZ(sum) + tempNormals[j].z;

        sum = { tX, tY, tZ, 0 };
        facesUsing++;
      }
    }

    sum /= (float)facesUsing;
    sum = XMVector3Normalize(sum);

    vertices[i].normal.x = XMVectorGetX(sum);
    vertices[i].normal.y = XMVectorGetY(sum);
    vertices[i].normal.z = XMVectorGetZ(sum);

    sum = {};
    facesUsing = 0;
  }
}

MeshStatic *GeometryConstructor::createPlane(ID3D11Device *device, const std::string &name) {
  std::ifstream input(name, std::ifstream::binary);
  if (!input) {
    assert("unable to load height map");
  }

  BITMAPFILEHEADER bfh;
  BITMAPINFOHEADER bih;

  input.read(reinterpret_cast<char *>(&bfh), sizeof(BITMAPFILEHEADER));
  input.read(reinterpret_cast<char *>(&bih), sizeof(BITMAPINFOHEADER));

  int width = bih.biWidth;
  int height = bih.biHeight;

  size_t imageSize = width * height * 3;
  uint8_t *bitmapImage = new uint8_t[imageSize];

  input.seekg(bfh.bfOffBits);
  input.read(reinterpret_cast<char *>(bitmapImage), imageSize);

  input.close();

  XMFLOAT3 *data = nullptr;
  {
    data = new XMFLOAT3[width * height];
    int k = 0;
    float heightFactor = 10.0f;

    uint8_t bitmapHeight = 0;
    int index = 0;
    for (int j = 0; j < height; j++) {
      for (int i = 0; i < width; i++) {
        bitmapHeight = bitmapImage[k];

        index = height * j + i;

        data[index].x = (float)i;
        data[index].y = bitmapHeight / (float)heightFactor;
        data[index].z = (float)j;

        k += 3;
      }
    }

    delete[] bitmapImage;
    bitmapImage = nullptr;
  }

  int numVertices = width * height;
  int numFaces = (width - 1) * (height - 1) * 2;

  std::vector<VertexStatic> vertices(numVertices);
  std::vector<int> indices(numFaces * 3);
  // Filling vertices and indices
  {
    for (int i = 0; i < height; i++) {
      for (int j = 0; j < width; j++) {
        vertices[i * width + j].pos = data[i * width + j];
        vertices[i * width + j].normal = { 0, 1, 0 };
      }
    }

    int k = 0;
    int texUIndex = 0;
    int texVIndex = 0;
    for (int i = 0; i < height - 1; i++) {
      for (int j = 0; j < width - 1; j++) {
        indices[k + 5] = i * width + j; // CHANGED
        vertices[i * width + j].tex = { texUIndex + 0.0f, texVIndex + 1.0f };

        indices[k + 4] = i * width + j + 1; // CHANGED
        vertices[i * width + j + 1].tex = { texUIndex + 1.0f, texVIndex + 1.0f };

        indices[k + 3] = (i + 1) * width + j; // CHANGED
        vertices[(i + 1) * width + j].tex = { texUIndex + 0.0f, texVIndex + 0.0f };

        indices[k + 2] = (i + 1) * width + j; // CHANGED
        vertices[(i + 1) * width + j].tex = { texUIndex + 0.0f, texVIndex + 0.0f };

        indices[k + 1] = i * width + j + 1; // CHANGED
        vertices[i * width + j + 1].tex = { texUIndex + 1.0f, texVIndex + 1.0f };

        indices[k] = (i + 1) * width + j + 1; // CHANGED
        vertices[(i + 1) * width + j + 1].tex = { texUIndex + 1.0f, texVIndex + 0.0f };

        k += 6;
        texUIndex++;
      }
      texUIndex = 0;
      texVIndex++;
    }
  }

  // Normal averaging
  {
    std::vector<XMFLOAT3> tempNormals;

    XMFLOAT3 normal;
    for (int i = 0; i < numFaces; i++) {
      int indexEdge22 = indices[i * 3]; // CHANGAED UPWARD
      int indexEdge21 = indices[i * 3 + 2]; // CHANGAED UPWARD
      int indexEdge12 = indexEdge21; // CHANGAED UPWARD
      int indexEdge11 = indices[i * 3 + 1]; // CHANGED UPWARDS

      XMVECTOR edge1 = { vertices[indexEdge11].pos.x - vertices[indexEdge12].pos.x,
        vertices[indexEdge11].pos.y - vertices[indexEdge12].pos.y,
        vertices[indexEdge11].pos.z - vertices[indexEdge12].pos.z
      };

      XMVECTOR edge2 = { vertices[indexEdge21].pos.x - vertices[indexEdge22].pos.x,
        vertices[indexEdge21].pos.y - vertices[indexEdge22].pos.y,
        vertices[indexEdge21].pos.z - vertices[indexEdge22].pos.z
      };

      XMStoreFloat3(&normal, XMVector3Cross(edge1, edge2));
      tempNormals.push_back(normal);
    }

    int numThreads = std::thread::hardware_concurrency();
    int batchSize = numVertices / numThreads;
    int batchRemainder = numVertices % numThreads;

    std::vector<std::thread> threads(numThreads);

    for (int i = 0; i < numThreads; i++) {
      int start = i * batchSize;
      threads[i] = std::thread(computeAverageNormals, start, start + batchSize, numFaces, std::ref(tempNormals), std::ref(indices), std::ref(vertices));
    }

    // Remainder
    int start = numThreads * batchSize;
    computeAverageNormals(start, start + batchRemainder, numFaces, tempNormals, indices, vertices);

    // Wait for threads to finish
    std::for_each(threads.begin(), threads.end(), std::mem_fn(&std::thread::join));
  }

  ID3D11Buffer *vertexBuffer = nullptr;
  {
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = vertices.size() * sizeof(VertexStatic);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = &vertices[0];

    DX::ThrowIfFailed(device->CreateBuffer(&bd, &initData, &vertexBuffer));
  }

  ID3D11Buffer *indexBuffer = nullptr;
  {
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = indices.size() * sizeof(UINT);
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    bd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = &indices[0];

    DX::ThrowIfFailed(device->CreateBuffer(&bd, &initData, &indexBuffer));
  }

  return new MeshStatic{ vertexBuffer, indexBuffer, indices, vertices };
}

MeshStatic *GeometryConstructor::createBox(ID3D11Device *device, float w, float h, float d) {
  std::vector<VertexStatic> vertices(24);

  float w2 = 0.5f*w;
  float h2 = 0.5f*h;
  float d2 = 0.5f*d;

  // Fill in the front face vertex data.
  vertices[0] = VertexStatic(-w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
  vertices[1] = VertexStatic(-w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
  vertices[2] = VertexStatic(+w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
  vertices[3] = VertexStatic(+w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

  // Fill in the back face vertex data.
  vertices[4] = VertexStatic(-w2, -h2, +d2, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
  vertices[5] = VertexStatic(+w2, -h2, +d2, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
  vertices[6] = VertexStatic(+w2, +h2, +d2, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
  vertices[7] = VertexStatic(-w2, +h2, +d2, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);

  // Fill in the top face vertex data.
  vertices[8] = VertexStatic(-w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
  vertices[9] = VertexStatic(-w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
  vertices[10] = VertexStatic(+w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);
  vertices[11] = VertexStatic(+w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f);

  // Fill in the bottom face vertex data.
  vertices[12] = VertexStatic(-w2, -h2, -d2, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f);
  vertices[13] = VertexStatic(+w2, -h2, -d2, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f);
  vertices[14] = VertexStatic(+w2, -h2, +d2, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f);
  vertices[15] = VertexStatic(-w2, -h2, +d2, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f);

  // Fill in the left face vertex data.
  vertices[16] = VertexStatic(-w2, -h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
  vertices[17] = VertexStatic(-w2, +h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  vertices[18] = VertexStatic(-w2, +h2, -d2, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
  vertices[19] = VertexStatic(-w2, -h2, -d2, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

  // Fill in the right face vertex data.
  vertices[20] = VertexStatic(+w2, -h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
  vertices[21] = VertexStatic(+w2, +h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  vertices[22] = VertexStatic(+w2, +h2, +d2, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
  vertices[23] = VertexStatic(+w2, -h2, +d2, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

  // Vertex buffer
  ID3D11Buffer *vertexBuffer = nullptr;
  D3D11_BUFFER_DESC bd = {};
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.ByteWidth = vertices.size() * sizeof(VertexStatic);
  bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  bd.CPUAccessFlags = 0;
  bd.MiscFlags = 0;

  D3D11_SUBRESOURCE_DATA initData = {};
  initData.pSysMem = &vertices[0];

  DX::ThrowIfFailed(device->CreateBuffer(&bd, &initData, &vertexBuffer));

  std::vector<int> indices(36);

  // Fill in the front face index data
  indices[0] = 0; indices[1] = 1; indices[2] = 2;
  indices[3] = 0; indices[4] = 2; indices[5] = 3;

  // Fill in the back face index data
  indices[6] = 4; indices[7] = 5; indices[8] = 6;
  indices[9] = 4; indices[10] = 6; indices[11] = 7;

  // Fill in the top face index data
  indices[12] = 8; indices[13] = 9; indices[14] = 10;
  indices[15] = 8; indices[16] = 10; indices[17] = 11;

  // Fill in the bottom face index data
  indices[18] = 12; indices[19] = 13; indices[20] = 14;
  indices[21] = 12; indices[22] = 14; indices[23] = 15;

  // Fill in the left face index data
  indices[24] = 16; indices[25] = 17; indices[26] = 18;
  indices[27] = 16; indices[28] = 18; indices[29] = 19;

  // Fill in the right face index data
  indices[30] = 20; indices[31] = 21; indices[32] = 22;
  indices[33] = 20; indices[34] = 22; indices[35] = 23;

  // Index buffer
  ID3D11Buffer *indexBuffer = nullptr;
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.ByteWidth = indices.size() * sizeof(UINT);
  bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
  bd.CPUAccessFlags = 0;
  bd.MiscFlags = 0;

  initData.pSysMem = &indices[0];

  DX::ThrowIfFailed(device->CreateBuffer(&bd, &initData, &indexBuffer));

  return new MeshStatic{ vertexBuffer, indexBuffer, indices, vertices };
}

MeshStatic *GeometryConstructor::createSphere(ID3D11Device *device, int latLines, int longLines) {
  UINT numVertices = ((latLines - 2) * longLines) + 2;
  UINT numSphereFaces = ((latLines - 3)*(longLines) * 2) + (longLines * 2);

  float sphereYaw = 0.0f;
  float spherePitch = 0.0f;

  std::vector<VertexStatic> vertices(numVertices);

  XMVECTOR currVertPos = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

  vertices[0].pos.x = 0.0f;
  vertices[0].pos.y = 0.0f;
  vertices[0].pos.z = 1.0f;

  for (int i = 0; i < latLines - 2; ++i)
  {
    spherePitch = (i + 1) * (3.14 / (latLines - 1));
    XMMATRIX Rotationx = XMMatrixRotationX(spherePitch);
    for (int j = 0; j < longLines; ++j)
    {
      sphereYaw = j * (6.28 / (longLines));
      XMMATRIX Rotationy = XMMatrixRotationZ(sphereYaw);
      currVertPos = XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), (Rotationx * Rotationy));
      currVertPos = XMVector3Normalize(currVertPos);

      vertices[i*longLines + j + 1].pos.x = XMVectorGetX(currVertPos);
      vertices[i*longLines + j + 1].pos.y = XMVectorGetY(currVertPos);
      vertices[i*longLines + j + 1].pos.z = XMVectorGetZ(currVertPos);
    }
  }

  vertices[numVertices - 1].pos.x = 0.0f;
  vertices[numVertices - 1].pos.y = 0.0f;
  vertices[numVertices - 1].pos.z = -1.0f;

  ID3D11Buffer *vertexBuffer = nullptr;
  {
    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(VertexStatic) * numVertices;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA vertexBufferData = {};
    vertexBufferData.pSysMem = &vertices[0];

    DX::ThrowIfFailed(device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &vertexBuffer));
  }


  UINT numIndices = numSphereFaces * 3;
  std::vector<int> indices(numIndices);

  int k = 0;
  for (int l = 0; l < longLines - 1; ++l)
  {
    indices[k] = 0;
    indices[k + 1] = l + 1;
    indices[k + 2] = l + 2;
    k += 3;
  }

  indices[k] = 0;
  indices[k + 1] = longLines;
  indices[k + 2] = 1;
  k += 3;

  for (int i = 0; i < latLines - 3; ++i)
  {
    for (int j = 0; j < longLines - 1; ++j)
    {
      indices[k] = i * longLines + j + 1;
      indices[k + 1] = i * longLines + j + 2;
      indices[k + 2] = (i + 1)*longLines + j + 1;

      indices[k + 3] = (i + 1)*longLines + j + 1;
      indices[k + 4] = i * longLines + j + 2;
      indices[k + 5] = (i + 1)*longLines + j + 2;

      k += 6;
    }

    indices[k] = (i*longLines) + longLines;
    indices[k + 1] = (i*longLines) + 1;
    indices[k + 2] = ((i + 1)*longLines) + longLines;

    indices[k + 3] = ((i + 1)*longLines) + longLines;
    indices[k + 4] = (i*longLines) + 1;
    indices[k + 5] = ((i + 1)*longLines) + 1;

    k += 6;
  }

  for (int l = 0; l < longLines - 1; ++l)
  {
    indices[k] = numVertices - 1;
    indices[k + 1] = (numVertices - 1) - (l + 1);
    indices[k + 2] = (numVertices - 1) - (l + 2);
    k += 3;
  }

  indices[k] = numVertices - 1;
  indices[k + 1] = (numVertices - 1) - longLines;
  indices[k + 2] = numVertices - 2;

  ID3D11Buffer *indexBuffer = nullptr;
  {
    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(DWORD) * numSphereFaces * 3;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = &indices[0];

    DX::ThrowIfFailed(device->CreateBuffer(&indexBufferDesc, &initData, &indexBuffer));
  }

  return new MeshStatic{ vertexBuffer, indexBuffer, indices, vertices };
}