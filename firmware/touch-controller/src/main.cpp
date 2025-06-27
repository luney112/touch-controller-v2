#include <Arduino.h>
#include <Wire.h>

#include "air.h"
#include "led.h"
#include "serial.h"
#include "touch.h"
#include "utils.h"

// #define TEST_MODE

void processSensorData();
void processAirSensorData();
void processSliderData();
void processSensorDataTest();

constexpr int SensorReadFrequencyMicros = 1000;
constexpr int SliderTouchedPressureValue = 126;

// Max speed of the i2c bus
// 400kHz max speed for cap1188
// 1Mhz max speed for VL53L5CX
// Must use the lower of the two
constexpr uint32_t WireClockSpeed = 400000;

constexpr uint32_t DebugStateSendIntervalMillis = 5000;   // 5 seconds
constexpr uint32_t LatencyAveragingIntervalMillis = 5000; // 5 seconds

LedController led;
TouchController touch;
AirController air;
SerialController serial;

LatencyTracker serialWriteLatencyTracker(LatencyAveragingIntervalMillis);
LatencyTracker serialReadLatencyTracker(LatencyAveragingIntervalMillis);
LatencyTracker airLoopLatencyTracker(LatencyAveragingIntervalMillis, false);
LatencyTracker sensorProcessingLatencyTracker(LatencyAveragingIntervalMillis);

int ledData[LED_SEGMENT_COUNT] = {0};

unsigned long sensorReadFrequencyStartUs = 0;
unsigned long lastDebugStateSendMillis = 0;

void setup() {
  // Needed for some reason to get complete serial output
  delay(1000);

  Wire.begin();
  Wire.setClock(WireClockSpeed);

  serial.init(&led); // Should be first
  led.init(&serial);
  touch.begin(&serial);
  air.begin(&serial);

  serial.writeDebugLog("Preparing for calibration...")->processWrite();
  led.calibrate();
  delay(100);
  serial.writeDebugLog("Calibrating...")->processWrite();

  air.init();
  touch.init();

  serial.writeDebugLog("Finished calibrating!")->processWrite();
  led.setAllUntouched();

  serial.writeDebugLogf("Wire clock speed is %dhz", Wire.getClock())->processWrite();
  serial.writeDebugLog("Setup complete!")->processWrite();
}

void loop() {
  // Send debug state periodically
  if (millis() - lastDebugStateSendMillis >= DebugStateSendIntervalMillis) {
    serial.writeDebugState();
    lastDebugStateSendMillis = millis();
  }

  // Air loop
  airLoopLatencyTracker.measureAndRecord([]() { air.loop(); },
                                         [](uint32_t avg, uint32_t sum, uint32_t count) {
                                           serial.getDebugState()->airLoopLatencyUs = avg;
                                           serial.getDebugState()->airLoopTimeTotalUs = sum;
                                           serial.getDebugState()->airLoopCount = count;
                                         });

  // Process serial write
  serialWriteLatencyTracker.measureAndRecord(
      []() { serial.processWrite(); },
      [](uint32_t avg, uint32_t sum, uint32_t count) { serial.getDebugState()->serialWriteLatencyUs = avg; });

  // Process serial read
  serialReadLatencyTracker.measureAndRecord(
      []() { serial.read(); },
      [](uint32_t avg, uint32_t sum, uint32_t count) { serial.getDebugState()->serialReadLatencyUs = avg; });

  if (micros() - sensorReadFrequencyStartUs > SensorReadFrequencyMicros) {
#ifdef TEST_MODE
    sensorProcessingLatencyTracker.measureAndRecord(
        []() { processSensorDataTest(); },
        [](uint32_t avg, uint32_t sum, uint32_t count) { serial.getDebugState()->sensorProcessingLatencyUs = avg; });
#else
    sensorProcessingLatencyTracker.measureAndRecord(
        []() { processSensorData(); },
        [](uint32_t avg, uint32_t sum, uint32_t count) { serial.getDebugState()->sensorProcessingLatencyUs = avg; });
#endif
    sensorReadFrequencyStartUs = micros();
  }

  delayMicroseconds(50);
}

void processSensorData() {
  processAirSensorData();
  processSliderData();
}

/* Poll JVS input.

   opbtn returns the cabinet test/service state, where bit 0 is Test and Bit 1
   is Service.

   beam returns the IR beams that are currently broken, where bit 0 is the
   lowest IR beam and bit 5 is the highest IR beam, for a total of six beams.

   Both bit masks are active-high.

   Note that you cannot instantly break the entire IR grid in a single frame to
   simulate hand movement; this will be judged as a miss. You need to simulate
   a gradual raising and lowering of the hands. Consult the proof-of-concept
   implementation for details.

   NOTE: Previous releases of Segatools mapped the IR beam inputs incorrectly.
   Please ensure that you advertise an API version of at least 0x0101 so that
   the correct mapping can be used.

   Minimum API version: 0x0100
   Latest API version: 0x0101 */
void processAirSensorData() {
  // Data is already in expected format
  uint8_t blocked = air.getBlockedSensors();
  serial.writeAirSensorData(&blocked, 1);
}

/* Chunithm touch slider layout:

                               ^^^ Toward screen ^^^

----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 31 | 29 | 27 | 25 | 23 | 21 | 19 | 17 | 15 | 13 | 11 |  9 |  7 |  5 |  3 |  1 |
----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
 32 | 30 | 28 | 26 | 24 | 22 | 20 | 18 | 16 | 14 | 12 | 10 |  8 |  6 |  4 |  2 |
----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+

   There are a total of 32 regions on the touch slider. Each region can return
   an 8-bit pressure value. The operator menu allows the operator to adjust the
   pressure level at which a region is considered to be pressed; the factory
   default value for this setting is 20.

   Callback function supplied to your IO DLL. This must be called with a
   pointer to a 32-byte array of pressure values, one byte per slider cell.
   See above for layout and pressure threshold information.

   The callback will copy the pressure state data out of your buffer before
   returning. The pointer will not be retained. */
void processSliderData() {
  uint32_t touched = touch.getTouchStatus();
  uint8_t keyData[32] = {0};

  // Our data is slighty different from the expected format
  for (int i = 0; i < 32; i += 2) {
    keyData[30 - i] = touched & (1 << i) ? SliderTouchedPressureValue : 0;
    keyData[31 - i] = touched & (1 << (i + 1)) ? SliderTouchedPressureValue : 0;
  }

  serial.writeSliderData(keyData, sizeof(keyData));
}

void processSensorDataTest() {
  // Read and update air sensor
  uint8_t blocked = air.getBlockedSensors();
  led.setBeamBroken(blocked > 0);
  // serial.writeDebugLogf("Air sensor is %d", blocked);

  // Read and update slider
  uint32_t touched = touch.getTouchStatus();
  for (int i = 0, pos = 0; i < 32; i += 2, pos += 3) {
    if (touched & (1 << i) || touched & (1 << (i + 1))) {
      if (ledData[pos] == 0) {
        serial.writeDebugLogf("Key pressed %d", i);
      }
      ledData[pos] = 1;
      ledData[pos + 1] = 1;
    } else {
      if (ledData[pos] == 1) {
        serial.writeDebugLogf("Key released %d", i);
      }
      ledData[pos] = 0;
      ledData[pos + 1] = 0;
    }
  }
  led.set(ledData, LED_SEGMENT_COUNT);
}
