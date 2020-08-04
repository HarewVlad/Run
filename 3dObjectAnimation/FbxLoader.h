// Credits to YujinJung https://github.com/YujinJung/FBX-Loader

#pragma once
#include "includes.h"
#include "directx.h"
#include "geometry.h"

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
  std::unordered_map<std::string, AnimationClip> animations;

  void getFinalTransformsForAnimation(const std::string &animationName, float dt, std::vector<XMFLOAT4X4> &finalTransforms) {
    int numBones = boneOffsets.size();

    std::vector<XMFLOAT4X4> toParentTransforms(numBones);

    auto clip = animations[animationName];
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
      finalTransform *= XMMatrixScaling(0.01f, 0.01f, 0.01f);

      XMStoreFloat4x4(&finalTransforms[i], XMMatrixTranspose(finalTransform));
    }
  }
};

struct SkinnedModelInstance {
  SkinnedData skinnedData;
  std::string animationName;
  std::vector<XMFLOAT4X4> finalTransforms;
  float timePos = 0.0f;

  void update(float dt) {
    timePos += dt;

    skinnedData.getFinalTransformsForAnimation(animationName, timePos, finalTransforms);
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
  XMFLOAT3 pos;
  std::vector<BoneIndexWeight> boneInfo;
  std::string boneName;

  ControlPoint() {
    boneInfo.reserve(4);
  }

  void sortBlendingInfoByWeight() {
    std::sort(boneInfo.begin(), boneInfo.end());
  }
};

struct FbxLoader {
  FbxManager *manager;
  SkinnedData skinnedData;
  std::unordered_map<int, ControlPoint *> controlPoints;
  std::unordered_map<std::string, SkinnedModelInstance> modelInstances;

  FbxLoader() {
    // Init fbxManager
    manager = FbxManager::Create();
    FbxIOSettings *ioSettings = FbxIOSettings::Create(manager, IOSROOT);
    manager->SetIOSettings(ioSettings);
  }

  void getControlPoints(FbxMesh *mesh) {
    int controlPointsCount = mesh->GetControlPointsCount();
    for (int i = 0; i < controlPointsCount; i++) {
      ControlPoint *controlPoint = new ControlPoint();

      XMFLOAT3 pos;
      pos.x = static_cast<float>(mesh->GetControlPointAt(i).mData[0]);
      pos.y = static_cast<float>(mesh->GetControlPointAt(i).mData[1]);
      pos.z = static_cast<float>(mesh->GetControlPointAt(i).mData[2]);

      controlPoint->pos = pos;
      controlPoints[i] = controlPoint;
    }
  }

  void getSkeletonHierachy(FbxNode *node, int currentIndex, int parentIndex, SkinnedData &skinnedData) {
    skinnedData.boneHierarchy.push_back(parentIndex);
    skinnedData.boneName.push_back(node->GetName());

    for (int i = 0; i < node->GetChildCount(); i++) {
      getSkeletonHierachy(node->GetChild(i), skinnedData.boneHierarchy.size(), currentIndex, skinnedData);
    }
  }

