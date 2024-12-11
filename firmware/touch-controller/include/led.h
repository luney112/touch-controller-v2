#ifndef LED_H
#define LED_H

#include "layout.h"
#include <FastLED.h>

class SerialController;

class LedController {
public:
  void init(SerialController *serial);
  void setAllUntouched();
  void setAllOff();
  void setTouched(int led);
  void setUntouched(int led);
  void set(int *vals, int sz);
  void setChuniIo(const uint8_t *brg, int sz);
  void test();
  void calibrate();
  void setBeamBroken(bool broken);

private:
  CRGB leds[LED_COUNT];
  bool isBeamBroken = false;
  uint32_t ledCrc = 0;

  SerialController *serial;
};

#endif
