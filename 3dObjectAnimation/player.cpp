#include "player.h"

void Player::update(Directx *dx, float t) {
  // Update animation
  {
    animationsData[currentAnimationName]->update(currentAnimationName, t);
  }

  // Update constant buffer
  {
    ConstantBuffer updatedConstantBuffer = {};
    updatedConstantBuffer.world = XMMatrixTranspose(camera->world);
    updatedConstantBuffer.view = XMMatrixTranspose(camera->view);
    updatedConstantBuffer.proj = XMMatrixTranspose(camera->proj);
    updatedConstantBuffer.worldViewProj = XMMatrixTranspose(camera->world * camera->view * camera->proj);
    updatedConstantBuffer.eye = XMFLOAT3(camera->pos.m128_f32[0], camera->pos.m128_f32[1], camera->pos.m128_f32[2]);

    AnimationData *animationData = getCurrentAnimation();
    memcpy(&updatedConstantBuffer.boneTransforms[0],
      &animationData->finalTransforms[0],
      sizeof(XMMATRIX) * animationData->finalTransforms.size());
    dx->deviceContext->UpdateSubresource(constantBuffer, 0, nullptr, &updatedConstantBuffer, 0, 0);
  }

  // Update camera
  {
    camera->update(t);
  }
  
  // update Player position
}

void Player::initCamera() {
  static bool isInited = false;
  if (isInited) {
    return;
  }
}

void Player::move(MoveDirection moveDirection, float value) {
  switch (moveDirection) {
  case FORWARD:
    pos += forward * value;
    currentAnimationName = "run.fbx";
    break;
  case BACKWARD:
    pos -= forward * value;
    currentAnimationName = "run.fbx";
    break;
  case LEFT:
    pos += right * value;
    currentAnimationName = "run.fbx";
    break;
  case RIGHT:
    pos -= right * value;
    currentAnimationName = "run.fbx";
    break;
  default:
    OutputDebugStringA("Unsupported player direction movement!");
    break;
  }
}