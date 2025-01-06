#ifndef TOUCH_H
#define TOUCH_H

#define CAP1188_COUNT 1

#include "cap1188.h"
#include <cstdint>

class SerialController;

struct TouchData {
  // Bitmap of touch sensor data
  // 4 sensors, 8 keys each, so 32 bits
  // Touch status is in LSB ordering (Key1=bit0, key2=bit1, ...)
  uint32_t touched;
};

class TouchController {
public:
  bool init(SerialController *serial);
  void getTouchStatus(TouchData &data);

private:
  SerialController *serial;

  CAP1188 caps[CAP1188_COUNT];
};

#endif
