#ifndef SERIAL_H
#define SERIAL_H

#include <cstdint>

class LedController;

constexpr int SerialReadBufferSize = 256;
constexpr int SerialWriteBufferSize = 256;

class SerialController {
public:
  void init(LedController *ledController);
  void read();
  void processWrite();

  SerialController *writeDebugLog(const char *str);
  SerialController *writeDebugLogf(const char *fmt, ...);
  void writeAirSensorData(uint8_t *buf, int sz);
  void writeSliderData(uint8_t *buf, int sz);

  int availableToWrite() { return SerialWriteBufferSize - writeBufferLen; }

private:
  void processBuffer();
  void processLedDataCommand(uint8_t *buffer, int sz);
  void writeFramed(uint8_t header, uint8_t *buf, int sz);
  bool writeByte(uint8_t b);

  uint8_t readBuffer[SerialReadBufferSize];
  int readBufferLen = 0;
  bool lastByteEsc = false;

  uint8_t writeBuffer[SerialWriteBufferSize];
  int writeBufferLen = 0;
  int writeStart = 0;

  LedController *ledController;
};

#endif
