#pragma once

#include "StopWatch.h"

class MockStopWatch : public StopWatch {
public:

   MockStopWatch() : StopWatch() {
   };

   void SetTimeForwardSeconds(const std::time_t warp) {
      mStart = clock::time_point(clock::now() + std::chrono::seconds(warp));
   }
};

