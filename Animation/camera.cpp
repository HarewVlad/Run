#include "camera.h"

Camera::Camera() : Camera({ 0, 0, 0 }) {
}

Camera::Camera(const XMVECTOR &position) {
  this->position = position;
  this->forward = DEFAULT_FORWARD;
  this->up = DEFAULT_UP;
  this->right = DEFAULT_RIGHT;
  this->rotateX = 0;
  this->rotateY = 0;

  view = XMMatrixLookAtLH(position, forward, up);
  proj = XMMatrixPerspectiveFovLH(XM_PIDIV2, Window::WIDTH / (FLOAT)Window::HEIGHT, 0.01f, 1025.0f);
}

void Camera::update(float t) {
  if (isViewDirty) {
    forward = XMVector3Normalize(forward);
    up = XMVector3Normalize(XMVector3Cross(forward, right));
    right = XMVector3Cross(up, forward);

    // View matrix
    float x = -XMVectorGetX(XMVector3Dot(position, right));
    float y = -XMVectorGetX(XMVector3Dot(position, up));
    float z = -XMVectorGetX(XMVector3Dot(position, forward));

    view.r[0] = { right.m128_f32[0], up.m128_f32[0], forward.m128_f32[0], 0 };
    view.r[1] = { right.m128_f32[1], up.m128_f32[1], forward.m128_f32[1], 0 };
    view.r[2] = { right.m128_f32[2], up.m128_f32[2], forward.m128_f32[2], 0 };
    view.r[3] = { x, y, z, 1.0f };

    isViewDirty = false;
  }
}

void Camera::moveRight(float value) {
  position += right * value;

  isViewDirty = true;
}
void Camera::moveForward(float value) {
  position += forward * value;

  isViewDirty = true;
}

void Camera::addPitch(float value) {
  XMMATRIX rotation = XMMatrixRotationAxis(right, value);

  up = XMVector3TransformNormal(up, rotation);
  forward = XMVector3TransformNormal(forward, rotation);

  isViewDirty = true;
}
void Camera::addYaw(float value) {
  XMMATRIX rotation = XMMatrixRotationAxis(DEFAULT_UP, value);

  up = XMVector3TransformNormal(up, rotation);
  forward = XMVector3TransformNormal(forward, rotation);
  right = XMVector3TransformNormal(right, rotation);

  isViewDirty = true;
}