  void getAnimation(const std::string &animationName, FbxScene *scene, FbxNode *node, SkinnedData &skinnedData) {
    FbxMesh *mesh = (FbxMesh *)node->GetNodeAttribute();

    const FbxVector4 t = node->GetGeometricTranslation(FbxNode::eSourcePivot);
    const FbxVector4 r = node->GetGeometricRotation(FbxNode::eSourcePivot);
    const FbxVector4 s = node->GetGeometricScaling(FbxNode::eSourcePivot);
    FbxAMatrix geometryTransform = FbxAMatrix(t, r, s);

    AnimationClip animation;
    animation.boneAnimations.resize(skinnedData.boneName.size());

    for (int deformerIndex = 0; deformerIndex < mesh->GetDeformerCount(); deformerIndex++) {
      FbxSkin *skin = reinterpret_cast<FbxSkin *>(mesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));
      if (!skin) { continue; }

      for (int clusterIndex = 0; clusterIndex < skin->GetClusterCount(); clusterIndex++) {
        FbxCluster *cluster = skin->GetCluster(clusterIndex);

        std::string currentJointName = cluster->GetLink()->GetName();
        int currentJointIndex;
        for (currentJointIndex = 0; currentJointIndex < skinnedData.boneName.size(); currentJointIndex++) {
          if (skinnedData.boneName[currentJointIndex] == currentJointName) {
            break;
          }
        }

        FbxAMatrix transformMatrix, transformLinkMatrix;
        FbxAMatrix globalBindposeInverseMatrix;

        transformMatrix = cluster->GetTransformMatrix(transformMatrix);
        transformLinkMatrix = cluster->GetTransformLinkMatrix(transformLinkMatrix);
        globalBindposeInverseMatrix = transformLinkMatrix.Inverse() * transformMatrix * geometryTransform;

        XMFLOAT4X4 boneOffset;
        for (int i = 0; i < 4; i++) {
          for (int j = 0; j < 4; j++) {
            boneOffset.m[i][j] = globalBindposeInverseMatrix.Get(i, j);
          }
        }

        skinnedData.boneOffsets[currentJointIndex] = boneOffset;

        int *controlPointIndices = cluster->GetControlPointIndices();
        for (int i = 0; i < cluster->GetControlPointIndicesCount(); i++) {
          BoneIndexWeight boneIndexWeight;
          boneIndexWeight.boneIndex = currentJointIndex;
          boneIndexWeight.boneWeight = cluster->GetControlPointWeights()[i];

          controlPoints[controlPointIndices[i]]->boneInfo.push_back(boneIndexWeight);
          controlPoints[controlPointIndices[i]]->boneName = currentJointName;
        }

        BoneAnimation boneAnimation;
        FbxAnimStack *animStack = scene->GetSrcObject<FbxAnimStack>(0);
        FbxAnimEvaluator *sceneEvaluator = scene->GetAnimationEvaluator();

        FbxLongLong index;
        for (index = 0; index < 100; index++) {
          FbxTime currTime;
          currTime.SetFrame(index, FbxTime::eCustom);

          Keyframe key;
          key.time = static_cast<float>(index) / 8.0f;

          FbxAMatrix currentTransformOffset = sceneEvaluator->GetNodeGlobalTransform(node, currTime) * geometryTransform;
          FbxAMatrix temp = currentTransformOffset.Inverse() * sceneEvaluator->GetNodeGlobalTransform(cluster->GetLink(), currTime);

          FbxVector4 t = temp.GetT();
          key.t = { (float)t.mData[0], (float)t.mData[1], (float)t.mData[2] };
          t = temp.GetS();
          key.s = { (float)t.mData[0], (float)t.mData[1], (float)t.mData[2] };
          FbxQuaternion q = temp.GetQ();
          key.r = { (float)q.mData[0], (float)q.mData[1], (float)q.mData[2], (float)q.mData[3] };

          if (index != 0 && boneAnimation.keyframes.back() == key) break;

          boneAnimation.keyframes.push_back(key);
        }
        animation.boneAnimations[currentJointIndex] = boneAnimation;
      }
    }

    BoneAnimation initBoneAnimation;

    // Initialize InitBoneAnim
    for (int i = 0; i < skinnedData.boneName.size(); ++i)
    {
      int KeyframeSize = animation.boneAnimations[i].keyframes.size();
      if (KeyframeSize != 0)
      {
        for (int j = 0; j < KeyframeSize; ++j) // 60 frames
        {
          Keyframe key;

          key.time = static_cast<float>(j / 24.0f);
          key.t = { 0.0f, 0.0f, 0.0f };
          key.s = { 1.0f, 1.0f, 1.0f };
          key.r = { 0.0f, 0.0f, 0.0f, 0.0f };
          initBoneAnimation.keyframes.push_back(key);
        }
        break;
      }
    }

    for (int i = 0; i < skinnedData.boneName.size(); ++i)
    {
      if (animation.boneAnimations[i].keyframes.size() != 0)
        continue;

      animation.boneAnimations[i] = initBoneAnimation;
    }

    BoneIndexWeight currBoneIndexAndWeight;
    currBoneIndexAndWeight.boneIndex = 0;
    currBoneIndexAndWeight.boneWeight = 0;
    for (auto itr = controlPoints.begin(); itr != controlPoints.end(); ++itr)
    {
      for (uint32_t i = itr->second->boneInfo.size(); i <= 4; ++i)
      {
        itr->second->boneInfo.push_back(currBoneIndexAndWeight);
      }
    }

    skinnedData.animations[animationName] = animation;
  }

