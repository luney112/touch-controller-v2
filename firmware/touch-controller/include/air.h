#ifndef AIR_H
#define AIR_H

#include "layout.h"
#include <cstdint>

struct AirSensorData {};

class AirController {
public:
  void init();
  void calibrate();
  uint8_t getBlockedSensors(); // LSB ordering, low=bit0, 6 bits only

private:
};

#endif
