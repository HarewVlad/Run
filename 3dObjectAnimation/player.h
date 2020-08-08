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
  XMFLOAT3 pos;
  std::unordered_map<std::string, AnimationData *> animationsData;
  std::string currentAnimationName;

  void setMeshData(Mesh *mesh) {
    this->mesh = mesh;
  }

  void addAnimationData(const std::string &animationName, AnimationData *animationData) {
    animationsData[animationName] = animationData;
  }

  void update(float t);
  void initCamera(); // NOTE: calculate pos of camera (camera like in The Witcher 3)
  void move(MoveDirection moveDirection);
};