  void getVertexInfo(FbxMesh *mesh) {
    uint32_t count = mesh->GetPolygonCount();
    for (int i = 0; i < count; i++) {
      std::string currBoneName = controlPoints[mesh->GetPolygonVertex(i, 1)]->boneName;

      for (int j = 0; j < 3; j++) {
        int controlPointIndex = mesh->GetPolygonVertex(i, j);
        ControlPoint *controlPoint = controlPoints[controlPointIndex];

        FbxVector4 normal;
        mesh->GetPolygonVertexNormal(i, j, normal);

        FbxStringList uvNames;
        mesh->GetUVSetNames(uvNames);
        const char *uvName = nullptr;
        if (uvNames.GetCount()) {
          uvName = uvNames[0];
        }

        FbxVector2 uvs;
        bool unmappedUV;
        if (!mesh->GetPolygonVertexUV(i, j, uvName, uvs, unmappedUV)) {
          MessageBox(0, "Error", 0, 0);
        }

        Vertex temp;
        temp.pos.x = controlPoint->pos.x;
        temp.pos.y = controlPoint->pos.y;
        temp.pos.z = controlPoint->pos.z;

        temp.normal.x = static_cast<float>(normal.mData[0]);
        temp.normal.y = static_cast<float>(normal.mData[1]);
        temp.normal.z = static_cast<float>(normal.mData[2]);

        temp.tex.x = static_cast<float>(uvs.mData[0]);
        temp.tex.y = static_cast<float>(1.0f - uvs.mData[1]);

        // TODO: sort by weights
        controlPoint->sortBlendingInfoByWeight();

        for (int k = 0; k < controlPoint->boneInfo.size(); k++) {
          if (k >= 4) {
            break;
          }

          temp.boneIndices[k] = controlPoint->boneInfo[k].boneIndex;

          switch (k) {
          case 0:
            temp.boneWeights.x = controlPoint->boneInfo[k].boneWeight;
            break;
          case 1:
            temp.boneWeights.y = controlPoint->boneInfo[k].boneWeight;
            break;
          case 2:
            temp.boneWeights.z = controlPoint->boneInfo[k].boneWeight;
            break;
          }
        }

        vertices.push_back(temp);
      }
    }
  }

  SkinnedModelInstance loadModel(Directx *dx, const std::string &fileName) {
    // Get animation name
    int index = fileName.find('.');
    std::string animationName = fileName;
    animationName.erase(animationName.begin() + index, animationName.end());

    FbxImporter* fbxImporter = FbxImporter::Create(manager, "");
    FbxScene* fbxScene = FbxScene::Create(manager, "");

    DX::ThrowIfFailed(fbxImporter->Initialize(fileName.c_str(), -1, manager->GetIOSettings()));
    DX::ThrowIfFailed(fbxImporter->Import(fbxScene));

    fbxImporter->Destroy();

    FbxNode *rootNode = fbxScene->GetRootNode();
    if (!rootNode) {
      return;
    }

    FbxGeometryConverter geometryConverter(manager);
    geometryConverter.Triangulate(fbxScene, true);

    // Skeleton
    for (int i = 0; i < rootNode->GetChildCount(); i++) {
      FbxNode *childNode = rootNode->GetChild(i);
      FbxMesh *mesh = (FbxMesh *)childNode->GetNodeAttribute();

      FbxNodeAttribute::EType attributeType = mesh->GetAttributeType();
      if (!attributeType) {
        continue;
      }

      switch (attributeType) {
      case FbxNodeAttribute::eSkeleton:
        getSkeletonHierachy(childNode, 0, -1, skinnedData);
        break;
      }
    }

    const int childCount = rootNode->GetChildCount();
    for (int i = 0; i < childCount; i++) {
      FbxNode *childNode = rootNode->GetChild(i);

      if (childNode->GetNodeAttribute() == NULL) {
        continue;
      }

      FbxNodeAttribute::EType attributeType = childNode->GetNodeAttribute()->GetAttributeType();
      if (attributeType == FbxNodeAttribute::eMesh) {
        FbxMesh *mesh = (FbxMesh *)childNode->GetNodeAttribute();

        getControlPoints(mesh);

        skinnedData.boneOffsets.resize(skinnedData.boneHierarchy.size());

        getAnimation(animationName, fbxScene, childNode, skinnedData);

        getVertexInfo(mesh);
      }

      modelInstance.skinnedInfo = skinnedData;
      modelInstance.timePos = 0;
    }

    ID3D11Buffer *vertexBuffer = nullptr;

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(Vertex) * vertices.size();
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = &vertices[0];

    DX::ThrowIfFailed(dx->device->CreateBuffer(&bd, &initData, &vertexBuffer));

    // TODO: free resources

    // Return
    Geometry *geometry = new Geometry();
    geometry->vertexBuffer = vertexBuffer;
    // TODO: fill the fields
  }

  SkinnedModelInstance getModelInstance(const std::string &modelInstanceName) {
    return modelInstances[modelInstanceName];
  }
};