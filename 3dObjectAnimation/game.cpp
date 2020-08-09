#include "game.h"

void Game::init() {
  // Camera
  camera = new Camera({ 0.0f, 10.0f, 30.0f }, { 0, 0, -1 });

  // Fbx manager
  fbxManager = FbxManager::Create();
  FbxIOSettings *fbxIOSettings = FbxIOSettings::Create(fbxManager, IOSROOT);
  fbxManager->SetIOSettings(fbxIOSettings);

  // Shaders
  Directx::createVertexShader(L"testVs.hlsl", "VS");
  Directx::createPixelShader(L"testPs.hlsl", "PS");
  Directx::createVertexShader(L"scorpVs.hlsl", "VS");
  Directx::createPixelShader(L"scorpPs.hlsl", "PS");

  // Input layouts
  D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
  {
    { "position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "normal", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "texcoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "weights", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "boneindices", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
  };
  Directx::createInputLayout("mainLayout", vertexDesc, ARRAYSIZE(vertexDesc), &dx->vertexShaders[L"scorpVs.hlsl"]);

  // Buffers
  ConstantBuffer cb = {};
  cb.world = XMMatrixTranspose(camera->world);
  cb.view = XMMatrixTranspose(camera->view);
  cb.proj = XMMatrixTranspose(camera->proj);
  cb.worldViewProj = XMMatrixTranspose(camera->world * camera->view * camera->proj);
  cb.eye = { camera->pos.m128_f32[0], camera->pos.m128_f32[1], camera->pos.m128_f32[2] };
  Directx::createBuffer("constantBuffer", cb);

  // Textures
  // Scorp skin
  Directx::createTexture("Kachujin.dds", 0);

  // Objects
  geometryManager = new GeometryManager();
  geometryManager->createBox3D(dx, "box3d", 10, 10, 10);
  geometryManager->createFBXModel(dx, fbxManager, "idle.fbx");
  geometryManager->createFBXModel(dx, fbxManager, "run.fbx"); // TODO: make it run in parallel

  // Player
  player = new Player(dx);
  player->setMeshData(geometryManager->getObjectMeshData("idle.fbx"));
  player->addAnimationData("idle.fbx", geometryManager->getObjectAnimationData("idle.fbx"));
  player->addAnimationData("run.fbx", geometryManager->getObjectAnimationData("run.fbx"));
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
      input(frameTime);
      update(frameTime);
      updateTimer -= frameTime;
      should_render = true;
    }

    if (should_render)
    {
      render(frameTime);
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

void Game::input(float t) {
  player->currentAnimationName = "idle.fbx";

  // Keyboard
  {
    if (GetAsyncKeyState(VK_UP) & 0x8000) {
      player->move(FORWARD, SPEED); // NOTE: to move the player -> XMMatrixTranslawtion(x, y, z) ... player->world *= translationMatrix;
      XMMATRIX translationMatrix = XMMatrixTranslationFromVector(player->forward * SPEED);
      player->camera->world *= translationMatrix;
    }
    else if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
      player->move(BACKWARD, SPEED); // NOTE: to move the player -> XMMatrixTranslation(x, y, z) ... player->world *= translationMatrix;
      XMMATRIX translationMatrix = XMMatrixTranslationFromVector(player->forward * -SPEED);
      player->camera->world *= translationMatrix;
    }
    else if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
      player->move(LEFT, SPEED); // NOTE: to move the player -> XMMatrixTranslation(x, y, z) ... player->world *= translationMatrix;
      XMMATRIX translationMatrix = XMMatrixTranslationFromVector(player->right * -SPEED);
      player->camera->world *= translationMatrix;
    }
    else if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
      player->move(RIGHT, SPEED);
      XMMATRIX translationMatrix = XMMatrixTranslationFromVector(player->right * SPEED);
      player->camera->world *= translationMatrix;
    }
    else if (GetAsyncKeyState(0x44) & 0x8000)
    {
      player->camera->moveRight(SPEED);
    }
    else if (GetAsyncKeyState(0x41) & 0x8000)
    {
      player->camera->moveRight(-SPEED);
    }
    else if (GetAsyncKeyState(0x57) & 0x8000)
    {
      player->camera->moveForward(SPEED);
    }
    else if (GetAsyncKeyState(0x53) & 0x8000)
    {
      player->camera->moveForward(-SPEED);
    }
    else if (GetKeyState(VK_SPACE) & 0x8000)
    {
      // Directx::swapChain->SetFullscreenState(true, NULL);
    }
    /*
    else if (GetKeyState(VK_SHIFT) & 0x8000)
    {
      Directx::swapChain->SetFullscreenState(false, NULL);
    }
    */
  }
}

