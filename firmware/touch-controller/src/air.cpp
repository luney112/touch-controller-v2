#include "air.h"
#include "layout.h"
#include "serial.h"
#include <Arduino.h>
#include <vl53l7cx_class.h>

constexpr uint8_t SpadIndexCount = 4;
constexpr uint8_t SpadIndexes[SpadIndexCount] = {9, 10, 5, 6};

volatile int AirController::interruptCount;

bool AirController::begin(SerialController *serial) {
  this->serial = serial;

  // sensorLeft = new VL53L7CX(&Wire, GPIO_TOF_LPN0, GPIO_TOF_RESET);
  sensorRight = new VL53L7CX(&Wire, GPIO_TOF_LPN1, GPIO_TOF_RESET);

  // sensorLeft->begin();
  sensorRight->begin();
  return true;
}

bool AirController::init() {
  sensorRight->vl53l7cx_on();
  sensorRight->vl53l7cx_i2c_reset();
  sensorRight->vl53l7cx_set_i2c_address(TOF_ADDR_RIGHT);

  // Set interrupt pin
  pinMode(GPIO_TOF_INT, INPUT_PULLUP);
  attachInterrupt(GPIO_TOF_INT, interruptFn, FALLING);

  // initSingleSensor(sensorLeft, TOF_ADDR_LEFT);
  initSingleSensor(sensorRight, TOF_ADDR_RIGHT);

  serial->writeDebugLog("Initialized ToF")->processWrite();
  return true;
}

void AirController::initSingleSensor(VL53L7CX *sensor, uint8_t addr) {
  assertOk(sensor->init_sensor(addr), "init_sensor");
  // assertOk(sensor->vl53l7cx_set_ranging_frequency_hz(30), "vl53l7cx_set_ranging_frequency_hz");
  assertOk(sensor->vl53l7cx_set_ranging_mode(VL53L7CX_RANGING_MODE_CONTINUOUS), "vl53l7cx_set_ranging_mode");
  assertOk(sensor->vl53l7cx_set_sharpener_percent(60), "vl53l7cx_set_sharpener_percent");
  assertOk(sensor->vl53l7cx_start_ranging(), "vl53l7cx_start_ranging");
}

void AirController::assertOk(uint8_t status, const char *name) {
  if (status != VL53L7CX_STATUS_OK) {
    serial->writeDebugLogf("%s failed: %d", name, status)->processWrite();
  }
}

uint8_t AirController::getBlockedSensors() {
  VL53L7CX_ResultsData resultsData;

  if (interruptCount > 0) {
    interruptCount = 0;
    assertOk(sensorRight->vl53l7cx_get_ranging_data(&resultsData), "vl53l7cx_get_ranging_data");

    int32_t sum = 0;
    int32_t cnt = 0;
    for (int i = 0; i < SpadIndexCount; i++) {
      uint8_t idx = SpadIndexes[i];
      if (resultsData.target_status[idx] == 5) {
        sum += resultsData.distance_mm[idx];
        cnt++;
      }
    }
    if (cnt > 0) {
      double distance = static_cast<double>(sum) / cnt;
      serial->writeDebugLogf("%fmm", distance);
    }
  }

  return 0; // TODO: Implement
}
