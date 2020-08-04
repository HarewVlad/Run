#pragma once
#include "directx.h"

struct Keyframe {
  XMFLOAT3 t;
  XMFLOAT3 s;
  XMFLOAT4 r;
  float time;

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
};

struct BoneAnimation {
  std::vector<Keyframe> keyframes;

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
};

struct AnimationClip {
  std::vector<BoneAnimation> boneAnimations;

  void interpolate(float t, std::vector<XMFLOAT4X4> &boneTransforms) {
    for (int i = 0; i < boneAnimations.size(); i++) {
      boneAnimations[i].Interpolate(t, boneTransforms[i]);
    }
  }
};

struct SkinnedData {
  std::vector<std::string> boneName;
  std::vector<int> boneHierarchy;
  std::vector<XMFLOAT4X4> boneOffsets;
  std::vector<std::string> animationName;
  std::unordered_map<std::string, AnimationClip> animations;

  void getFinalTransforms(float dt, std::vector<XMFLOAT4X4> &finalTransforms) {
    int numBones = boneOffsets.size();

    std::vector<XMFLOAT4X4> toParentTransforms(numBones);
    
    auto clip = animations["MAIN"];
    clip.interpolate(dt, toParentTransforms);

    std::vector<XMFLOAT4X4> toRootTransforms(numBones);

    toRootTransforms[0] = toParentTransforms[0];

    for (int i = 1; i < numBones; i++) {
      XMMATRIX toParent = XMLoadFloat4x4(&toParentTransforms[i]);
      
      int parentIndex = boneHierarchy[i];
      XMMATRIX parentToRoot = XMLoadFloat4x4(&toRootTransforms[parentIndex]);

      XMMATRIX toRoot = XMMatrixMultiply(toParent, parentToRoot);

      XMStoreFloat4x4(&toRootTransforms[i], toRoot);
    }
    
    finalTransforms.resize(numBones);
    for (int i = 0; i < numBones; i++) {
      XMMATRIX offset = XMLoadFloat4x4(&boneOffsets[i]);
      XMMATRIX toRoot = XMLoadFloat4x4(&toParentTransforms[i]);
      XMMATRIX finalTransform = XMMatrixMultiply(offset, toRoot);
      finalTransform *= XMMatrixScaling(0.1f, 0.1f, 0.1f);

      XMStoreFloat4x4(&finalTransforms[i], XMMatrixTranspose(finalTransform));
    }
  }
};

struct SkinnedModelInstance {
  SkinnedData skinnedInfo;
  std::vector<XMFLOAT4X4> finalTransforms;
  float timePos = 0.0f;

  void update(float dt) {
    timePos += dt;

    skinnedInfo.getFinalTransforms(timePos, finalTransforms);
  }
};

struct BoneIndexWeight {
  int boneIndex;
  float boneWeight;

  bool operator < (const BoneIndexWeight &rhs) {
    return boneWeight > rhs.boneWeight;
  }
};

struct ControlPoint {
  ControlPoint() {
    boneInfo.reserve(4);
  }

  void sortBlendingInfoByWeight() {
    std::sort(boneInfo.begin(), boneInfo.end());
  }

  XMFLOAT3 pos;
  std::vector<BoneIndexWeight> boneInfo;
  std::string boneName;
};

struct Geometry {
  ID3D11Buffer *vertexBuffer;
  ID3D11Buffer *indexBuffer;
  std::vector<int> indices;
  std::vector<Vertex> vertices;

  void createBox3D(Directx *dx, float width, float height, float depth);
  void createBox2D(Directx *dx, float x, float y, float w, float h);
  void createFBXModel(Directx *dx, FbxManager *manager, const std::string &path);
  

  // Animation data
  void getSkeletonHierachy(FbxNode *node, int currentIndex, int parentIndex, SkinnedData &skinnedData);
  void getAnimation(FbxScene *scene, FbxNode *node, SkinnedData &skinnedData);
  void getControlPoints(FbxMesh *mesh);
  void getVertexInfo(FbxMesh *mesh);

  SkinnedData skinnedData;
  SkinnedModelInstance modelInstance;
  std::unordered_map<int, ControlPoint *> controlPoints;
};

// TODO: Add correct clearing behaviour
enum class ObjectType {
  STATIC,
  PLAYER
};

struct Objects {
  ObjectType type;

  std::unordered_map<std::string, Geometry *> data;

  Geometry *getGeometry(const std::string &name) {
    return data[name];
  }

  void addGeometry(Geometry *geometry, const std::string &name) {
    data[name] = geometry;
  }
};
