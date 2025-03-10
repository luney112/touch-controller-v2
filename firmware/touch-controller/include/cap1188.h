#ifndef CAP1188_H
#define CAP1188_H

#include "Arduino.h"
#include <Adafruit_I2CDevice.h>

class CAP1188 {
public:
  CAP1188(int8_t resetPin = -1) { this->resetPin = resetPin; }

  bool begin();
  bool init(uint8_t i2cAddr, TwoWire *theWire = &Wire);
  void enable();
  void disable();
  uint8_t touched();

private:
  uint8_t readRegister(uint8_t reg);
  void writeRegister(uint8_t reg, uint8_t value);

  Adafruit_I2CDevice *i2cDevice = NULL;
  int8_t resetPin;
};

#endif