void Game::onMouseDown(WPARAM btnState, int x, int y) {

}
void Game::onMouseUp(WPARAM btnState, int x, int y) {

}
void Game::onMouseMove(WPARAM btnState, int x, int y) {
  float dx = XMConvertToRadians(0.25f*static_cast<float>(x - prevMousePos.x));
  float dy = XMConvertToRadians(0.25f*static_cast<float>(y - prevMousePos.y));

  if ((btnState & MK_LBUTTON) != 0)
  {
    player->camera->rotateX += dx;
    player->camera->rotateY += dy;
  }
  else if ((btnState & MK_RBUTTON) != 0)
  {
    player->camera->rotateX += dx;
    player->camera->rotateY += dy;
  }

  prevMousePos.x = x;
  prevMousePos.y = y;
}

void Game::update(float t) {
  // TODO: redo this part and make struct and so on
  {
    player->update(dx, t);
  }
}

void Game::render(float t) {
  Directx::deviceContext->ClearRenderTargetView(dx->renderTargetView, Colors::Gray);
  Directx::deviceContext->ClearDepthStencilView(dx->depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

  Directx::deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  Directx::deviceContext->IASetInputLayout(dx->layouts["mainLayout"]);

  Directx::deviceContext->OMSetRenderTargets(1, &dx->renderTargetView, dx->depthStencilView);

  /*
  // Cube 3d
  {
    VertexShader vertexShader = dx->vertexShaders[L"testVs.hlsl"];
    PixelShader pixelShader = dx->pixelShaders[L"testPs.hlsl"];
    ID3D11Buffer *buffer = dx->buffers["constantBuffer"];

    dx->deviceContext->VSSetShader(vertexShader.vs, nullptr, 0);
    dx->deviceContext->VSSetConstantBuffers(0, 1, &buffer);
    dx->deviceContext->PSSetShader(pixelShader.ps, nullptr, 0);

    Mesh *mesh = geometryManager->getObjectMeshData("box3d");

    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    dx->deviceContext->IASetVertexBuffers(0, 1, &mesh->vertexBuffer, &stride, &offset);
    dx->deviceContext->IASetIndexBuffer(mesh->indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    dx->deviceContext->DrawIndexed(mesh->indices.size(), 0, 0);
  }
  */

  // Fbx scorp
  {
    VertexShader vertexShader = Directx::vertexShaders[L"scorpVs.hlsl"];
    PixelShader pixelShader = Directx::pixelShaders[L"scorpPs.hlsl"];
    ID3D11Buffer *buffer = player->getConstantBuffer();
    ID3D11ShaderResourceView *modelTexture = Directx::textures["Kachujin.dds"];

    Directx::deviceContext->VSSetShader(vertexShader.vs, nullptr, 0);
    Directx::deviceContext->VSSetConstantBuffers(0, 1, &buffer);
    Directx::deviceContext->PSSetShader(pixelShader.ps, nullptr, 0);
    Directx::deviceContext->PSSetShaderResources(0, 1, &modelTexture);
    Directx::deviceContext->PSSetSamplers(0, 1, &sampler);

    Mesh *mesh = geometryManager->getObjectMeshData("idle.fbx"); // NOTE: player
    
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    
    Directx::deviceContext->IASetVertexBuffers(0, 1, &mesh->vertexBuffer, &stride, &offset);
    Directx::deviceContext->Draw(mesh->vertices.size(), 0);
  }

  DX::ThrowIfFailed(Directx::swapChain->Present(0, 0));
}