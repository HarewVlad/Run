#pragma once
#include "includes.h"

namespace Time
{
  typedef std::chrono::steady_clock::time_point TimePoint;
  TimePoint(*timeGet)() = std::chrono::high_resolution_clock::now;

  double getDuration(TimePoint timeEnd, TimePoint timeStart)
  {
    return std::chrono::duration<float>(timeEnd - timeStart).count();
  }
}
