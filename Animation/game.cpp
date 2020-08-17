#include "game.h"

// TODO(vlad): input layout for every object
// TODO(vlad): loading shaders, textures, geometry from file (game.data) - Geometry: "Plane, heightmap.bmp", "Sphere, 20, 20"

Game::Game() {
  initGame();
}

void Game::initGame() {
  // Components
  {
    camera = new Camera({ 0, 0, 0 });
    player = new Player(dx->device, camera, { 0, 0, 0 });
    fbxLoader = new FbxLoader();
    geometryConstructor = new GeometryConstructor();
  }

  // Shaders
  {
    // Plane
    VertexShader *planeVertexShader = Shaders::createVertexShaderDefault(device, L"planeVS.hlsl", "VS");
    PixelShader *planePixelShader = Shaders::createPixelShaderDefault(device, L"planePS.hlsl", "PS");
    addElementToUMap("PlaneVS", planeVertexShader, vertexShaders);
    addElementToUMap("PlanePS", planePixelShader, pixelShaders);

    // Sky
    VertexShader *skyVertexShader = Shaders::createVertexShaderDefault(device, L"skyVS.hlsl", "VS");
    PixelShader *skyPixelShader = Shaders::createPixelShaderDefault(device, L"skyPS.hlsl", "PS");
    addElementToUMap("SkyVS", skyVertexShader, vertexShaders);
    addElementToUMap("SkyPS", skyPixelShader, pixelShaders);

    // Player
    VertexShader *playerVertexShader = Shaders::createVertexShaderDefault(device, L"playerVS.hlsl", "VS");
    PixelShader *playerPixelShader = Shaders::createPixelShaderDefault(device, L"playerPS.hlsl", "PS");
    addElementToUMap("PlayerVS", playerVertexShader, vertexShaders);
    addElementToUMap("PlayerPS", playerPixelShader, pixelShaders);
  }
  
  // Input layouts
  {
    // Default
    {
      D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
      {
        { "position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "normal", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "texcoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      };
      ID3D11InputLayout *inputLayout = Shaders::createInputLayout(device, inputLayoutDesc, ARRAYSIZE(inputLayoutDesc), getElementFromUMap("PlaneVS", vertexShaders));
      addElementToUMap("Default", inputLayout, inputLayouts);
    }

    // Player
    {
      D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[] =
      {
        { "position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "normal", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "texcoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "weights", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "boneindices", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
      };
      ID3D11InputLayout *inputLayout = Shaders::createInputLayout(device, inputLayoutDesc, ARRAYSIZE(inputLayoutDesc), getElementFromUMap("PlayerVS", vertexShaders));
      addElementToUMap("Player", inputLayout, inputLayouts);
    }
  }

  // Geometry
  {
    MeshStatic *plane = geometryConstructor->createPlane(device, "heightmap.bmp");
    MeshStatic *sky = geometryConstructor->createSphere(device, 20, 20);

    addElementToUMap("Plane", new Object{ plane, MATRIX_IDENTITY }, objects);
    addElementToUMap("Sky", new Object{ sky, MATRIX_IDENTITY }, objects);
  }

  // Constant buffers
  {
    // Plane constant buffer
    {
      Object *plane = getElementFromUMap("Plane", objects);

      ConstantBufferDefault cbp = {};
      cbp.world = XMMatrixTranspose(plane->world);
      cbp.view = XMMatrixTranspose(camera->view);
      cbp.proj = XMMatrixTranspose(camera->proj);
      cbp.worldViewProj = XMMatrixTranspose(plane->world * camera->view * camera->proj);
      cbp.eye = { camera->position.m128_f32[0], camera->position.m128_f32[1], camera->position.m128_f32[2] };
      addElementToUMap("Plane", ConstantBuffer::createBuffer(device, &cbp, sizeof(ConstantBufferDefault)), buffers);
    }

    // Lightning constant buffer
    {
      ConstantBufferLightning cbl = {};
      cbl.dir = { 0.25f, 0.5f, -1.0f };
      cbl.ambient = { 0.2f, 0.2f, 0.2f, 1.0f };
      cbl.diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
      addElementToUMap("Lightning", ConstantBuffer::createBuffer(device, &cbl, sizeof(ConstantBufferLightning)), buffers);
    }
    
    // Sky constant buffer 
    {
      Object *sky = getElementFromUMap("Sky", objects);

      ConstantBufferDefault cbs = {};
      cbs.world = XMMatrixTranspose(sky->world);
      cbs.view = XMMatrixTranspose(camera->view);
      cbs.proj = XMMatrixTranspose(camera->proj);
      cbs.worldViewProj = XMMatrixTranspose(sky->world * camera->view * camera->proj);
      cbs.eye = { camera->position.m128_f32[0], camera->position.m128_f32[1], camera->position.m128_f32[2] };
      addElementToUMap("Sky", ConstantBuffer::createBuffer(device, &cbs, sizeof(ConstantBufferDefault)), buffers);
    }
  }

  // Textures
  {
    ID3D11ShaderResourceView *grassTexture = Textures::createTextureFromFile(device, "grass.dds", 0);
    ID3D11ShaderResourceView *skyTexture = Textures::createTextureFromFile(device, "skymap.dds", D3D11_RESOURCE_MISC_TEXTURECUBE);
    ID3D11ShaderResourceView *playerTexture = Textures::createTextureFromFile(device, "humanmale_Body.dds", 0);
    addElementToUMap("Grass", grassTexture, textures);
    addElementToUMap("Sky", skyTexture, textures);
    addElementToUMap("Player", playerTexture, textures);
  }

  // Player data
  {
    Model model = fbxLoader->loadFBXModel(dx, "humanmale_Idle.fbx");
    player->setMesh(model.mesh);
    player->addAnimation(IDLE, model.animation);
    player->addAnimation(RUN, fbxLoader->loadFBXModelAnimation(dx, "humanmale_Run.fbx"));

    // Fix player orientation
    XMMATRIX rotation = XMMatrixRotationY(90 * XM_PI / 180);
    player->forward = XMVector3TransformNormal(player->forward, rotation);
    player->right = XMVector3TransformNormal(player->right, rotation);
  }
}

void Game::onRender(float t) {
  deviceContext->ClearRenderTargetView(renderTargetView, Colors::Gray);
  deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

  deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  deviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

  // Plane
  {
    // Shaders
    VertexShader *vertexShader = getElementFromUMap("PlaneVS", vertexShaders);
    PixelShader *pixelShader = getElementFromUMap("PlanePS", pixelShaders);

    // Input layout
    ID3D11InputLayout *inputLayout = getElementFromUMap("Default", inputLayouts);

    // Buffers
    ID3D11Buffer *defaultBuffer = getElementFromUMap("Plane", buffers);
    ID3D11Buffer *lightningBuffer = getElementFromUMap("Lightning", buffers);

    // Resources
    ID3D11ShaderResourceView *resource = getElementFromUMap("Grass", textures);
    
    deviceContext->IASetInputLayout(inputLayout);
    deviceContext->VSSetShader(vertexShader->vs, nullptr, 0);
    deviceContext->VSSetConstantBuffers(0, 1, &defaultBuffer);
    deviceContext->PSSetShader(pixelShader->ps, nullptr, 0);
    deviceContext->PSSetSamplers(0, 1, &sampler);
    deviceContext->PSSetShaderResources(0, 1, &resource);
    deviceContext->PSSetConstantBuffers(1, 1, &lightningBuffer);

    MeshStatic *plane = getElementFromUMap("Plane", objects)->mesh;
    UINT stride = sizeof(VertexStatic);
    UINT offset = 0;

    deviceContext->IASetVertexBuffers(0, 1, &plane->vertexBuffer, &stride, &offset);
    deviceContext->IASetIndexBuffer(plane->indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    deviceContext->DrawIndexed(plane->indices.size(), 0, 0);

    // Clean
    {
      ID3D11SamplerState *nullSampler = nullptr;
      ID3D11ShaderResourceView *nullShaderResourceView = nullptr;

      deviceContext->PSSetSamplers(0, 1, &nullSampler);
      deviceContext->PSSetShaderResources(0, 1, &nullShaderResourceView);
    }
  }

  // Sky
  {
    // Shaders
    VertexShader *vertexShader = getElementFromUMap("SkyVS", vertexShaders);
    PixelShader *pixelShader = getElementFromUMap("SkyPS", pixelShaders);

    // Input layout
    ID3D11InputLayout *inputLayout = getElementFromUMap("Default", inputLayouts);

    // Buffers
    ID3D11Buffer *buffer = getElementFromUMap("Sky", buffers);

    // Resources
    ID3D11ShaderResourceView *resource = getElementFromUMap("Sky", textures);

    deviceContext->IASetInputLayout(inputLayout);
    deviceContext->VSSetShader(vertexShader->vs, nullptr, 0);
    deviceContext->VSSetConstantBuffers(0, 1, &buffer);
    deviceContext->PSSetShader(pixelShader->ps, nullptr, 0);
    deviceContext->PSSetSamplers(0, 1, &sampler);
    deviceContext->PSSetShaderResources(0, 1, &resource);

    MeshStatic *sphere = getElementFromUMap("Sky", objects)->mesh;
    UINT stride = sizeof(VertexStatic);
    UINT offset = 0;

    deviceContext->IASetVertexBuffers(0, 1, &sphere->vertexBuffer, &stride, &offset);
    deviceContext->IASetIndexBuffer(sphere->indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    deviceContext->OMSetDepthStencilState(depthStencilState, 0);
    deviceContext->RSSetState(rastState);

    deviceContext->DrawIndexed(sphere->indices.size(), 0, 0);

    // Clean
    {
      ID3D11RasterizerState *nullRastState = nullptr;
      ID3D11DepthStencilState *nullDepthStencilState = nullptr;

      deviceContext->OMSetDepthStencilState(nullDepthStencilState, 0);
      deviceContext->RSSetState(nullRastState);
    }
  }

  // Player
  {
    // Shaders
    VertexShader *vertexShader = getElementFromUMap("PlayerVS", vertexShaders);
    PixelShader *pixelShader = getElementFromUMap("PlayerPS", pixelShaders);

    // Input layout
    ID3D11InputLayout *inputLayout = getElementFromUMap("Player", inputLayouts);

    // Buffers
    ID3D11Buffer *constantBuffer = player->constantBuffer;

    // Resources
    ID3D11ShaderResourceView *resource = getElementFromUMap("Player", textures);
    
    deviceContext->IASetInputLayout(inputLayout);
    deviceContext->VSSetShader(vertexShader->vs, nullptr, 0);
    deviceContext->VSSetConstantBuffers(0, 1, &constantBuffer);
    deviceContext->PSSetShader(pixelShader->ps, nullptr, 0);
    deviceContext->PSSetSamplers(0, 1, &sampler);
    deviceContext->PSSetShaderResources(0, 1, &resource);

    MeshDynamic *mesh = player->mesh;
    UINT stride = sizeof(VertexDynamic);
    UINT offset = 0;

    deviceContext->IASetVertexBuffers(0, 1, &mesh->vertexBuffer, &stride, &offset);
    deviceContext->Draw(mesh->vertices.size(), 0);

    // Clean
    {
      ID3D11SamplerState *nullSampler = nullptr;
      ID3D11ShaderResourceView *nullShaderResourceView = nullptr;

      deviceContext->PSSetSamplers(0, 1, &nullSampler);
      deviceContext->PSSetShaderResources(0, 1, &nullShaderResourceView);
    }
  }

  DX::ThrowIfFailed(swapChain->Present(0, 0));
}

void Game::onUpdate(float t) {
  camera->update(t);

  player->update(deviceContext, camera, t);

  // Constant buffer plane
  {
    Object *plane = getElementFromUMap("Plane", objects);

    ConstantBufferDefault cbd = {};
    cbd.world = XMMatrixTranspose(plane->world);
    cbd.view = XMMatrixTranspose(camera->view);
    cbd.proj = XMMatrixTranspose(camera->proj);
    cbd.worldViewProj = XMMatrixTranspose(plane->world * camera->view * camera->proj);
    cbd.eye = XMFLOAT3{ camera->position.m128_f32[0], camera->position.m128_f32[1], camera->position.m128_f32[2] };
    
    deviceContext->UpdateSubresource(getElementFromUMap("Plane", buffers), 0, nullptr, &cbd, 0, 0);
  }

  // Constant buffer lightning
  {
    ConstantBufferLightning cbl = {};
    cbl.dir = { 0.25f, 0.5f, -1.0f };
    cbl.ambient = { 0.2f, 0.2f, 0.2f, 1.0f };
    cbl.diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };

    deviceContext->UpdateSubresource(getElementFromUMap("Lightning", buffers), 0, nullptr, &cbl, 0, 0);
  }

  // Constant buffer sky
  {
    Object *sky = getElementFromUMap("Sky", objects);

    ConstantBufferDefault cbs = {};
    sky->world = MATRIX_IDENTITY;
    sky->world *= XMMatrixScaling(5, 5, 5);
    sky->world *= XMMatrixTranslationFromVector(camera->position);
    cbs.world = XMMatrixTranspose(sky->world);
    cbs.view = XMMatrixTranspose(camera->view);
    cbs.proj = XMMatrixTranspose(camera->proj);
    cbs.worldViewProj = XMMatrixTranspose(sky->world * camera->view * camera->proj);
    cbs.eye = XMFLOAT3{ camera->position.m128_f32[0], camera->position.m128_f32[1], camera->position.m128_f32[2] };

    deviceContext->UpdateSubresource(getElementFromUMap("Sky", buffers), 0, nullptr, &cbs, 0, 0);
  }
}

void Game::onInput(float t) {
  // Camera
  {
    if (GetAsyncKeyState(0x44) & 0x8000)
    {
      camera->moveRight(SPEED);
    }
    else if (GetAsyncKeyState(0x41) & 0x8000)
    {
      camera->moveRight(-SPEED);
    }
    else if (GetAsyncKeyState(0x57) & 0x8000)
    {
      camera->moveForward(SPEED);
    }
    else if (GetAsyncKeyState(0x53) & 0x8000)
    {
      camera->moveForward(-SPEED);
    }
  }
  
  // Player
  {
    player->currentState = IDLE;

    if (GetAsyncKeyState(VK_UP) & 0x8000) {
      player->move(FORWARD, SPEED);
    }

    if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
      player->move(BACKWARD, SPEED);
    }

    if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
      player->move(LEFT, SPEED * t * 15.0f);
    }

    if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
      player->move(RIGHT, SPEED * t * 15.0f);
    }
  }
}

