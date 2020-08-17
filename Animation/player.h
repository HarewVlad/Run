#pragma once
#include "includes.h"
#include "constantBuffers.h"
#include "camera.h"
#include "fbxLoader.h"

enum PlayerState {
  IDLE,
  RUN
};

enum MoveDirection {
  FORWARD,
  BACKWARD,
  LEFT,
  RIGHT
};

struct Player {
  Player(ID3D11Device *device, Camera *camera, const XMVECTOR &position);

  void update(ID3D11DeviceContext *deviceContext, Camera *camera, float t);
  void move(MoveDirection direction, float value);

  void addAnimation(PlayerState state, Animation *animation) {
    animations[state] = animation;
  }

  void setMesh(MeshDynamic *mesh) {
    this->mesh = mesh;
  }
  
  MeshDynamic *mesh;
  std::unordered_map<PlayerState, Animation *> animations;
  PlayerState currentState;

  XMMATRIX world;

  XMVECTOR position;
  XMVECTOR forward;
  XMVECTOR right;
  XMVECTOR up;

  ID3D11Buffer *constantBuffer;
};