#ifndef TOUCH_H
#define TOUCH_H

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
};

#endif
