#pragma once
#include "includes.h"
#include "constantBuffers.h"
#include "camera.h"
#include "fbxLoader.h"
#include "collision.h"

enum PlayerState {
  IDLE,
  RUN
};

enum class MoveDirection {
  NONE,
  FORWARD,
  BACKWARD,
  LEFT,
  RIGHT
};

struct Player {
  Player(ID3D11Device *device, Camera *camera, const XMVECTOR &position);

  void update(ID3D11DeviceContext *deviceContext, Camera *camera, float t);
  void move(MoveDirection direction, Camera *camera, float value);
  void rotate(float value);

  void addAnimation(PlayerState state, Animation *animation) {
    animations[state] = animation;
  }

  void setMesh(MeshDynamic *mesh) {
    this->mesh = mesh;
  }
  
  // Data
  MeshDynamic *mesh;
  std::unordered_map<PlayerState, Animation *> animations;
  PlayerState currentState;
  MoveDirection currentDirection;

  XMMATRIX world;

  // Orientations
  XMVECTOR position;
  XMVECTOR forward;
  XMVECTOR right;
  XMVECTOR up;

  // Buffers
  ID3D11Buffer *constantBuffer;

  // Collision
  BoundingBox *boundingBox;
};