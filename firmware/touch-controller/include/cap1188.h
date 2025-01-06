#include "Arduino.h"
#include <Adafruit_I2CDevice.h>

class CAP1188 {
public:
  CAP1188(int8_t resetPin = -1) { this->resetPin = resetPin; }

  boolean begin(uint8_t i2cAddr, TwoWire *theWire = &Wire);
  uint8_t touched();

private:
  uint8_t readRegister(uint8_t reg);
  void writeRegister(uint8_t reg, uint8_t value);

  Adafruit_I2CDevice *i2cDevice = NULL;
  int8_t resetPin;
};
