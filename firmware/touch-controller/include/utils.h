#ifndef UTILS_H
#define UTILS_H

#include <cstdint>

struct LatencyTracker {
  unsigned long sum = 0;
  unsigned long count = 0;
  unsigned long startTime = 0;
};

void scan();
uint32_t updateLatencyMetric(LatencyTracker &tracker, unsigned long dt);

#endif
