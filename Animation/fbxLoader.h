#pragma once
#include "includes.h"
#include "geometry.h"

struct BoneIndexWeight {
  bool operator < (const BoneIndexWeight &rhs) {
    return boneWeight > rhs.boneWeight;
  }

  int boneIndex;
  float boneWeight;
};

struct Keyframe {
  bool operator == (const Keyframe& key)
  {
    if (t.x != key.t.x || t.y != key.t.y || t.z != key.t.z)
      return false;

    if (s.x != key.s.x || s.y != key.s.y || s.z != key.s.z)
      return false;

    if (r.x != key.r.x || r.y != key.r.y || r.z != key.r.z || r.w != key.r.w)
      return false;

    return true;
  }

  XMFLOAT3 t;
  XMFLOAT3 s;
  XMFLOAT4 r;
  float time;
};

struct BoneAnimation {
  void Interpolate(float t, XMFLOAT4X4 &m) {
    if (t <= keyframes.front().time)
    {
      XMVECTOR S = XMLoadFloat3(&keyframes.front().s);
      XMVECTOR P = XMLoadFloat3(&keyframes.front().t);
      XMVECTOR Q = XMLoadFloat4(&keyframes.front().r);

      XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
      XMStoreFloat4x4(&m, XMMatrixAffineTransformation(S, zero, Q, P));
    }
    else if (t >= keyframes.back().time)
    {
      XMVECTOR S = XMLoadFloat3(&keyframes.back().s);
      XMVECTOR P = XMLoadFloat3(&keyframes.back().t);
      XMVECTOR Q = XMLoadFloat4(&keyframes.back().r);

      XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
      XMStoreFloat4x4(&m, XMMatrixAffineTransformation(S, zero, Q, P));
    }
    else
    {
      for (UINT i = 0; i < keyframes.size() - 1; ++i)
      {
        if (t >= keyframes[i].time && t <= keyframes[i + 1].time)
        {
          float lerpPercent = (t - keyframes[i].time) / (keyframes[i + 1].time - keyframes[i].time);

          XMVECTOR s0 = XMLoadFloat3(&keyframes[i].s);
          XMVECTOR s1 = XMLoadFloat3(&keyframes[i + 1].s);

          XMVECTOR p0 = XMLoadFloat3(&keyframes[i].t);
          XMVECTOR p1 = XMLoadFloat3(&keyframes[i + 1].t);

          XMVECTOR q0 = XMLoadFloat4(&keyframes[i].r);
          XMVECTOR q1 = XMLoadFloat4(&keyframes[i + 1].r);

          XMVECTOR S = XMVectorLerp(s0, s1, lerpPercent);
          XMVECTOR P = XMVectorLerp(p0, p1, lerpPercent);
          XMVECTOR Q = XMQuaternionSlerp(q0, q1, lerpPercent);

          XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
          XMStoreFloat4x4(&m, XMMatrixAffineTransformation(S, zero, Q, P));

          break;
        }
      }
    }
  }

  float getEndTime() {
    float f = keyframes.back().time;
    return f;
  }

  std::vector<Keyframe> keyframes;
};

struct ControlPoint {
  ControlPoint() {
    boneInfo.reserve(4);
  }

  void sortBlendingInfoByWeight() {
    std::sort(boneInfo.begin(), boneInfo.end());
  }

  std::vector<BoneIndexWeight> boneInfo;
  std::string boneName;
  XMFLOAT3 position;
};

struct Animation {
  void interpolate(float t, std::vector<XMFLOAT4X4> &boneTransforms) {
    for (int i = 0; i < boneAnimations.size(); i++) {
      boneAnimations[i].Interpolate(t, boneTransforms[i]);
    }
  }

  float getEndTime() {
    float t = 0.0f;
    for (int i = 0; i < boneAnimations.size(); i++) {
      t = max(t, boneAnimations[i].getEndTime());
    }
    return t;
  
  }
  void update(float t) {
    timePosition += t;
    if (timePosition > getEndTime()) {
      timePosition = 0.0f;
    }

    getFinalTransforms(timePosition);
  }

  void getFinalTransforms(float t);

  std::vector<int> boneHierarchy;
  std::vector<XMFLOAT4X4> boneOffsets;
  std::vector<XMFLOAT4X4> finalTransforms;
  std::vector<BoneAnimation> boneAnimations;

  float timePosition;
};

struct Model {
  Animation *animation;
  MeshDynamic *mesh;
};

struct FbxLoader {
  FbxLoader();
  Model loadFBXModel(Directx *dx, const std::string &name);
  Animation *loadFBXModelAnimation(Directx *dx, const std::string &name);

  int findCurrentJointIndexByName(const std::string &jointName, const std::vector<std::string> &boneNames);

  void getSkeleton(FbxNode *node, int currentIndex, int parentIndex,
    std::vector<int> &boneHierarhcy,
    std::vector<std::string> &boneNames);
  void getControlPoints(FbxNode *node, std::unordered_map<int, ControlPoint> &controlPoints,
    const std::vector<std::string> &boneNames);
  void getBoneOffsets(FbxScene *scene, FbxNode *node, std::vector<XMFLOAT4X4> &boneOffsets,
    const std::vector<std::string> &boneNames);
  void getAnimation(FbxScene *scene, FbxNode *node,
    std::unordered_map<int, ControlPoint> &controlPoitns,
    std::vector<BoneAnimation> &boneAnimations,
    const std::vector<std::string> &boneNames,
    const std::string &name);
  void getVertexInfo(FbxNode *node, std::vector<VertexDynamic> &vertices, 
    std::unordered_map<int, ControlPoint> &controlPoints);

  FbxManager *manager;

  //std::vector<int> boneHierarchy;
  //std::vector<std::string> boneNames;
  //std::vector<XMFLOAT4X4> boneOffsets;
  //std::vector<BoneAnimation> boneAnimations;
  //std::vector<XMFLOAT4X4> finalTransforms;
};