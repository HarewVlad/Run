#include "utils.h"

namespace Time
{
  TimePoint timeGet() {
    return std::chrono::high_resolution_clock::now();
  }

  double getDuration(TimePoint timeEnd, TimePoint timeStart)
  {
    return std::chrono::duration<float>(timeEnd - timeStart).count();
  }
}

namespace Debug {
  void printXMVector(const std::string &name, const XMVECTOR &vector) {
    std::string text = name + " - {" + std::to_string(vector.m128_f32[0]) + ", " + std::to_string(vector.m128_f32[1]) + ", " + std::to_string(vector.m128_f32[2]) + "}\n";
    OutputDebugStringA(text.c_str());
  }
}