#pragma once
#include "includes.h"
#include "vertex.h"

struct BoundingBox {
  XMVECTOR min;
  XMVECTOR max;
};

struct CollisionManager {
  void createBBForObject(const std::string &name, const std::vector<VertexStatic> &vertices);
  BoundingBox *createBBForPlayer(const std::vector<VertexDynamic> &vertices);

  bool testAABBAABB(const BoundingBox *box);
  bool testAABBHeightmap(const BoundingBox *box, int gridSize);

  std::unordered_map<std::string, BoundingBox *> boundingBoxes;
};

