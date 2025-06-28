/***
 * Some good reference material:
 *  https://ww1.microchip.com/downloads/aemDocuments/documents/OTH/ProductDocuments/DataSheets/00001620C.pdf
 *  https://ww1.microchip.com/downloads/aemDocuments/documents/OTH/ApplicationNotes/ApplicationNotes/00002034B.pdf
 *  https://ww1.microchip.com/downloads/aemDocuments/documents/OTH/ProductDocuments/LegacyCollaterals/00001334B.pdf
 */
#include "cap1188.h"

constexpr uint8_t Register_MainControl = 0x00;
constexpr uint8_t Register_SensorInputStatus = 0x3;
constexpr uint8_t Register_SensitivityControl = 0x1F;
constexpr uint8_t Register_Sampling = 0x24;
constexpr uint8_t Register_InterruptEnable = 0x27;
constexpr uint8_t Register_RepeatRateEnable = 0x28;
constexpr uint8_t Register_MultipleTouch = 0x2A;
constexpr uint8_t Register_MainControl_InterruptBit = 0x01;

bool CAP1188::begin(uint8_t i2cAddr, TwoWire *theWire) {
  // IMPORTANT!!!
  // DO NOT call begin() on `i2cDevice` as it also calls Wire.begin() and may re-initialize i2c
  this->i2cDevice = new Adafruit_I2CDevice(i2cAddr, theWire);
  return true;
}

bool CAP1188::init() {
  // Main register (default is [00 0 0 000 0])
  // Can tweak Gain for sensitiviy
  writeRegister(Register_MainControl, 0b10000000);

  // Sensitivity control (default is [0 010 1111])
  writeRegister(Register_SensitivityControl, 0b00101111);

  // Default is 0x39 [0 011 10 01]
  // count=4, time=640us, decode_time=35ms
  writeRegister(Register_Sampling, 0b00100100);

  // Enable interrupts on all keys (default is 0xFF)
  writeRegister(Register_InterruptEnable, 0xFF);

  // Disable repeating presses (default is 0xFF)
  writeRegister(Register_RepeatRateEnable, 0x0);

  // Allow multiple touches (default is 0x80)
  writeRegister(Register_MultipleTouch, 0x0);

  return true;
}

// TODO: Can this be done in fewer operations
// TODO: Can I use a cache and only rely on interupts?
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
