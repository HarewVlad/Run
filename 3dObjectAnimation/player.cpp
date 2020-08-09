#include "player.h"

void Player::update(float t) {
  animationsData[currentAnimationName]->update(currentAnimationName, t);
  // update Constant buffers
  // update Player position
  // update Camera
  // update Animation
}

void Player::initCamera() {
  static bool isInited = false;
  if (isInited) {
    return;
  }
}

void Player::move(MoveDirection moveDirection, float value) {
  switch (moveDirection) {
  case FORWARD:
    pos += forward * value;
    currentAnimationName = "run.fbx";
    break;
  case BACKWARD:
    pos -= forward * value;
    currentAnimationName = "run.fbx";
    break;
  case LEFT:
    pos += right * value;
    currentAnimationName = "run.fbx";
    break;
  case RIGHT:
    pos -= right * value;
    currentAnimationName = "run.fbx";
    break;
  default:
    OutputDebugStringA("Unsupported player direction movement!");
    break;
  }
}