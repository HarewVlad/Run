#include "camera.h"

Camera::Camera(const XMVECTOR &pos, const XMVECTOR &target) {
  this->defaultForward = XMVector3Normalize(target - pos);
  this->defaultRight = { -1, 0, 0 };
  this->defaultUp = XMVector3Cross(defaultForward, defaultRight);

  this->pos = pos;
  this->target = target;

  this->forward = defaultForward;
  this->right = defaultRight;
  this->up = defaultUp;

  this->rotateX = 0;
  this->rotateY = 0;

  world = XMMatrixIdentity();
  view = XMMatrixLookAtLH(pos, target, up);
  proj = XMMatrixPerspectiveFovLH(XM_PIDIV2, Window::WIDTH / (FLOAT)Window::HEIGHT, 0.01f, 1025.0f);
}

void Camera::update(float t) {
  XMMATRIX rotationMatrix = XMMatrixRotationX(rotateY) * XMMatrixRotationY(rotateX);

  target = XMVector3Normalize(XMVector3TransformCoord(defaultForward, rotationMatrix));
  forward = XMVector3Normalize(XMVector3TransformCoord(defaultForward, rotationMatrix));
  right = XMVector3Normalize(XMVector3TransformCoord(defaultRight, rotationMatrix));
  up = XMVector3Cross(forward, right);

  target += pos;

  view = XMMatrixLookAtLH(pos, target, up);
}

void Camera::moveRight(float value) {
  pos += right * value;
}
void Camera::moveForward(float value) {
  pos += forward * value;
}