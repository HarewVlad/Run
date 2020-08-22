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
    player = new Player(device, camera, { 0, 0, 0 });
    fbxLoader = new FbxLoader();
    collisionManager = new CollisionManager();
    geometryGenerator = new GeometryGenerator();
  }

  // Shaders
  {
    // Plane
    VertexShader *planeVertexShader = GPU::createVertexShaderDefault(device, L"planeVS.hlsl", "VS");
    PixelShader *planePixelShader = GPU::createPixelShaderDefault(device, L"planePS.hlsl", "PS");
    addElementToUMap("PlaneVS", planeVertexShader, vertexShaders);
    addElementToUMap("PlanePS", planePixelShader, pixelShaders);

    // Sky
    VertexShader *skyVertexShader = GPU::createVertexShaderDefault(device, L"skyVS.hlsl", "VS");
    PixelShader *skyPixelShader = GPU::createPixelShaderDefault(device, L"skyPS.hlsl", "PS");
    addElementToUMap("SkyVS", skyVertexShader, vertexShaders);
    addElementToUMap("SkyPS", skyPixelShader, pixelShaders);

    // Player
    VertexShader *playerVertexShader = GPU::createVertexShaderDefault(device, L"playerVS.hlsl", "VS");
    PixelShader *playerPixelShader = GPU::createPixelShaderDefault(device, L"playerPS.hlsl", "PS");
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
      ID3D11InputLayout *inputLayout = GPU::createInputLayout(device, inputLayoutDesc, ARRAYSIZE(inputLayoutDesc), getElementFromUMap("PlaneVS", vertexShaders));
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
      ID3D11InputLayout *inputLayout = GPU::createInputLayout(device, inputLayoutDesc, ARRAYSIZE(inputLayoutDesc), getElementFromUMap("PlayerVS", vertexShaders));
      addElementToUMap("Player", inputLayout, inputLayouts);
    }
  }

  // Geometry
  {
    // MeshStatic *plane = geometryGenerator->createPlaneFromHeightmap(device, "heightmap.bmp");
    MeshStatic *plane = geometryGenerator->createPlaneDefault(device, { 0, 0, 0 }, 1000, 1000, 1000, 1000);
    MeshStatic *sky = geometryGenerator->createSphereDefault(device, 20, 20);
    MeshStatic *cupboard = fbxLoader->loadFBXModelMesh(device, "wardrobedwarvenaverage02_[0]_Stand_2315.fbx");

    addElementToUMap("Plane", plane, objects);
    addElementToUMap("Sky", sky, objects);
    addElementToUMap("Cupboard", cupboard, objects);
  }

  // Constant buffers
  {
    // Light
    {
      ConstantBufferLightning cbl = {};
      cbl.dir = { 0.25f, 0.5f, -1.0f };
      cbl.ambient = { 0.2f, 0.2f, 0.2f, 1.0f };
      cbl.diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
      addElementToUMap("Lightning", ConstantBuffer::createBuffer(device, &cbl, sizeof(ConstantBufferLightning)), buffers);
    }

    // Default
    {
      ConstantBufferDefault cbd = {};
      cbd.world = MATRIX_IDENTITY;
      cbd.view = camera->view;
      cbd.proj = camera->proj;
      cbd.worldViewProj = XMMatrixTranspose(MATRIX_IDENTITY * camera->view * camera->proj);
      cbd.eye = { camera->position.m128_f32[0], camera->position.m128_f32[1], camera->position.m128_f32[2] };
      
      addElementToUMap("Default", ConstantBuffer::createBuffer(device, &cbd, sizeof(ConstantBufferDefault)), buffers);
    }

    // Sky
    {
      ConstantBufferDefault cbd = {};
      cbd.world = MATRIX_IDENTITY;
      cbd.view = camera->view;
      cbd.proj = camera->proj;
      cbd.worldViewProj = XMMatrixTranspose(MATRIX_IDENTITY * camera->view * camera->proj);
      cbd.eye = { camera->position.m128_f32[0], camera->position.m128_f32[1], camera->position.m128_f32[2] };

      addElementToUMap("Sky", ConstantBuffer::createBuffer(device, &cbd, sizeof(ConstantBufferDefault)), buffers);
    }
  }

  // Textures
  {
    ID3D11ShaderResourceView *grassTexture = GPU::createTextureFromFile(device, "grass.dds", 0);
    ID3D11ShaderResourceView *skyTexture = GPU::createTextureFromFile(device, "skymap.dds", D3D11_RESOURCE_MISC_TEXTURECUBE);
    ID3D11ShaderResourceView *playerTexture = GPU::createTextureFromFile(device, "humanmale_Body.dds", 0);
    ID3D11ShaderResourceView *cupboardTexture = GPU::createTextureFromFile(device, "DwarvenBookshelvesAverage01.dds", 0);
    addElementToUMap("Grass", grassTexture, textures);
    addElementToUMap("Sky", skyTexture, textures);
    addElementToUMap("Player", playerTexture, textures);
    addElementToUMap("Cupboard", cupboardTexture, textures);
  }

  // Player data
  {
    Model model = fbxLoader->loadFBXModel(device, "humanmale_Idle.fbx");
    player->setMesh(model.mesh);
    player->addAnimation(IDLE, model.animation);
    player->addAnimation(RUN, fbxLoader->loadFBXModelAnimation(device, "humanmale_Run.fbx"));

    // Fix player orientation
    XMMATRIX rotation = XMMatrixRotationY(90 * XM_PI / 180);
    player->forward = XMVector3TransformNormal(player->forward, rotation);
    player->right = XMVector3TransformNormal(player->right, rotation);
  }

  // Collision
  {
    // Player
    player->boundingBox = collisionManager->createBBForPlayer(player->mesh->vertices);
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
    ID3D11Buffer *lightningBuffer = getElementFromUMap("Lightning", buffers);
    ID3D11Buffer *planeBuffer = getElementFromUMap("Default", buffers);

    // Resources
    ID3D11ShaderResourceView *resource = getElementFromUMap("Grass", textures);
    
    deviceContext->IASetInputLayout(inputLayout);
    deviceContext->VSSetShader(vertexShader->vs, nullptr, 0);
    deviceContext->VSSetConstantBuffers(0, 1, &planeBuffer);
    deviceContext->PSSetShader(pixelShader->ps, nullptr, 0);
    deviceContext->PSSetSamplers(0, 1, &sampler);
    deviceContext->PSSetShaderResources(0, 1, &resource);
    deviceContext->PSSetConstantBuffers(1, 1, &lightningBuffer);

    MeshStatic *mesh = getElementFromUMap("Plane", objects);
    UINT stride = sizeof(VertexStatic);
    UINT offset = 0;

    deviceContext->IASetVertexBuffers(0, 1, &mesh->vertexBuffer, &stride, &offset);
    deviceContext->IASetIndexBuffer(mesh->indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    deviceContext->DrawIndexed(mesh->indices.size(), 0, 0);

    // Clean
    {
      ID3D11SamplerState *nullSampler = nullptr;
      ID3D11ShaderResourceView *nullShaderResourceView = nullptr;

      deviceContext->PSSetSamplers(0, 1, &nullSampler);
      deviceContext->PSSetShaderResources(0, 1, &nullShaderResourceView);
    }
  }

  // Cupboard
  {
    // Shaders
    VertexShader *vertexShader = getElementFromUMap("PlaneVS", vertexShaders);
    PixelShader *pixelShader = getElementFromUMap("PlanePS", pixelShaders);

    // Input layout
    ID3D11InputLayout *inputLayout = getElementFromUMap("Default", inputLayouts);

    // Buffers
    ID3D11Buffer *lightningBuffer = getElementFromUMap("Lightning", buffers);
    ID3D11Buffer *cupboardBuffer = getElementFromUMap("Default", buffers);

    // Resources
    ID3D11ShaderResourceView *cupboardTexture = getElementFromUMap("Cupboard", textures);

    deviceContext->IASetInputLayout(inputLayout);
    deviceContext->VSSetShader(vertexShader->vs, nullptr, 0);
    deviceContext->VSSetConstantBuffers(0, 1, &cupboardBuffer);
    deviceContext->PSSetShader(pixelShader->ps, nullptr, 0);
    deviceContext->PSSetSamplers(0, 1, &sampler);
    deviceContext->PSSetShaderResources(0, 1, &cupboardTexture);
    deviceContext->PSSetConstantBuffers(1, 1, &lightningBuffer);

    MeshStatic *mesh = getElementFromUMap("Cupboard", objects);
    UINT stride = sizeof(VertexStatic);
    UINT offset = 0;

    deviceContext->IASetVertexBuffers(0, 1, &mesh->vertexBuffer, &stride, &offset);
    deviceContext->IASetIndexBuffer(mesh->indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    deviceContext->Draw(mesh->vertices.size(), 0);

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
    ID3D11Buffer *skyBuffer = getElementFromUMap("Sky", buffers);

    // Resources
    ID3D11ShaderResourceView *resource = getElementFromUMap("Sky", textures);

    deviceContext->IASetInputLayout(inputLayout);
    deviceContext->VSSetShader(vertexShader->vs, nullptr, 0);
    deviceContext->VSSetConstantBuffers(0, 1, &skyBuffer);
    deviceContext->PSSetShader(pixelShader->ps, nullptr, 0);
    deviceContext->PSSetSamplers(0, 1, &sampler);
    deviceContext->PSSetShaderResources(0, 1, &resource);

    MeshStatic *mesh = getElementFromUMap("Sky", objects);
    UINT stride = sizeof(VertexStatic);
    UINT offset = 0;

    deviceContext->IASetVertexBuffers(0, 1, &mesh->vertexBuffer, &stride, &offset);
    deviceContext->IASetIndexBuffer(mesh->indexBuffer, DXGI_FORMAT_R32_UINT, 0);

    deviceContext->OMSetDepthStencilState(depthStencilState, 0);
    deviceContext->RSSetState(rastState);

    deviceContext->DrawIndexed(mesh->indices.size(), 0, 0);

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
    ID3D11Buffer *playerBuffer = player->constantBuffer;
    ID3D11Buffer *lightningBuffer = getElementFromUMap("Lightning", buffers);

    // Resources
    ID3D11ShaderResourceView *resource = getElementFromUMap("Player", textures);
    
    deviceContext->IASetInputLayout(inputLayout);
    deviceContext->VSSetShader(vertexShader->vs, nullptr, 0);
    deviceContext->VSSetConstantBuffers(0, 1, &playerBuffer);
    deviceContext->PSSetShader(pixelShader->ps, nullptr, 0);
    deviceContext->PSSetConstantBuffers(1, 0, &lightningBuffer);
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

  // Update plane with player pos
  {
    XMFLOAT3 playerPosition;
    XMStoreFloat3(&playerPosition, player->position);
    MeshStatic *plane = geometryGenerator->createPlaneDefault(device, playerPosition, 100, 100, 100, 100);

    changeElementInUMap("Plane", plane, objects);
  }

  // Default
  {
    ID3D11Buffer *plane = getElementFromUMap("Default", buffers);

    ConstantBufferDefault cbd = {};
    cbd.world = XMMatrixTranspose(MATRIX_IDENTITY);
    cbd.view = XMMatrixTranspose(camera->view);
    cbd.proj = XMMatrixTranspose(camera->proj);
    cbd.worldViewProj = XMMatrixTranspose(MATRIX_IDENTITY * camera->view * camera->proj);
    cbd.eye = XMFLOAT3{ camera->position.m128_f32[0], camera->position.m128_f32[1], camera->position.m128_f32[2] };

    deviceContext->UpdateSubresource(plane, 0, nullptr, &cbd, 0, 0);
  }

  // Sky
  {
    ID3D11Buffer *sky = getElementFromUMap("Sky", buffers);

    XMMATRIX world = MATRIX_IDENTITY * XMMatrixScaling(5, 5, 5) * XMMatrixTranslationFromVector(camera->position);

    ConstantBufferDefault cbd = {};
    cbd.world = XMMatrixTranspose(world);
    cbd.view = XMMatrixTranspose(camera->view);
    cbd.proj = XMMatrixTranspose(camera->proj);
    cbd.worldViewProj = XMMatrixTranspose(world * camera->view * camera->proj);
    cbd.eye = XMFLOAT3{ camera->position.m128_f32[0], camera->position.m128_f32[1], camera->position.m128_f32[2] };

    deviceContext->UpdateSubresource(sky, 0, nullptr, &cbd, 0, 0);
  }
}

void Game::onInput(float t) {
  // Camera
  if (!camera->isAttached)
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

  // Camera controlls
  {
    if (GetAsyncKeyState(0x11) & 0x8000) {
      camera->attachCamera(player->position, player->forward, player->right, player->up);
    }
    else if (GetAsyncKeyState(0xA0) & 0x8000) {
      camera->detachCamera();
    }
  }
  
  // Player
  {
    MoveDirection direction = MoveDirection::NONE;
    player->currentState = IDLE;

    if (GetAsyncKeyState(VK_UP) & 0x8000) {
      player->move(MoveDirection::FORWARD, camera, SPEED);
    }

    if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
      player->move(MoveDirection::BACKWARD, camera, SPEED);
    }

    if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
      player->move(MoveDirection::LEFT, camera, SPEED * t * 15.0f);
    }

    if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
      player->move(MoveDirection::RIGHT, camera, SPEED * t * 15.0f);
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