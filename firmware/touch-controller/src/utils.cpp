#include <Arduino.h>
#include <Wire.h>

#include "utils.h"

// Constructor
LatencyTracker::LatencyTracker(uint32_t intervalMillis) : intervalMillis(intervalMillis) {
  startTimeMillis = millis();
  sum = 0;
  count = 0;
}

void LatencyTracker::measureAndRecord(MeasureFunc_t measureFunc, ResultCallback_t resultCallback) {
  if (measureFunc) {
    unsigned long measureStartTimeUs = micros();
    measureFunc(); // Execute the function to be measured
    unsigned long dtUs = micros() - measureStartTimeUs;

    sum += dtUs;
    count++;
  }

  unsigned long currentMillis = millis();
  if (currentMillis - startTimeMillis >= intervalMillis) {
    uint32_t avg = 0;
    if (count > 0) {
      avg = sum / count;
    }

    sum = 0;
    count = 0;
    startTimeMillis = currentMillis;

    if (resultCallback) {
      resultCallback(avg); // Call the result callback with the average
    }
  }
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
