#include "fbxLoader.h"

FbxLoader::FbxLoader() {
  static bool isInited = false;
  if (!isInited) {
    manager = FbxManager::Create();
    FbxIOSettings *fbxIOSettings = FbxIOSettings::Create(manager, IOSROOT);
    manager->SetIOSettings(fbxIOSettings);

    isInited = true;
  }
}

void FbxLoader::getSkeleton(FbxNode *node, int currentIndex, int parentIndex,
  std::vector<int> &boneHierarchy,
  std::vector<std::string> &boneNames) {
  boneHierarchy.push_back(parentIndex);
  boneNames.push_back(node->GetName());

  for (int i = 0; i < node->GetChildCount(); ++i) {
    getSkeleton(node->GetChild(i), boneHierarchy.size(), currentIndex, boneHierarchy, boneNames);
  }
}

int FbxLoader::findCurrentJointIndexByName(const std::string &jointName, const std::vector<std::string> &boneNames) {
  int jointIndex;
  for (jointIndex = 0; jointIndex < boneNames.size(); ++jointIndex) {
    if (boneNames[jointIndex] == jointName) {
      break;
    }
  }

  return jointIndex;
}

void FbxLoader::getBoneOffsets(FbxScene *scene, FbxNode *node, std::vector<XMFLOAT4X4> &boneOffsets, const std::vector<std::string> &boneNames) {
  FbxMesh *mesh = reinterpret_cast<FbxMesh *>(node->GetNodeAttribute());

  const FbxVector4 t = node->GetGeometricTranslation(FbxNode::eSourcePivot);
  const FbxVector4 r = node->GetGeometricRotation(FbxNode::eSourcePivot);
  const FbxVector4 s = node->GetGeometricScaling(FbxNode::eSourcePivot);
  FbxAMatrix geometryTransform = FbxAMatrix(t, r, s);

  for (int deformerIndex = 0; deformerIndex < mesh->GetDeformerCount(); ++deformerIndex) {
    FbxSkin *skin = reinterpret_cast<FbxSkin *>(mesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));

    if (!skin) { continue; };

    for (int clusterIndex = 0; clusterIndex < skin->GetClusterCount(); ++clusterIndex) {
      FbxCluster *cluster = skin->GetCluster(clusterIndex);

      std::string currentJointName = cluster->GetLink()->GetName();
      int currentJointIndex = findCurrentJointIndexByName(currentJointName, boneNames);

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
      boneOffsets[currentJointIndex] = boneOffset;
    }
  }
}

void FbxLoader::getControlPoints(FbxNode *node, std::unordered_map<int, ControlPoint> &controlPoints, const std::vector<std::string> &boneNames) {
  FbxMesh *mesh = reinterpret_cast<FbxMesh *>(node->GetNodeAttribute());

  // Get position of control point
  for (int i = 0; i < mesh->GetControlPointsCount(); ++i) {
    ControlPoint controlPoint = {};

    XMFLOAT3 position = {};
    position.x = static_cast<float>(mesh->GetControlPointAt(i).mData[0]);
    position.y = static_cast<float>(mesh->GetControlPointAt(i).mData[1]);
    position.z = static_cast<float>(mesh->GetControlPointAt(i).mData[2]);

    controlPoint.position = position;
    controlPoints[i] = controlPoint;
  }

  // Get bone info and bone name of control point
  for (int deformerIndex = 0; deformerIndex < mesh->GetDeformerCount(); deformerIndex++) {
    FbxSkin *skin = reinterpret_cast<FbxSkin *>(mesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));
    if (!skin) { continue; }

    for (int clusterIndex = 0; clusterIndex < skin->GetClusterCount(); clusterIndex++) {
      FbxCluster *cluster = skin->GetCluster(clusterIndex);

      std::string currentJointName = cluster->GetLink()->GetName();
      int currentJointIndex = findCurrentJointIndexByName(currentJointName, boneNames);

      int *controlPointIndices = cluster->GetControlPointIndices();
      for (int i = 0; i < cluster->GetControlPointIndicesCount(); i++) {
        BoneIndexWeight boneIndexWeight;
        boneIndexWeight.boneIndex = currentJointIndex;
        boneIndexWeight.boneWeight = cluster->GetControlPointWeights()[i];

        controlPoints[controlPointIndices[i]].boneInfo.push_back(boneIndexWeight);
        controlPoints[controlPointIndices[i]].boneName = currentJointName;
      }
    }
  }
}

