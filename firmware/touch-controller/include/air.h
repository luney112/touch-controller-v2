#ifndef AIR_H
#define AIR_H

#include <SparkFun_VL53L5CX_Library.h>
#include <cstdint>

class SerialController;

constexpr uint8_t TofCount = 4;

class AirController {
public:
  bool begin(SerialController *serial);
  bool init();
  void loop();
  uint8_t getBlockedSensors(); // LSB ordering, low=bit0, 6 bits only

private:
  static void interruptFn() { interruptCount += 1; }

  SerialController *serial;

  SparkFun_VL53L5CX sensors[TofCount];
  VL53L5CX_ResultsData measurementData[TofCount];
  // Value is cached and updated in loop() when new data is available via interrupt
  uint8_t lastCalculatedSensorValue = 0;

  static volatile int interruptCount;
};

#endif
