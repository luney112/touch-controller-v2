#ifndef TOUCH_H
#define TOUCH_H

#include "cap1188.h"
#include <cstdint>

class SerialController;

constexpr uint8_t Cap1188Count = 4;

struct TouchData {
  // Bitmap of touch sensor data
  // 4 sensors, 8 keys each, so 32 bits
  // Touch status is in LSB ordering (Key1=bit0, key2=bit1, ...)
  uint32_t touched;
};

class TouchController {
public:
  bool begin(SerialController *serial);
  bool init();
  void getTouchStatus(TouchData &data);

private:
  SerialController *serial;

  void enableTouchSensors();
  void disableTouchSensors();

  CAP1188 sensors[Cap1188Count];
};

#endif
