#include "utils.h"
#include "game.h"

void Game::init() {
  dx = new Directx {};
  dx->initWindow();
  dx->initDirectx();

  // Camera
  camera = new Camera {};
  camera->init({ 0.0f, 10.0f, 30.0f }, { 0, 0, 0 });

  // Fbx manager
  fbxManager = FbxManager::Create();
  FbxIOSettings *fbxIOSettings = FbxIOSettings::Create(fbxManager, IOSROOT);
  fbxManager->SetIOSettings(fbxIOSettings);

  // Shaders
  //dx->createVertexShader(L"testVs.hlsl", "VS");
  //dx->createPixelShader(L"testPs.hlsl", "PS");
  dx->createVertexShader(L"scorpVs.hlsl", "VS");
  dx->createPixelShader(L"scorpPs.hlsl", "PS");

  // Input layouts
  D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
  {
    { "position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "normal", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "texcoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "weights", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "boneindices", 0, DXGI_FORMAT_R8G8B8A8_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
  };
  dx->createInputLayout("mainLayout", vertexDesc, ARRAYSIZE(vertexDesc), &dx->vertexShaders[L"scorpVs.hlsl"]);

  // Buffers
  ConstantBuffer cb = {};
  cb.world = XMMatrixTranspose(camera->world);
  cb.view = XMMatrixTranspose(camera->view);
  cb.proj = XMMatrixTranspose(camera->proj);
  cb.worldViewProj = XMMatrixTranspose(camera->world * camera->view * camera->proj);
  cb.eye = { camera->pos.m128_f32[0], camera->pos.m128_f32[1], camera->pos.m128_f32[2] };
  dx->createBuffer("constantBuffer", cb);

  // Textures
  // Scorp skin
  dx->createTexture("Kachujin.dds", 0);

  // Objects
  geometryManager = new GeometryManager();
  geometryManager->createBox3D(dx, "box3d", 10, 10, 10);
  geometryManager->createFBXModel(dx, fbxManager, "idle.fbx");
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
  // Keyboard
  {
    if (GetKeyState(0x44) & 0x8000)
    {
      camera->moveRight += SPEED;
    }
    else if (GetKeyState(0x41) & 0x8000)
    {
      camera->moveRight -= SPEED;
    }
    else if (GetKeyState(0x57) & 0x8000)
    {
      camera->moveForward += SPEED;
    }
    else if (GetKeyState(0x53) & 0x8000)
    {
      camera->moveForward -= SPEED;
    }
    else if (GetKeyState(VK_SPACE) & 0x8000)
    {
      dx->swapChain->SetFullscreenState(true, NULL);
    }
    else if (GetKeyState(VK_SHIFT) & 0x8000)
    {
      dx->swapChain->SetFullscreenState(false, NULL);
    }
  }
  // Mouse 
  {
    POINT newMousePos = {};
    if (GetKeyState(VK_RBUTTON) & 0x8000)
    {
      GetCursorPos(&newMousePos);
      if (ScreenToClient(dx->mainWindow, &newMousePos))
      {
        if (newMousePos.x > 0 && newMousePos.y > 0)
        {
          if (newMousePos.x <= Window::WIDTH && newMousePos.y <= Window::HEIGHT)
          {
            prevMousePos = currMousePos;
            currMousePos = newMousePos;
          }
        }
      }
    }
    else
    {
      GetCursorPos(&newMousePos);
      if (ScreenToClient(dx->mainWindow, &newMousePos))
      {
        if (newMousePos.x > 0 && newMousePos.y > 0)
        {
          if (newMousePos.x <= Window::WIDTH && newMousePos.y <= Window::HEIGHT)
          {
            currMousePos = newMousePos;
            prevMousePos = currMousePos;
          }
        }
      }
    }
  }
}

void Game::update(float t) {
  // Camera
  {
    camera->update(prevMousePos, currMousePos, t);
  }

  // Test FBX Model
  // TODO: redo this part and make struct and so on
  {
    AnimationData *animationData = geometryManager->getObjectAnimationData("idle.fbx");
    animationData->update(t);
  }

  // Update constant buffer
  {
    ID3D11Buffer *constantBuffer = dx->buffers["constantBuffer"];

    ConstantBuffer updatedConstantBuffer = {};
    updatedConstantBuffer.world = XMMatrixTranspose(camera->world);
    updatedConstantBuffer.view = XMMatrixTranspose(camera->view);
    updatedConstantBuffer.proj = XMMatrixTranspose(camera->proj);
    updatedConstantBuffer.worldViewProj = XMMatrixTranspose(camera->world * camera->view * camera->proj);
    updatedConstantBuffer.eye = XMFLOAT3(camera->pos.m128_f32[0], camera->pos.m128_f32[1], camera->pos.m128_f32[2]);

    // TODO: Change this shit
    AnimationData *animationData = geometryManager->getObjectAnimationData("idle.fbx");
    memcpy(&updatedConstantBuffer.boneTransforms[0],
      &animationData->finalTransforms[0],
      sizeof(XMMATRIX) * animationData->finalTransforms.size());
    dx->deviceContext->UpdateSubresource(constantBuffer, 0, nullptr, &updatedConstantBuffer, 0, 0);
  }
}

void Game::render(float t) {
  dx->deviceContext->ClearRenderTargetView(dx->renderTargetView, Colors::Gray);
  dx->deviceContext->ClearDepthStencilView(dx->depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

  dx->deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  dx->deviceContext->IASetInputLayout(dx->layouts["mainLayout"]);

  dx->deviceContext->OMSetRenderTargets(1, &dx->renderTargetView, dx->depthStencilView);

  // Cube 3d
  /*{
    VertexShader vertexShader = dx->vertexShaders[L"testVs.hlsl"];
    PixelShader pixelShader = dx->pixelShaders[L"testPs.hlsl"];
    ID3D11Buffer *buffer = dx->buffers["constantBuffer"];

    dx->deviceContext->VSSetShader(vertexShader.vs, nullptr, 0);
    dx->deviceContext->VSSetConstantBuffers(0, 1, &buffer);
    dx->deviceContext->PSSetShader(pixelShader.ps, nullptr, 0);

    Geometry *box3d = objects["box3d"];

    UINT stride = sizeof(Vertex);
    UINT offset = 0;

    dx->deviceContext->IASetVertexBuffers(0, 1, &box3d->vertexBuffer, &stride, &offset);
    dx->deviceContext->IASetIndexBuffer(box3d->indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    dx->deviceContext->DrawIndexed(box3d->indices.size(), 0, 0);
  }*/

  // Fbx scorp
  {
    VertexShader vertexShader = dx->vertexShaders[L"scorpVs.hlsl"];
    PixelShader pixelShader = dx->pixelShaders[L"scorpPs.hlsl"];
    ID3D11Buffer *buffer = dx->buffers["constantBuffer"];
    ID3D11ShaderResourceView *scorpTexture = dx->textures["Kachujin.dds"];

    dx->deviceContext->VSSetShader(vertexShader.vs, nullptr, 0);
    dx->deviceContext->VSSetConstantBuffers(0, 1, &buffer);
    dx->deviceContext->PSSetShader(pixelShader.ps, nullptr, 0);
    dx->deviceContext->PSSetShaderResources(0, 1, &scorpTexture);
    dx->deviceContext->PSSetSamplers(0, 1, &dx->sampler);

    Mesh *mesh = geometryManager->getObjectMeshData("idle.fbx");
    
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    
    dx->deviceContext->IASetVertexBuffers(0, 1, &mesh->vertexBuffer, &stride, &offset);
    dx->deviceContext->Draw(mesh->vertices.size(), 0);
  }

  DX::ThrowIfFailed(dx->swapChain->Present(0, 0));
}