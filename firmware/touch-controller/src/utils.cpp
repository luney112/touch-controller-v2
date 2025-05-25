#include <Arduino.h>
#include <Wire.h>

#include "utils.h"

constexpr uint32_t LatencyAveragingIntervalMillis = 5000; // 5 seconds

// Helper function to update latency metrics. Returns true if an averaging period has passed.
uint32_t updateLatencyMetric(LatencyTracker &tracker, unsigned long dt) {
  tracker.sum += dt;
  tracker.count++;

  unsigned long currentMillis = millis();
  if (currentMillis - tracker.startTime >= LatencyAveragingIntervalMillis) {
    uint32_t avg = tracker.sum / tracker.count;
    tracker.sum = 0;
    tracker.count = 0;
    tracker.startTime = currentMillis;
    return avg;
  }

  return 0;
}

void scan() {
  Serial.println("Scanning...");

  int nDevices = 0;
  for (byte address = 1; address < 127; address++) {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.print(address, HEX);
      Serial.println("  !");
      nDevices++;
    } else if (error == 4) {
      Serial.print("Unknown error at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found");
  } else {
    Serial.println("Done");
  }
}
