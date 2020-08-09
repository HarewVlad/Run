#pragma once
#include "includes.h"
#include "camera.h"
#include "geometry.h"

enum MoveDirection {
  FORWARD,
  BACKWARD,
  LEFT,
  RIGHT
};

struct Player {
  Camera *cam;
  Mesh *mesh;
  std::unordered_map<std::string, AnimationData *> animationsData;
  std::string currentAnimationName;

  Player() {
    pos = { 0, 0, 0 };
    forward = { 0, 0, 1 };
    right = { 1, 0, 0 };
    up = { 0, 1, 0 };
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

  void update(float t);
  void initCamera(); // NOTE: calculate pos of camera (camera like in The Witcher 3)
  void move(MoveDirection moveDirection, float value);
};