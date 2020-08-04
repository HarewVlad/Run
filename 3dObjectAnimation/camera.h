#include "includes.h"

const XMVECTOR DEFAULT_UP = { 0, 1, 0 };
const XMVECTOR DEFAULT_FORWARD = { 0, 0, 1 };
const XMVECTOR DEFAULT_RIGHT = {1, 0, 0 }; // NOTE: inverted for better mouse experience

struct Camera {
  XMVECTOR pos;
  XMVECTOR target;
  XMVECTOR up;
  XMVECTOR right;
  XMVECTOR forward;

  float moveRight;
  float moveForward;

  XMMATRIX world;
  XMMATRIX view;
  XMMATRIX proj;

  void init(XMVECTOR pos, XMVECTOR target);
  void update(const POINT &prevMousePos, const POINT &currMousePos, float t);
};