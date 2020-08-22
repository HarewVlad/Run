#include "player.h"
#include "camera.h"

Player::Player(ID3D11Device *device, Camera *camera, const XMVECTOR &position) {
  this->position = position;
  this->forward = { 0, 0, 1 };
  this->right = { 1, 0, 0 };
  this->up = { 0, 1, 0 };
  this->world = MATRIX_IDENTITY;

  // Constant buffer
  {
    ConstantBufferPlayer cbp = {};
    cbp.world = XMMatrixTranspose(world);
    cbp.view = XMMatrixTranspose(camera->view);
    cbp.proj = XMMatrixTranspose(camera->proj);
    cbp.worldViewProj = XMMatrixTranspose(world * camera->view * camera->proj);
    cbp.rotation = XMMatrixTranspose(XMMatrixIdentity());
    cbp.eye = { camera->position.m128_f32[0], camera->position.m128_f32[1], camera->position.m128_f32[2] };

    constantBuffer = ConstantBuffer::createBuffer(device, reinterpret_cast<void *>(&cbp), sizeof(ConstantBufferPlayer));
  }

  this->currentState = IDLE;
}

void Player::update(ID3D11DeviceContext *deviceContext, Camera *camera, float t) {
  Animation *animation = animations[currentState];
  animation->update(t);

  // Constant buffer
  {
    ConstantBufferPlayer cbp = {};
    cbp.world = XMMatrixTranspose(world);
    cbp.view = XMMatrixTranspose(camera->view);
    cbp.proj = XMMatrixTranspose(camera->proj);
    cbp.worldViewProj = XMMatrixTranspose(world * camera->view * camera->proj);
    cbp.rotation = XMMatrixTranspose(XMMatrixIdentity());
    cbp.eye = XMFLOAT3(camera->position.m128_f32[0], camera->position.m128_f32[1], camera->position.m128_f32[2]);

    memcpy(&cbp.boneTransforms[0],
      &animation->finalTransforms[0],
      sizeof(XMMATRIX) * animation->finalTransforms.size());
    deviceContext->UpdateSubresource(constantBuffer, 0, nullptr, &cbp, 0, 0);
  }
}

void Player::move(MoveDirection direction, Camera *camera, float value) {
  currentState = RUN;
  currentDirection = direction;

  switch (direction) {
  case MoveDirection::FORWARD:
    position += forward * value;
    world *= XMMatrixTranslationFromVector(forward * value);

    // Move bounding box
    boundingBox->min += forward * value;
    boundingBox->max += forward * value;

    // Move camera 
    if (camera->isAttached) {
      camera->updateFollowTarget(position, forward, right, up);
    }
    break;
  case MoveDirection::BACKWARD:
    position -= forward * value;
    world *= XMMatrixTranslationFromVector(forward * -value);

    // Move bounding box
    boundingBox->min -= forward * value;
    boundingBox->max -= forward * value;

    // Move camera 
    if (camera->isAttached) {
      camera->updateFollowTarget(position, forward, right, up);
    }
    break;
  case MoveDirection::LEFT:
    // Rotate to left
    XMMATRIX rotation = XMMatrixRotationAxis(up, -value);
    right = XMVector3TransformNormal(right, rotation);
    forward = XMVector3TransformNormal(forward, rotation);

    world *= XMMatrixTranslationFromVector(-position) * rotation * XMMatrixTranslationFromVector(position);

    // Move camera
    if (camera->isAttached) {
      camera->updateFollowTarget(position, forward, right, up);
    }
    break;
  case MoveDirection::RIGHT:
    // Rotate to right
    rotation = XMMatrixRotationAxis(up, value);
    right = XMVector3TransformNormal(right, rotation);
    forward = XMVector3TransformNormal(forward, rotation);

    world *= XMMatrixTranslationFromVector(-position) * rotation * XMMatrixTranslationFromVector(position);

    // Move camera
    if (camera->isAttached) {
      camera->updateFollowTarget(position, forward, right, up);
    }
    break;
  }
}