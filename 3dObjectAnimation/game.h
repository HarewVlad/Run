#pragma once
#include "directx.h"
#include "geometry.h"
#include "camera.h"

struct Camera;
struct Geometry;

struct Game {
  Directx *dx;
  Camera *camera;
  FbxManager *fbxManager;
  GeometryManager *geometryManager;

  // Input
  // Mouse
  POINT currMousePos;
  POINT prevMousePos;
  
  void init();
  void run();
  void update(float t);
  void render(float t);
  void input(float t);
};
