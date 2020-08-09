#pragma once
#include "includes.h"
#include "camera.h"
#include "geometry.h"
#include "directx.h"

enum MoveDirection {
  FORWARD,
  BACKWARD,
  LEFT,
  RIGHT
};

struct Player {
  Camera *camera;
  Mesh *mesh;
  std::unordered_map<std::string, AnimationData *> animationsData;
  std::string currentAnimationName;
  ID3D11Buffer *constantBuffer;
  MoveDirection prevMoveDirection;

  Player(Directx *dx) {
    pos = { 0, 0, 0 };
    forward = { 0, 0, 1 };
    right = { 1, 0, 0 };
    up = { 0, 1, 0 };

    // Camera
    camera = new Camera({ 0.0f, 10.0f, 30.0f }, { 0, 0, -1 });

    // Constant buffer
    ConstantBuffer cb = {};
    cb.world = XMMatrixTranspose(camera->world);
    cb.view = XMMatrixTranspose(camera->view);
    cb.proj = XMMatrixTranspose(camera->proj);
    cb.worldViewProj = XMMatrixTranspose(camera->world * camera->view * camera->proj);
    cb.eye = { camera->pos.m128_f32[0], camera->pos.m128_f32[1], camera->pos.m128_f32[2] };

    constantBuffer = dx->createBufferInstance(cb);
  }

  XMVECTOR pos;
  XMVECTOR forward;
  XMVECTOR right;
  XMVECTOR up;

  void setMeshData(Mesh *mesh) {
    this->mesh = mesh;
  }

  void addAnimationData(const std::string &animationName, AnimationData *animationData) {
    animationsData[animationName] = animationData;
  }

  AnimationData *getCurrentAnimation() {
    return animationsData[currentAnimationName];
  }

  ID3D11Buffer *getConstantBuffer() {
    return constantBuffer;
  }

  void update(Directx *dx, float t);
  void initCamera(); // NOTE: calculate pos of camera (camera like in The Witcher 3)
  void move(MoveDirection moveDirection, float value);
};