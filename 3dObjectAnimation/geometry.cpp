#pragma once
#include "geometry.h"

void GeometryManager::createBox3D(Directx *dx, const std::string &name, float width, float height, float depth) {
  std::vector<Vertex> vertices(24);

  float w2 = 0.5f*width;
  float h2 = 0.5f*height;
  float d2 = 0.5f*depth;

  // Fill in the front face vertex data.
  vertices[0] = Vertex(-w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
  vertices[1] = Vertex(-w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
  vertices[2] = Vertex(+w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
  vertices[3] = Vertex(+w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

  // Fill in the back face vertex data.
  vertices[4] = Vertex(-w2, -h2, +d2, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
  vertices[5] = Vertex(+w2, -h2, +d2, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
  vertices[6] = Vertex(+w2, +h2, +d2, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
  vertices[7] = Vertex(-w2, +h2, +d2, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);

  // Fill in the top face vertex data.
  vertices[8] = Vertex(-w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
  vertices[9] = Vertex(-w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f);
  vertices[10] = Vertex(+w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);
  vertices[11] = Vertex(+w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f);

  // Fill in the bottom face vertex data.
  vertices[12] = Vertex(-w2, -h2, -d2, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f);
  vertices[13] = Vertex(+w2, -h2, -d2, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f);
  vertices[14] = Vertex(+w2, -h2, +d2, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f);
  vertices[15] = Vertex(-w2, -h2, +d2, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f);

  // Fill in the left face vertex data.
  vertices[16] = Vertex(-w2, -h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
  vertices[17] = Vertex(-w2, +h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  vertices[18] = Vertex(-w2, +h2, -d2, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
  vertices[19] = Vertex(-w2, -h2, -d2, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

  // Fill in the right face vertex data.
  vertices[20] = Vertex(+w2, -h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
  vertices[21] = Vertex(+w2, +h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  vertices[22] = Vertex(+w2, +h2, +d2, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
  vertices[23] = Vertex(+w2, -h2, +d2, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

  // Vertex buffer
  D3D11_BUFFER_DESC bd = {};
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.ByteWidth = vertices.size() * sizeof(Vertex);
  bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  bd.CPUAccessFlags = 0;
  bd.MiscFlags = 0;

  D3D11_SUBRESOURCE_DATA initData = {};
  initData.pSysMem = &vertices[0];

  ID3D11Buffer *vertexBuffer = nullptr;
  DX::ThrowIfFailed(dx->device->CreateBuffer(&bd, &initData, &vertexBuffer));

  std::vector<int> indices(36);

  // Fill in the front face index data
  indices[0] = 0; indices[1] = 1; indices[2] = 2;
  indices[3] = 0; indices[4] = 2; indices[5] = 3;

  // Fill in the back face index data
  indices[6] = 4; indices[7] = 5; indices[8] = 6;
  indices[9] = 4; indices[10] = 6; indices[11] = 7;

  // Fill in the top face index data
  indices[12] = 8; indices[13] = 9; indices[14] = 10;
  indices[15] = 8; indices[16] = 10; indices[17] = 11;

  // Fill in the bottom face index data
  indices[18] = 12; indices[19] = 13; indices[20] = 14;
  indices[21] = 12; indices[22] = 14; indices[23] = 15;

  // Fill in the left face index data
  indices[24] = 16; indices[25] = 17; indices[26] = 18;
  indices[27] = 16; indices[28] = 18; indices[29] = 19;

  // Fill in the right face index data
  indices[30] = 20; indices[31] = 21; indices[32] = 22;
  indices[33] = 20; indices[34] = 22; indices[35] = 23;

  // Index buffer
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.ByteWidth = indices.size() * sizeof(UINT);
  bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
  bd.CPUAccessFlags = 0;
  bd.MiscFlags = 0;

  initData.pSysMem = &indices[0];

  ID3D11Buffer *indexBuffer = nullptr;
  DX::ThrowIfFailed(dx->device->CreateBuffer(&bd, &initData, &indexBuffer));

  Mesh *mesh = new Mesh();
  mesh->indexBuffer = indexBuffer;
  mesh->indices = indices;
  mesh->vertexBuffer = vertexBuffer;
  mesh->vertices = vertices;

  objectsMeshData[name] = mesh;
}

void GeometryManager::createBox2D(Directx *dx, const std::string &name, float x, float y, float w, float h) {
  std::vector<Vertex> vertices(4);

  vertices[0] = Vertex{ x, y, 0, -1, 0, 0, 0, 1 };
  vertices[1] = Vertex{ x, y + h, 0, 0, -1, 0, 0, 0 };
  vertices[2] = Vertex{ x + w, y + h, 0, 1, 0, 0, 1, 0 };
  vertices[3] = Vertex{ x + w, y, 0, 0, 1, 0, 1, 1 };

  std::vector<int> indices = { 0, 1, 2, 0, 2, 3 };

  D3D11_BUFFER_DESC bd = {};
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.ByteWidth = sizeof(int) * indices.size();
  bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
  bd.CPUAccessFlags = 0;
  bd.MiscFlags = 0;

  D3D11_SUBRESOURCE_DATA initData = {};
  initData.pSysMem = &indices[0];

  ID3D11Buffer *indexBuffer = nullptr;
  DX::ThrowIfFailed(dx->device->CreateBuffer(&bd, &initData, &indexBuffer));

  bd.Usage = D3D11_USAGE_DYNAMIC;
  bd.ByteWidth = sizeof(Vertex) * vertices.size();
  bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  bd.MiscFlags = 0;

  initData.pSysMem = &vertices[0];

  ID3D11Buffer *vertexBuffer = nullptr;
  DX::ThrowIfFailed(dx->device->CreateBuffer(&bd, &initData, &vertexBuffer));

  Mesh *mesh = new Mesh();
  mesh->indexBuffer = indexBuffer;
  mesh->indices = indices;
  mesh->vertexBuffer = vertexBuffer;
  mesh->vertices = vertices;

  objectsMeshData[name] = mesh;
}

void GeometryManager::getControlPoints(FbxMesh *mesh) {
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

void GeometryManager::getSkeletonHierachy(FbxNode *node, int currentIndex, int parentIndex, SkinnedData &skinnedData) {
  skinnedData.boneHierarchy.push_back(parentIndex);
  skinnedData.boneNames.push_back(node->GetName());

  for (int i = 0; i < node->GetChildCount(); i++) {
    getSkeletonHierachy(node->GetChild(i), skinnedData.boneHierarchy.size(), currentIndex, skinnedData);
  }
}

void GeometryManager::getAnimation(FbxScene *scene, FbxNode *node, SkinnedData &skinnedData) {
  FbxMesh *mesh = (FbxMesh *)node->GetNodeAttribute();

  const FbxVector4 t = node->GetGeometricTranslation(FbxNode::eSourcePivot);
  const FbxVector4 r = node->GetGeometricRotation(FbxNode::eSourcePivot);
  const FbxVector4 s = node->GetGeometricScaling(FbxNode::eSourcePivot);
  FbxAMatrix geometryTransform = FbxAMatrix(t, r, s);

  AnimationClip animation;
  animation.boneAnimations.resize(skinnedData.boneNames.size());

  for (int deformerIndex = 0; deformerIndex < mesh->GetDeformerCount(); deformerIndex++) {
    FbxSkin *skin = reinterpret_cast<FbxSkin *>(mesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));
    if (!skin) { continue; }

    for (int clusterIndex = 0; clusterIndex < skin->GetClusterCount(); clusterIndex++) {
      FbxCluster *cluster = skin->GetCluster(clusterIndex);

      std::string currentJointName = cluster->GetLink()->GetName();
      int currentJointIndex;
      for (currentJointIndex = 0; currentJointIndex < skinnedData.boneNames.size(); currentJointIndex++) {
        if (skinnedData.boneNames[currentJointIndex] == currentJointName) {
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
  for (int i = 0; i < skinnedData.boneNames.size(); ++i)
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

  for (int i = 0; i < skinnedData.boneNames.size(); ++i)
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

  skinnedData.animations["MAIN"] = animation;
}

void GeometryManager::createFBXModel(Directx *dx, FbxManager *manager, const std::string &path) {
  FbxImporter* fbxImporter = FbxImporter::Create(manager, "");
  FbxScene* fbxScene = FbxScene::Create(manager, "");

  DX::ThrowIfFailed(fbxImporter->Initialize(path.c_str(), -1, manager->GetIOSettings()));
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

  skinnedData.boneOffsets.resize(skinnedData.boneHierarchy.size());

  // Mesh data
  std::vector<Vertex> vertices = {};
  std::vector<int> indices = {};
  ID3D11Buffer *indexBuffer = nullptr;
  ID3D11Buffer *vertexBuffer = nullptr;

  // Animation data
  AnimationData *animationData = new AnimationData();

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
      getAnimation(fbxScene, childNode, skinnedData);
      getVertexInfo(mesh, vertices);

      animationData->skinnedData = skinnedData;
      animationData->timePos = 0;

      objectsAnimationData[path] = animationData;
    }
  }

  D3D11_BUFFER_DESC bd = {};
  bd.Usage = D3D11_USAGE_DYNAMIC;
  bd.ByteWidth = sizeof(Vertex) * vertices.size();
  bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  bd.MiscFlags = 0;

  D3D11_SUBRESOURCE_DATA initData = {};
  initData.pSysMem = &vertices[0];

  DX::ThrowIfFailed(dx->device->CreateBuffer(&bd, &initData, &vertexBuffer));

  // Mesh
  Mesh *mesh = new Mesh();
  mesh->indexBuffer = indexBuffer;
  mesh->indices = indices;
  mesh->vertexBuffer = vertexBuffer;
  mesh->vertices = vertices;

  objectsMeshData[path] = mesh;
}

void GeometryManager::getVertexInfo(FbxMesh *mesh, std::vector<Vertex> &vertices) {
  uint32_t count = mesh->GetPolygonCount();
  for (int i = 0; i < count; i++) {
    std::vector<Vertex> vertex(3);

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