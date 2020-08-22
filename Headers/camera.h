#pragma once
#include "includes.h"

const XMVECTOR DEFAULT_FORWARD = { 0, 0, 1 };
const XMVECTOR DEFAULT_UP = { 0, 1, 0 };
const XMVECTOR DEFAULT_RIGHT = { 1, 0, 0 };

struct Camera {
  Camera();
  Camera(const XMVECTOR &position);
  void update(float t);
  void addPitch(float value);
  void addYaw(float value);
  void moveForward(float value);
  void moveRight(float value);
  void updateFollowTarget(XMVECTOR position, XMVECTOR forward, XMVECTOR right, XMVECTOR up); // TODO(vlad): give cam more freedom

  void attachCamera(XMVECTOR position, XMVECTOR forward, XMVECTOR right, XMVECTOR up);
  void detachCamera();

  XMMATRIX view;
  XMMATRIX proj;

  XMVECTOR position;
  XMVECTOR forward;
  XMVECTOR up;
  XMVECTOR right;

  float rotateX;
  float rotateY;

  bool isViewDirty;
  bool isAttached;
};