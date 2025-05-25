/**
 * Some good reference material:
 *  https://www.st.com/resource/en/datasheet/vl53l5cx.pdf
 *  https://www.pololu.com/file/0J1885/um2884-a-guide-to-using-the-vl53l5cx-multizone-timeofflight-ranging-sensor-with-wide-field-of-view-ultra-lite-driver-uld-stmicroelectronics.pdf
 *  https://community.st.com/t5/imaging-sensors/vl53l8cx-example-quot-platform-h-quot-ram-reduction/td-p/714296
 *  https://github.com/sparkfun/SparkFun_VL53L5CX_Arduino_Library/tree/main/examples
 */
#include "air.h"
#include "layout.h"
#include "serial.h"
#include <Arduino.h>
#include <SparkFun_VL53L5CX_Library.h>

constexpr uint8_t AddressMap[TofCount] = {TOF_ADDR_0, TOF_ADDR_1, TOF_ADDR_2, TOF_ADDR_3};
constexpr uint8_t LpnMap[TofCount] = {GPIO_TOF_LPN0, GPIO_TOF_LPN1, GPIO_TOF_LPN2, GPIO_TOF_LPN3};

constexpr uint8_t Resolution = VL53L5CX_RESOLUTION_8X8;
constexpr uint8_t RangingFrequency = 15; // 60hz for 4x4, 15hz for 8x8

constexpr uint16_t BandCount = 6;
constexpr uint16_t SingleBandSizeMm = 23;
constexpr uint16_t LowBarMm = 10.0 * 10; // 10cm
constexpr uint16_t HighBarMm = LowBarMm + BandCount * SingleBandSizeMm;

constexpr uint32_t MaxWireClockSpeed = 1000000;

volatile int AirController::interruptCount;

// This function puts the ToF sensors in a disabled state
// No communication or setup should be done with the sensors
bool AirController::begin(SerialController *serial) {
  this->serial = serial;

  memset(measurementData, 0, sizeof(measurementData));

  // Disable all ToF sensors
  for (int i = 0; i < TofCount; i++) {
    pinMode(LpnMap[i], OUTPUT);
    digitalWrite(LpnMap[i], LOW);
  }
  delay(50);

  return true;
}

// Setup and initialize the ToF sensors
bool AirController::init() {
  /*
      At each power on reset, a staggering 86,000 bytes of firmware have to be sent to the sensor.
      At 100kHz, this can take ~9.4s. By increasing the clock speed, we can cut this time down to ~1.4s.

      Two parameters can be tweaked:

        Clock speed: The VL53L5CX has a max bus speed of 1MHz.

        Max transfer size: The majority of Arduino platforms default to 32 bytes. If you are using one
        with a larger buffer (ESP32 is 128 bytes for example), this can help decrease transfer times a bit.

      https://github.com/sparkfun/SparkFun_VL53L5CX_Arduino_Library/blob/main/examples/Example2_FastStartup/Example2_FastStartup.ino

      ---

      The issue is that CAP1188 has a max speed of 400kHz.
      So only use 1MHz for firmware transfer, then reset it back to what it was before
  */
  const uint32_t currentWireClock = Wire.getClock();
  Wire.setClock(MaxWireClockSpeed);

  // Re-write i2c addresses
  // 1. Disable i2c for all sensors
  // 2. For every sensor:
  //      a) Enable i2c and initialize
  //      b) Write new address
  //      c) Disable i2c
  // 3. Re-enable i2c for all sensors

  // 1. Disable all ToF sensors
  for (int i = 0; i < TofCount; i++) {
    digitalWrite(LpnMap[i], LOW);
  }
  delay(50);

  // 2. Change addresses one-by-one
  for (int i = 0; i < TofCount; i++) {
    // Enable i2c
    digitalWrite(LpnMap[i], HIGH);
    delay(50);

    // Increase default from 32 bytes to 128 - not supported on all platforms
    sensors[i].setWireMaxPacketSize(128);

    // Call begin() using the default address
    // All other i2c devices that could interfere should be disabled
    if (!sensors[i].begin()) {
      serial->writeDebugLogf("[ERROR] Unable to initialize ToF %#02X", AddressMap[i])->processWrite();
      return false;
    }

    // Set the new address
    sensors[i].setAddress(AddressMap[i]);

    // Disable i2c
    digitalWrite(LpnMap[i], LOW);
    delay(50);
  }

  // 3. Re-enable all devices
  for (int i = 0; i < TofCount; i++) {
    digitalWrite(LpnMap[i], HIGH);
  }
  delay(50);

  // Setup interupt
  pinMode(GPIO_TOF_INT, INPUT_PULLUP);
  attachInterrupt(GPIO_TOF_INT, interruptFn, FALLING);

  // TODO: Look at code examples for setting up sensors
  for (int i = 0; i < TofCount; i++) {
    sensors[i].setResolution(Resolution);
    sensors[i].setRangingFrequency(RangingFrequency);
    sensors[i].setRangingMode(SF_VL53L5CX_RANGING_MODE::CONTINUOUS);
    sensors[i].setTargetOrder(SF_VL53L5CX_TARGET_ORDER::STRONGEST);
    // sensors[i].setSharpenerPercent(40);
    sensors[i].startRanging();
  }

  serial->writeDebugLog("Initialized ToFs")->processWrite();

  // Before finishing, set the clock speed back
  Wire.setClock(currentWireClock);

  return true;
}

void AirController::loop() {
  if (interruptCount == 0) {
    return;
  }
  interruptCount = 0;

  for (int i = 0; i < TofCount; i++) {
    if (!sensors[i].getRangingData(&measurementData[i])) {
      serial->writeDebugLog("Could not get ranging data")->processWrite();
      continue;
    }
  }

  uint8_t val = 0;

  // Now, calculate using the ranging data the blocked sensor value
  // For every sensor
  for (int i = 0; i < TofCount; i++) {
    auto data = &measurementData[i];
    // For each zone/pixel of the sensor
    for (int zid = 0; zid < Resolution; zid++) {
      // We must have at least one thing detected
      if (data->nb_target_detected[zid] == 0) {
        continue;
      }
      // We consider 5,6,9 as valid
      auto status = data->target_status[zid];
      if (status == 5 || status == 6 || status == 9) {
        auto mm = data->distance_mm[zid];
        if (mm < LowBarMm || mm > HighBarMm) {
          continue;
        }
        uint16_t normalized = mm - LowBarMm;
        uint16_t pos = (normalized / SingleBandSizeMm); // pos will be 0-indexed (0 to BandCount - 1)
        if (pos >= BandCount) {                         // pos should be less than BandCount
          serial->writeDebugLogf("Detected invalid ToF band state: %dmm -> pos=%d", mm, pos);
          continue;
        }
        val |= (1 << pos); // Set only the bit corresponding to the detected band (0-indexed)
      }
    }
  }

  lastCalculatedSensorValue = val;
}

uint8_t AirController::getBlockedSensors() {
  return lastCalculatedSensorValue;
}
