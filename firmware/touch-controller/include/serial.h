#ifndef SERIAL_H
#define SERIAL_H

#include <cstdint>

class LedController;

struct DebugStatePayload {
  uint32_t writeBufferOverflowCount = 0;
  uint32_t serialReadLatencyUs = 0;
  uint32_t serialWriteLatencyUs = 0;
  uint32_t sensorProcessingLatencyUs = 0;
  uint32_t airLoopLatencyUs = 0;
  uint32_t airLoopTimeTotalUs = 0;
  uint32_t airLoopCount = 0;
};

enum FramedPacketHeader {
  FramedPacketHeader_DebugLog = 0x20,
  FramedPacketHeader_LedData = 0x30,
  FramedPacketHeader_SliderData = 0x31,
  FramedPacketHeader_AirSensorData = 0x32,
  FramedPacketHeader_DebugState = 0x40,
};

constexpr int SerialReadBufferSize = 512;
constexpr int SerialWriteBufferSize = 512;

class SerialController {
public:
  void init(LedController *ledController);
  void read();
  void processWrite();

  SerialController *writeDebugLog(const char *str);
  SerialController *writeDebugLogf(const char *fmt, ...);
  void writeAirSensorData(uint8_t *buf, int sz);
  void writeSliderData(uint8_t *buf, int sz);
  void writeDebugState();

  DebugStatePayload *getDebugState() { return &debugState; };

  int availableToWrite() { return SerialWriteBufferSize - writeBufferLen; }

private:
  void processBuffer();
  void processLedDataCommand(uint8_t *buffer, int sz);
  bool writeFramed(uint8_t header, uint8_t *buf, int sz);
  bool writeByte(uint8_t b);

  uint8_t readBuffer[SerialReadBufferSize];
  int readBufferLen = 0;
  bool lastByteEsc = false;

  uint8_t writeBuffer[SerialWriteBufferSize];
  int writeBufferLen = 0;
  int writeStart = 0;

  LedController *ledController;
  DebugStatePayload debugState;
};

#endif
