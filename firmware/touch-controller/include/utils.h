#ifndef UTILS_H
#define UTILS_H

#include <cstdint>
#include <functional>

class LatencyTracker {
public:
  using MeasureFunc_t = std::function<void()>;
  using ResultCallback_t = std::function<void(uint32_t)>;

  LatencyTracker(uint32_t intervalMillis);

  void measureAndRecord(MeasureFunc_t measureFunc, ResultCallback_t resultCallback);

private:
  uint32_t intervalMillis = 0;
  unsigned long sum = 0;
  unsigned long count = 0;
  unsigned long startTimeMillis = 0;
};

void scan();

#endif