void FbxLoader::getAnimation(FbxScene *scene, FbxNode *node,
  std::unordered_map<int, ControlPoint> &controlPoints, std::vector<BoneAnimation> &boneAnimations,
  const std::vector<std::string> &boneNames,
  const std::string &name) {
  FbxMesh *mesh = reinterpret_cast<FbxMesh *>(node->GetNodeAttribute());

  boneAnimations.resize(boneNames.size());

  for (int deformerIndex = 0; deformerIndex < mesh->GetDeformerCount(); deformerIndex++) {
    FbxSkin *skin = reinterpret_cast<FbxSkin *>(mesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));
    if (!skin) { continue; }

    for (int clusterIndex = 0; clusterIndex < skin->GetClusterCount(); clusterIndex++) {
      FbxCluster *cluster = skin->GetCluster(clusterIndex);

      std::string currentJointName = cluster->GetLink()->GetName();
      int currentJointIndex = findCurrentJointIndexByName(currentJointName, boneNames);

      BoneAnimation boneAnimation = {};
      FbxAnimStack *animStack = scene->GetSrcObject<FbxAnimStack>(0);
      FbxAnimEvaluator *sceneEvaluator = scene->GetAnimationEvaluator();
      FbxTakeInfo *takeInfo = scene->GetTakeInfo(animStack->GetName());
      FbxTime start = takeInfo->mLocalTimeSpan.GetStart();
      FbxTime end = takeInfo->mLocalTimeSpan.GetStop();

      for (FbxLongLong index = start.GetFrameCount(FbxTime::eCustom); index < end.GetFrameCount(FbxTime::eCustom); ++index) {
        FbxTime currentTime = {};
        currentTime.SetFrame(index, FbxTime::eCustom);

        Keyframe key;
        key.time = static_cast<float>(index / 8.0f);

        FbxAMatrix currentTransformOffset = sceneEvaluator->GetNodeGlobalTransform(node, currentTime);
        FbxAMatrix temp = currentTransformOffset.Inverse() * sceneEvaluator->GetNodeGlobalTransform(cluster->GetLink(), currentTime);
        
        FbxVector4 t = temp.GetT();
        key.t = { (float)t.mData[0], (float)t.mData[1], (float)t.mData[2] };
        t = temp.GetS();
        key.s = { (float)t.mData[0], (float)t.mData[1], (float)t.mData[2] };
        FbxQuaternion q = temp.GetQ();
        key.r = { (float)q.mData[0], (float)q.mData[1], (float)q.mData[2], (float)q.mData[3] };

        if (index != 0 && boneAnimation.keyframes.back() == key) break;

        boneAnimation.keyframes.push_back(key);
      }
      boneAnimations[currentJointIndex] = boneAnimation;
    }
  }

  // Init animation
  BoneAnimation initialBoneAnimation = {};
  for (int i = 0; i < boneNames.size(); ++i) {
    int keyframeSize = boneAnimations[i].keyframes.size();
    if (keyframeSize != 0) {
      for (int j = 0; j < keyframeSize; ++j) {
        Keyframe key;

        key.time = static_cast<float>(j / 144.0f);
        key.t = { 0.0f, 0.0f, 0.0f };
        key.s = { 1.0f, 1.0f, 1.0f };
        key.r = { 0.0f, 0.0f, 0.0f, 0.0f };
        initialBoneAnimation.keyframes.push_back(key);
      }
      break;
    }
  }

  for (int i = 0; i < boneNames.size(); ++i) {
    if (!boneAnimations[i].keyframes.size()) {
      boneAnimations[i] = initialBoneAnimation;
    }
  }

  BoneIndexWeight boneIndexWeight = {};
  for (auto it = controlPoints.begin(); it != controlPoints.end(); ++it) {
    for (int i = it->second.boneInfo.size(); i <= 4; ++i) {
      it->second.boneInfo.push_back(boneIndexWeight);
    }
  }
}

