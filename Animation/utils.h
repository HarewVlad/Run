#pragma once
#include "includes.h"

namespace Time {
  typedef std::chrono::steady_clock::time_point TimePoint;

  TimePoint timeGet();
  double getDuration(TimePoint timeEnd, TimePoint timeStart);
}

namespace Debug {
  void printXMVector(const std::string &name, const XMVECTOR &vector);
}
