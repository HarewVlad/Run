#pragma once
#include "includes.h"
#include "utils.h"

struct Camera {
  XMVECTOR pos;
  XMVECTOR target;
  XMVECTOR forward;
  XMVECTOR up;
  XMVECTOR right;

  XMMATRIX world;
  XMMATRIX view;
  XMMATRIX proj;

  float rotateX;
  float rotateY;

  XMVECTOR defaultForward;
  XMVECTOR defaultUp;
  XMVECTOR defaultRight;

  Camera(const XMVECTOR &pos, const XMVECTOR &target);
  void update(float t);
  void moveRight(float value);
  void moveForward(float value);
};