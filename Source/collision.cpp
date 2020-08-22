#include "collision.h"

internal const float MAX = 99999999;
internal const float MIN = -99999999;

// TODO: make it clean
void CollisionManager::createBBForObject(const std::string &name, const std::vector<VertexStatic> &vertices) {
  XMVECTOR min = { FLT_MAX, FLT_MAX, FLT_MAX };
  XMVECTOR max = { FLT_MIN, FLT_MIN, FLT_MIN};

  for (size_t i = 0; i < vertices.size(); ++i) {
    XMFLOAT3 pos = vertices[i].pos;

    if (min.m128_f32[0] > pos.x) { min.m128_f32[0] = pos.x; }
    if (min.m128_f32[1] > pos.y) { min.m128_f32[1] = pos.y; }
    if (min.m128_f32[2] > pos.z) { min.m128_f32[2] = pos.z; }

    if (max.m128_f32[0] < pos.x) { max.m128_f32[0] = pos.x; }
    if (max.m128_f32[1] < pos.y) { max.m128_f32[1] = pos.y; }
    if (max.m128_f32[2] < pos.z) { max.m128_f32[2] = pos.z; }
  }

  boundingBoxes[name] = new BoundingBox{ min, max };
}

BoundingBox *CollisionManager::createBBForPlayer(const std::vector<VertexDynamic> &vertices) {
  XMVECTOR min = { FLT_MAX, FLT_MAX, FLT_MAX };
  XMVECTOR max = { FLT_MIN, FLT_MIN, FLT_MIN };

  for (size_t i = 0; i < vertices.size(); ++i) {
    XMFLOAT3 pos = vertices[i].pos;

    if (min.m128_f32[0] > pos.x) { min.m128_f32[0] = pos.x; }
    if (min.m128_f32[1] > pos.y) { min.m128_f32[1] = pos.y; }
    if (min.m128_f32[2] > pos.z) { min.m128_f32[2] = pos.z; }

    if (max.m128_f32[0] < pos.x) { max.m128_f32[0] = pos.x; }
    if (max.m128_f32[1] < pos.y) { max.m128_f32[1] = pos.y; }
    if (max.m128_f32[2] < pos.z) { max.m128_f32[2] = pos.z; }
  }

  return new BoundingBox{ min, max };
}

// TODO(vlad): objects is simple geometry i guess
bool CollisionManager::testAABBAABB(const BoundingBox *player) {
  for (auto it = boundingBoxes.begin(); it != boundingBoxes.end(); ++it) {
    XMVECTOR min = it->second->min;
    XMVECTOR max = it->second->max;

    if (player->max.m128_f32[0] < min.m128_f32[0] || player->min.m128_f32[0] > max.m128_f32[0]) return false;
    if (player->max.m128_f32[1] < min.m128_f32[1] || player->min.m128_f32[1] > max.m128_f32[1]) return false;
    if (player->max.m128_f32[2] < min.m128_f32[2] || player->min.m128_f32[2] > max.m128_f32[2]) return false;

    return true;
   }
  return false;
}

internal float lerp(float a, float b, float t) {
  return a + t * (b - a);
}

bool CollisionManager::testAABBHeightmap(const BoundingBox *box, int gridSize) {
  return false;
}