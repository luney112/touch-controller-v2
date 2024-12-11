#include "led.h"
#include "layout.h"
#include "serial.h"
#include <CRC32.h>
#include <FastLED.h>

#define ENABLE_RGBW 1

#define _HSV(h, s, v) (h * 255.0 / 360.0), (s * 255.0 / 100.0), (v * 255.0 / 100.0)

const CRGB ColorUntouched = CHSV(_HSV(51, 100, 100));
const CRGB ColorTouched = CHSV(_HSV(291, 100, 100));
const CRGB ColorCalibrate = CHSV(_HSV(0, 100, 100));
const CRGB ColorUntouchedBeamBroken = CHSV(_HSV(3, 100, 71));
const CRGB ColorTouchedBeamBroken = CHSV(_HSV(120, 78, 72));

constexpr uint8_t LedBrightness = 255;

void LedController::init(SerialController *serial) {
  this->serial = serial;

#ifdef ENABLE_RGBW
  FastLED.addLeds<WS2812B, GPIO_LED_DATA, BRG>(leds, LED_COUNT).setRgbw(RgbwDefault());
#else
  FastLED.addLeds<WS2812B, GPIO_LED_DATA, GRB>(leds, LED_COUNT);
#endif

  for (int i = 0; i < LED_COUNT; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.setBrightness(LedBrightness);
  FastLED.show();

  serial->writeDebugLog("Initialized LEDs")->processWrite();
}

void LedController::setAllUntouched() {
  for (int i = LED_OFFSET; i < LED_COUNT; i++) {
    leds[i] = this->isBeamBroken ? ColorUntouchedBeamBroken : ColorUntouched;
  }
  FastLED.show();
}

void LedController::setAllOff() {
  for (int i = LED_OFFSET; i < LED_COUNT; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
}

// 0 1 | 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17
// 17 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 | 1 0
void LedController::setTouched(int led) {
  int idx = LED_DIRECTION == LeftToRight ? (LED_OFFSET + led) : (LED_COUNT - 1 - led);
  leds[idx] = this->isBeamBroken ? ColorTouchedBeamBroken : ColorTouched;
  FastLED.show();
}

void LedController::setUntouched(int led) {
  int idx = LED_DIRECTION == LeftToRight ? (LED_OFFSET + led) : (LED_COUNT - 1 - led);
  leds[idx] = this->isBeamBroken ? ColorUntouchedBeamBroken : ColorUntouched;
  FastLED.show();
}

// TODO: Use bitmap
void LedController::set(int *vals, int sz) {
  for (int i = 0; i < sz; i++) {
    int idx = LED_DIRECTION == LeftToRight ? (LED_OFFSET + i) : (LED_COUNT - 1 - i);
    if (vals[i] == 0) {
      leds[idx] = this->isBeamBroken ? ColorUntouchedBeamBroken : ColorUntouched;
    } else {
      leds[idx] = this->isBeamBroken ? ColorTouchedBeamBroken : ColorTouched;
    }
  }
  FastLED.show();
}

/* Update the RGB lighting on the slider. A pointer to an array of 32 * 3 = 96
  bytes is supplied, organized in BRG format.
   The first set of bytes is the right-most slider key, and from there the bytes
   alternate between the dividers and the keys until the left-most key.
   There are 31 illuminated sections in total.

   Minimum API version: 0x0100 */
void LedController::setChuniIo(const uint8_t *brg, int sz) {
  if (sz != 93) {
    serial->writeDebugLogf("Expected 93 bytes for LED data, got %d", sz);
    return;
  }

  for (int i = 0; i < sz; i += 6) {
    int idx = LED_DIRECTION == RightToLeft ? (LED_OFFSET + (i / 6)) : (LED_COUNT - 1 - (i / 6));
    leds[idx].setRGB(brg[i + 1], brg[i + 2], brg[i + 0]);
  }

  // show() can be slow, so only call it when the crc of the light pattern changes
  uint32_t currentCrc = CRC32::calculate(brg, sz);
  if (currentCrc != this->ledCrc) {
    FastLED.show();
  }
  this->ledCrc = currentCrc;
}

void LedController::test() {
  leds[2] = CRGB(0xFF0000);
  leds[3] = CRGB(0x00FF00);
  leds[4] = CRGB(0x0000FF);
  leds[5] = CRGB(0xFFD700);
  leds[6] = CHSV(_HSV(51, 100, 100));
  leds[7] = CRGB(0x87CEEB);
  leds[8] = CHSV(_HSV(197, 42.6, 92.2));
  FastLED.show();
}

void LedController::calibrate() {
  for (int i = LED_OFFSET; i < LED_COUNT; i++) {
    leds[i] = ColorCalibrate;
  }
  FastLED.show();
}

void LedController::setBeamBroken(bool broken) {
  this->isBeamBroken = broken;
}
