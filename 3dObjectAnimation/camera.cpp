#include "camera.h"

void Camera::init(XMVECTOR pos, XMVECTOR target) {
  this->pos = pos;
  this->target = target;
  this->up = DEFAULT_UP;
  this->forward = DEFAULT_FORWARD;
  this->right = DEFAULT_RIGHT;
  this->moveForward = 0;
  this->moveRight = 0;

  world = XMMatrixIdentity();
  view = XMMatrixLookAtLH(pos, target, up);
  proj = XMMatrixPerspectiveFovLH(XM_PIDIV2, Window::WIDTH / (FLOAT)Window::HEIGHT, 0.01f, 1025.0f);
}

void Camera::update(const POINT &prevMousePos, const POINT &currMousePos, float t) {
  float dx = XMConvertToRadians(
    0.25f*static_cast<float>(prevMousePos.x - currMousePos.x));
  float dy = XMConvertToRadians(
    0.25f*static_cast<float>(prevMousePos.y - currMousePos.y));

  static float totalDx = 0;
  static float totalDy = 0;
  totalDx += dx;
  totalDy += dy;

  XMMATRIX rotateX = -XMMatrixRotationX(totalDy);
  XMMATRIX rotateY = XMMatrixRotationY(totalDx);
  XMMATRIX camRotaionMatrix = rotateX * rotateY;

  target = XMVector3TransformCoord(DEFAULT_FORWARD, camRotaionMatrix);
  target = XMVector3Normalize(target);

  right = XMVector3TransformCoord(DEFAULT_RIGHT, camRotaionMatrix);
  forward = XMVector3TransformCoord(DEFAULT_FORWARD, camRotaionMatrix);
  up = XMVector3Cross(forward, right);

  pos += moveRight * right;
  pos += moveForward * forward;
  target = pos + target;

  view = XMMatrixLookAtLH(pos, target, up);

  moveRight = 0;
  moveForward = 0;
}