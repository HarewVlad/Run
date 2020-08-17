#pragma once
#include "directx.h"
#include "utils.h"
#include "shaders.h"
#include "constantBuffers.h"
#include "vertex.h"
#include "geometry.h"
#include "camera.h"
#include "textures.h"
#include "fbxLoader.h"
#include "player.h"

// TODO(vlad): use forward declarations of above shit

struct Game : public Directx {
  Game();
  void initGame();

  template<typename T>
  void addElementToUMap(const std::string &name, T element, std::unordered_map<std::string, T> &uMap) {
    uMap[name] = element;
  } 

  template<typename T>
  T getElementFromUMap(const std::string &name, std::unordered_map<std::string, T> &uMap) {
    auto it = uMap.find(name);
    if (it == uMap.end()) {
      OutputDebugStringA(std::string("Element with name \"" + name + "\" doesn't exists\n").c_str());
      throw std::exception();
    }

    return uMap[name];
  }

  void run();
  void onRender(float t);
  void onInput(float t);
  void onUpdate(float t);

  void onMouseDown(WPARAM btnState, int x, int y) override;
  void onMouseUp(WPARAM btnState, int x, int y) override;
  void onMouseMove(WPARAM btnState, int x, int y) override;

  // Components
  FbxLoader *fbxLoader;
  GeometryConstructor *geometryConstructor;
  Camera *camera;
  Player *player;

  // Data
  std::unordered_map<std::string, VertexShader *> vertexShaders;
  std::unordered_map<std::string, PixelShader *> pixelShaders;
  std::unordered_map<std::string, ID3D11InputLayout *> inputLayouts;
  std::unordered_map<std::string, ID3D11Buffer *> buffers;
  std::unordered_map<std::string, ID3D11ShaderResourceView *> textures;
  std::unordered_map<std::string, Object *> objects;

  // Mouse
  POINT prevMousePosition;
};