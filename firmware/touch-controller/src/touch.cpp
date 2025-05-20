#include "touch.h"
#include "layout.h"
#include "serial.h"

constexpr uint8_t AddressMap[Cap1188Count] = {CAP1188_ADDR_0, CAP1188_ADDR_1, CAP1188_ADDR_2, CAP1188_ADDR_3};

bool TouchController::begin(SerialController *serial) {
  this->serial = serial;

  for (int i = 0; i < Cap1188Count; i++) {
    this->sensors[i].begin(AddressMap[i]);
  }

  // Cap1188s start off disabled
  if (GPIO_TOUCH_RESET > 0) {
    pinMode(GPIO_TOUCH_RESET, OUTPUT);
    disableTouchSensors();
  }

  return true;
}

bool TouchController::init() {
  disableTouchSensors();
  enableTouchSensors();

  for (int i = 0; i < Cap1188Count; i++) {
    if (!this->sensors[i].init()) {
      serial->writeDebugLogf("[ERROR] Unable to initialize CAP1188 %#02X", AddressMap[i])->processWrite();
      return false;
    }
  }

  serial->writeDebugLog("Initialized CAP1188s")->processWrite();
  return true;
}

void TouchController::getTouchStatus(TouchData &data) {
  data.touched = 0;
  for (int i = 0; i < Cap1188Count; i++) {
    data.touched |= this->sensors[i].touched() << (i * 8);
  }
}

void TouchController::enableTouchSensors() {
  if (GPIO_TOUCH_RESET > 0) {
    digitalWrite(GPIO_TOUCH_RESET, LOW);
    delay(50);
  }
}

void TouchController::disableTouchSensors() {
  if (GPIO_TOUCH_RESET > 0) {
    // The RESET pin is an active high reset that is driven from an external source.
    // While it is asserted high, all the internal blocks will be held in reset including the communications protocol
    digitalWrite(GPIO_TOUCH_RESET, HIGH);
    delay(50);
  }
}
