#include "cap1188.h"

constexpr uint8_t Register_MainControl = 0x00;
constexpr uint8_t Register_SensorInputStatus = 0x3;
constexpr uint8_t Register_SensitivityControl = 0x1F;
constexpr uint8_t Register_RepeatRateEnable = 0x28;
constexpr uint8_t Register_MultipleTouch = 0x2A;

constexpr uint8_t Register_MainControl_InterruptBit = 0x01;

boolean CAP1188::begin(uint8_t i2cAddr, TwoWire *theWire) {
  this->i2cDevice = new Adafruit_I2CDevice(i2cAddr, theWire);
  if (!this->i2cDevice->begin()) {
    return false;
  }

  if (this->resetPin != -1) {
    pinMode(this->resetPin, OUTPUT);
    digitalWrite(this->resetPin, LOW);
    delay(100);
    digitalWrite(this->resetPin, HIGH);
    delay(100);
    digitalWrite(this->resetPin, LOW);
    delay(100);
  }

  // Main register (default is [00 0 0 000 0])
  // Can tweak Gain for sensitiviy
  writeRegister(Register_MainControl, 0b00000000);

  // Sensitivity control (default is [0 010 1111])
  writeRegister(Register_SensitivityControl, 0b01001111);

  // Enable interrupt
  // Disable auto-reconfigure?
  // Disable repeat?
  // Disable auto-reconfig on long press?
  // Look at other settings to tweak

  // Disable repeating presses (default is 0xFF)
  writeRegister(Register_RepeatRateEnable, 0x0);

  // Allow multiple touches
  writeRegister(Register_MultipleTouch, 0x0);

  return true;
}

uint8_t CAP1188::touched() {
  uint8_t t = readRegister(Register_SensorInputStatus);
  if (t) {
    writeRegister(Register_MainControl, readRegister(Register_MainControl) & ~Register_MainControl_InterruptBit);
  }
  return t;
}

uint8_t CAP1188::readRegister(uint8_t reg) {
  uint8_t buffer[3] = {reg, 0, 0};
  this->i2cDevice->write_then_read(buffer, 1, buffer, 1);
  return buffer[0];
}

void CAP1188::writeRegister(uint8_t reg, uint8_t value) {
  uint8_t buffer[4] = {reg, value, 0, 0};
  this->i2cDevice->write(buffer, 2);
}