void Game::run() {
  MSG msg = {};

  float updateTimer = 0;
  float frameTime = 1 / 144.0f;

  Time::TimePoint startTime = Time::timeGet();
  Time::TimePoint lastTime = Time::timeGet();

  while (msg.message != WM_QUIT)
  {
    Time::TimePoint currentTime = Time::timeGet();
    float passedTime = Time::getDuration(currentTime, lastTime);

    updateTimer += passedTime;

    bool should_render = false;
    while (updateTimer >= frameTime)
    {
      onInput(frameTime);
      onUpdate(frameTime);
      updateTimer -= frameTime;
      should_render = true;
    }

    if (should_render)
    {
      onRender(frameTime);
    }

    if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    else
    {
      std::this_thread::sleep_for(std::chrono::microseconds(1));
    }

    lastTime = currentTime;
  }
}

void Game::onMouseDown(WPARAM btnState, int x, int y) {

}
void Game::onMouseUp(WPARAM btnState, int x, int y) {

}
void Game::onMouseMove(WPARAM btnState, int x, int y) {
  float dx = XMConvertToRadians(0.25f*static_cast<float>(x - prevMousePosition.x));
  float dy = XMConvertToRadians(0.25f*static_cast<float>(y - prevMousePosition.y));

  if ((btnState & MK_LBUTTON) != 0)
  {
    camera->addPitch(dy);
    camera->addYaw(dx);
  }
  else if ((btnState & MK_RBUTTON) != 0)
  {
    camera->addPitch(dy);
    camera->addYaw(dx);
  }

  prevMousePosition.x = x;
  prevMousePosition.y = y;
}