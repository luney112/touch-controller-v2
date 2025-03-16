#ifndef AIR_H
#define AIR_H

#include <cstdint>
#include <vl53l7cx_class.h>

class SerialController;

struct AirSensorData {};

class AirController {
public:
  bool begin(SerialController *serial);
  bool init();
  uint8_t getBlockedSensors(); // LSB ordering, low=bit0, 6 bits only

private:
  void initSingleSensor(VL53L7CX *sensor, uint8_t addr);
  void assertOk(uint8_t status, const char *name);

  static void interruptFn() { interruptCount += 1; }

  SerialController *serial;

  VL53L7CX *sensorLeft;
  VL53L7CX *sensorRight;

  static volatile int interruptCount;
};

#endif
