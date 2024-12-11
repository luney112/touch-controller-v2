#include "air.h"
#include "layout.h"
#include <Arduino.h>

constexpr int CalibrationCount = 200;
constexpr double ThresholdRatio = 0.8;
constexpr uint32_t IrToggleSettleTimeMicroseconds = 50;
constexpr uint32_t CalibrationIntervalMillis = 10;

void AirController::init() {
  // TODO: Init
}

void AirController::calibrate() {
  // TODO: Calibrate
}

uint8_t AirController::getBlockedSensors() {
  return 0; // TODO: Implement
}
