#pragma once
#include "includes.h"
#include "utils.h"

const XMVECTOR DEFAULT_UP = { 0, 1, 0 };
const XMVECTOR DEFAULT_RIGHT = {1, 0, 0 };
const XMVECTOR DEFAULT_FORWARD = { 0, 0, 1 };

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

  Camera(const XMVECTOR &pos, const XMVECTOR &target);
  void update(float t);
  void moveRight(float value);
  void moveForward(float value);
};