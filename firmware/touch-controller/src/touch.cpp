#include "touch.h"
#include "Wire.h"
#include "layout.h"
#include "serial.h"

// #define SET_QUICK_WIRE_CLOCK_SPEED

constexpr uint32_t WireClockSpeed = 400000;

constexpr uint8_t AddressMap[Cap1188Count] = {CAP1188_ADDR_0, CAP1188_ADDR_1, CAP1188_ADDR_2, CAP1188_ADDR_3};

bool TouchController::begin(SerialController *serial) {
  this->serial = serial;

#ifdef SET_QUICK_WIRE_CLOCK_SPEED
  Wire.setClock(WireClockSpeed);
#endif

  for (int i = 0; i < Cap1188Count; i++) {
    this->caps[i] = CAP1188(GPIO_TOUCH_RESET);
    this->caps[i].begin();
  }

  return true;
}

bool TouchController::init() {
  for (int i = 0; i < Cap1188Count; i++) {
    auto i2cAddress = AddressMap[i];
    if (!this->caps[i].init(i2cAddress)) {
      serial->writeDebugLogf("[ERROR] Unable to initialize CAP1188 %#02X", i2cAddress)->processWrite();
      return false;
    }
  }

  serial->writeDebugLog("Initialized CAP1188s")->processWrite();
  return true;
}

void TouchController::getTouchStatus(TouchData &data) {
  data.touched = 0;
  for (int i = 0; i < Cap1188Count; i++) {
    data.touched |= this->caps[i].touched() << (i * 8);
  }
}
