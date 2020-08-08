#include "camera.h"

Camera::Camera(const XMVECTOR &pos, const XMVECTOR &forward) {
  this->pos = pos;
  this->forward = forward;
  this->up = DEFAULT_UP;
  this->right = DEFAULT_RIGHT;
  this->rotateX = 0;
  this->rotateY = 0;

  world = XMMatrixIdentity();
  view = XMMatrixLookAtLH(pos, forward, up);
  proj = XMMatrixPerspectiveFovLH(XM_PIDIV2, Window::WIDTH / (FLOAT)Window::HEIGHT, 0.01f, 1025.0f);
}

void Camera::update(float t) {
  XMMATRIX rotationMatrix = XMMatrixRotationX(rotateY) * XMMatrixRotationY(rotateX);

  forward = XMVector3Normalize(XMVector3TransformCoord(DEFAULT_FORWARD, rotationMatrix));
  right = XMVector3Normalize(XMVector3TransformCoord(DEFAULT_RIGHT, rotationMatrix));
  up = XMVector3Cross(forward, right);

  forward += pos;

  view = XMMatrixLookAtLH(pos, forward, up);
}

void Camera::moveRight(float value) {
  pos += right * value;
}
void Camera::moveForward(float value) {
  pos += forward * value;
}