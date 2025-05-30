#ifndef CAP1188_H
#define CAP1188_H

#include "Arduino.h"
#include <Adafruit_I2CDevice.h>

class CAP1188 {
public:
  bool begin(uint8_t i2cAddr, TwoWire *theWire = &Wire);
  bool init();
  uint8_t touched(); // Touch status is in LSB ordering (Key1=bit0, key2=bit1, ...)

private:
  uint8_t readRegister(uint8_t reg);
  void writeRegister(uint8_t reg, uint8_t value);

  Adafruit_I2CDevice *i2cDevice = NULL;
};

#endif