void Animation::getFinalTransforms(float t) {
  int numBones = boneOffsets.size();

  std::vector<XMFLOAT4X4> toParentTransforms(numBones);

  interpolate(t, toParentTransforms);

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

void FbxLoader::getVertexInfo(FbxNode *node, std::vector<VertexDynamic> &vertices,
  std::unordered_map<int, ControlPoint> &controlPoints) {
  FbxMesh *mesh = reinterpret_cast<FbxMesh *>(node->GetNodeAttribute());

  for (int i = 0; i < mesh->GetPolygonCount(); ++i) {
    for (int j = 0; j < 3; ++j) {
      int controlPointIndex = mesh->GetPolygonVertex(i, j);
      ControlPoint controlPoint = controlPoints[controlPointIndex];

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

      VertexDynamic temp;
      temp.pos.x = controlPoint.position.x;
      temp.pos.y = controlPoint.position.y;
      temp.pos.z = controlPoint.position.z;

      temp.normal.x = static_cast<float>(normal.mData[0]);
      temp.normal.y = static_cast<float>(normal.mData[1]);
      temp.normal.z = static_cast<float>(normal.mData[2]);

      temp.tex.x = static_cast<float>(uvs.mData[0]);
      temp.tex.y = static_cast<float>(1.0f - uvs.mData[1]);

      controlPoint.sortBlendingInfoByWeight();

      for (int k = 0; k < controlPoint.boneInfo.size(); k++) {
        if (k >= 4) {
          break;
        }

        temp.boneIndices[k] = controlPoint.boneInfo[k].boneIndex;

        switch (k) {
        case 0:
          temp.boneWeights.x = controlPoint.boneInfo[k].boneWeight;
          break;
        case 1:
          temp.boneWeights.y = controlPoint.boneInfo[k].boneWeight;
          break;
        case 2:
          temp.boneWeights.z = controlPoint.boneInfo[k].boneWeight;
          break;
        }
      }

      vertices.push_back(temp);
    }
  }
}

Model FbxLoader::loadFBXModel(Directx *dx, const std::string &name) {
  FbxImporter *importer = FbxImporter::Create(manager, "");
  FbxScene *scene = FbxScene::Create(manager, "");

  if (!importer->Initialize(name.c_str(), -1, manager->GetIOSettings())) {
    throw std::exception();
  }

  if (!importer->Import(scene)) {
    throw std::exception();
  }

  importer->Destroy();

  FbxNode *root = scene->GetRootNode();
  if (!root) {
    throw std::exception();
  }

  FbxGeometryConverter gc(manager);
  gc.Triangulate(scene, true);

  // Data
  std::vector<int> boneHierarchy;
  std::vector<std::string> boneNames;
  std::vector<XMFLOAT4X4> boneOffsets;
  std::vector<BoneAnimation> boneAnimations;
  std::vector<XMFLOAT4X4> finalTransforms;

  // Get skeleton
  {
    for (int i = 0; i < root->GetChildCount(); ++i) {
      FbxNode *child = root->GetChild(i);
      FbxMesh *mesh = reinterpret_cast<FbxMesh *>(child->GetNodeAttribute());
      FbxNodeAttribute::EType type = mesh->GetAttributeType();

      if (!type) {
        continue;
      }

      switch (type) {
      case FbxNodeAttribute::eSkeleton:
        getSkeleton(child, 0, -1, boneHierarchy, boneNames);
        boneOffsets.resize(boneHierarchy.size());
      }
    }
  }

  std::unordered_map<int, ControlPoint> controlPoints = {};
  std::vector<VertexDynamic> vertices = {};
  std::vector<int> indices = {};
  // Get bone offsets, control points, vertex info
  {
    for (int i = 0; i < root->GetChildCount(); ++i) {
      FbxNode *child = root->GetChild(i);

      if (child->GetNodeAttribute() == NULL) {
        continue;
      }

      FbxNodeAttribute::EType type = child->GetNodeAttribute()->GetAttributeType();
      if (type == FbxNodeAttribute::eMesh) {
        getControlPoints(child, controlPoints, boneNames);
        getBoneOffsets(scene, child, boneOffsets, boneNames);
        getAnimation(scene, child, controlPoints, boneAnimations, boneNames, name);
        getVertexInfo(child, vertices, controlPoints);
      }
    }
  }

  ID3D11Buffer *vertexBuffer = nullptr;
  {
    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(VertexDynamic) * vertices.size();
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bd.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = &vertices[0];

    DX::ThrowIfFailed(dx->device->CreateBuffer(&bd, &initData, &vertexBuffer));
  }

  ID3D11Buffer *indexBuffer = nullptr;
  {
    // TODO(vlad): read indices from .fbx
  }

  Animation *animation = new Animation;
  {
    animation->boneHierarchy = boneHierarchy;
    animation->boneOffsets = boneOffsets;
    animation->boneAnimations = boneAnimations;
    animation->timePosition = 0.0f;
  }

  MeshDynamic *mesh = new MeshDynamic;
  {
    mesh->vertexBuffer = vertexBuffer;
    mesh->indexBuffer = indexBuffer;
    mesh->vertices = vertices;
    mesh->indices = indices;
  }

  return Model{ animation, mesh };
}

Animation *FbxLoader::loadFBXModelAnimation(Directx *dx, const std::string &name) {
  FbxImporter *importer = FbxImporter::Create(manager, "");
  FbxScene *scene = FbxScene::Create(manager, "");

  if (!importer->Initialize(name.c_str(), -1, manager->GetIOSettings())) {
    throw std::exception();
  }

  if (!importer->Import(scene)) {
    throw std::exception();
  }

  importer->Destroy();

  FbxNode *root = scene->GetRootNode();
  if (!root) {
    throw std::exception();
  }

  FbxGeometryConverter gc(manager);
  gc.Triangulate(scene, true);

  // Data
  std::vector<int> boneHierarchy;
  std::vector<std::string> boneNames;
  std::vector<XMFLOAT4X4> boneOffsets;
  std::vector<BoneAnimation> boneAnimations;
  std::vector<XMFLOAT4X4> finalTransforms;

  // Get skeleton
  {
    for (int i = 0; i < root->GetChildCount(); ++i) {
      FbxNode *child = root->GetChild(i);
      FbxMesh *mesh = reinterpret_cast<FbxMesh *>(child->GetNodeAttribute());
      FbxNodeAttribute::EType type = mesh->GetAttributeType();

      if (!type) {
        continue;
      }

      switch (type) {
      case FbxNodeAttribute::eSkeleton:
        getSkeleton(child, 0, -1, boneHierarchy, boneNames);
        boneOffsets.resize(boneHierarchy.size());
      }
    }
  }

  std::unordered_map<int, ControlPoint> controlPoints = {};
  {
    for (int i = 0; i < root->GetChildCount(); ++i) {
      FbxNode *child = root->GetChild(i);

      if (child->GetNodeAttribute() == NULL) {
        continue;
      }

      FbxNodeAttribute::EType type = child->GetNodeAttribute()->GetAttributeType();
      if (type == FbxNodeAttribute::eMesh) {
        getControlPoints(child, controlPoints, boneNames);
        getBoneOffsets(scene, child, boneOffsets, boneNames);
        getAnimation(scene, child, controlPoints, boneAnimations, boneNames, name);
      }
    }
  }

  Animation *animation = new Animation;
  {
    animation->boneHierarchy = boneHierarchy;
    animation->boneOffsets = boneOffsets;
    animation->boneAnimations = boneAnimations;
    animation->timePosition = 0.0f;
  }

  return animation;
}