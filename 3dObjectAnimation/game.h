#pragma once
#include "directx.h"
#include "geometry.h"
#include "camera.h"
#include "player.h"
#include "utils.h"

struct Camera;
struct Geometry;

struct Game : public Directx {
  Game() {};

  void init();
  void run();
  void update(float t);
  void render(float t);
  void input(float t);

  virtual void onMouseDown(WPARAM btnState, int x, int y) override;
  virtual void onMouseUp(WPARAM btnState, int x, int y) override;
  virtual void onMouseMove(WPARAM btnState, int x, int y) override;

  Camera *camera;
  FbxManager *fbxManager;
  GeometryManager *geometryManager;
  Player *player;

  // Input
  // Mouse
  POINT prevMousePos;
};
