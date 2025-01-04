#include "touch.h"
#include "Wire.h"
#include "serial.h"

#define SET_QUICK_WIRE_CLOCK_SPEED

constexpr uint32_t WireClockSpeed = 400000;

bool TouchController::init(SerialController *serial) {
  this->serial = serial;

  // TODO: Initialize CAP1188

  Wire.begin();
#ifdef SET_QUICK_WIRE_CLOCK_SPEED
  Wire.setClock(WireClockSpeed);
#endif

  serial->writeDebugLog("Initialized CAP1188s")->processWrite();
  return true;
}

void TouchController::getTouchStatus(TouchData &data) {
  data.touched = 0; // TODO: Read CAP1188 states
}
