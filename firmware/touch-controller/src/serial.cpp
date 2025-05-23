#include "serial.h"
#include "led.h"
#include <Arduino.h>
#include <stdarg.h>

// Calculated by:
// LedData[32+2+1] + AirData[1+2+1] = 39bytes
// 1khz send rate = 39000 bytes/s
// 460800 baud = 46080 bytes/s real
constexpr unsigned long SerialBaudRate = 460800;

constexpr uint8_t FrameEsc = 0x5C;
constexpr uint8_t FrameStart = 0x24;
constexpr uint8_t FrameEnd = 0x0A;

enum FramedPacketHeader {
  FramedPacketHeader_DebugLog = 0x20,
  FramedPacketHeader_LedData = 0x30,
  FramedPacketHeader_SliderData = 0x31,
  FramedPacketHeader_AirSensorData = 0x32,
};

void SerialController::init(LedController *ledController) {
  Serial.begin(SerialBaudRate);
  writeDebugLog("-------------------------------------")->processWrite();
  writeDebugLog("Initialized serial")->processWrite();
  this->ledController = ledController;
}

// TODO: Add CRC
void SerialController::read() {
  while (Serial.available() > 0) {
    uint8_t b = static_cast<uint8_t>(Serial.read());

    if (b == FrameStart && !lastByteEsc) {
      this->readBufferLen = 0; // Reset buffer, discard everything
    } else if (b == FrameEnd && !lastByteEsc) {
      this->processBuffer();
    } else if (b == FrameEsc && !lastByteEsc) {
      this->lastByteEsc = true;
    } else {
      if (this->readBufferLen >= SerialReadBufferSize) {
        this->readBufferLen = 0; // Error state: Reset buffer, discard everything
      }
      this->readBuffer[this->readBufferLen++] = b;
      this->lastByteEsc = false;
    }
  }
}

void SerialController::processBuffer() {
  if (this->readBufferLen == 0) {
    return;
  }
  // this->writeDebugLogf("Processing buffer data: %d bytes with header %d", this->readBufferLen, this->readBuffer[0]);
  switch (this->readBuffer[0]) {
  case FramedPacketHeader_LedData:
    this->processLedDataCommand(this->readBuffer + 1, this->readBufferLen - 1);
    break;
  default:
    // Do nothing
    break;
  }
}

void SerialController::processLedDataCommand(uint8_t *buffer, int sz) {
  this->ledController->setChuniIo(buffer, sz);
}

SerialController *SerialController::writeDebugLog(const char *str) {
  this->writeFramed(FramedPacketHeader_DebugLog, (uint8_t *)str, strlen(str));
  return this;
}

SerialController *SerialController::writeDebugLogf(const char *fmt, ...) {
  char buffer[256];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  this->writeDebugLog(buffer);
  va_end(args);
  return this;
}

void SerialController::writeAirSensorData(uint8_t *buf, int sz) {
  this->writeFramed(FramedPacketHeader_AirSensorData, buf, sz);
}

void SerialController::writeSliderData(uint8_t *buf, int sz) {
  this->writeFramed(FramedPacketHeader_SliderData, buf, sz);
}

void SerialController::writeFramed(uint8_t header, uint8_t *buf, int sz) {
  writeByte(FrameStart);
  writeByte(header);
  for (int i = 0; i < sz; i++) {
    if (buf[i] == FrameEsc || buf[i] == FrameStart || buf[i] == FrameEnd) {
      writeByte(FrameEsc);
    }
    writeByte(buf[i]);
  }
  writeByte(FrameEnd);
}

// Write one byte to the ring buffer
// Returns false if the byte was NOT successfuly written
bool SerialController::writeByte(uint8_t b) {
  if (this->writeBufferLen >= SerialWriteBufferSize) {
    return false;
  }
  this->writeBuffer[(this->writeStart + this->writeBufferLen) % SerialWriteBufferSize] = b;
  this->writeBufferLen += 1;
  return true;
}

void SerialController::processWrite() {
  int available = Serial.availableForWrite();
  int toWrite = min(this->writeBufferLen, available);

  while (toWrite > 0) { // Should take max 2 iterations
    uint8_t *buf = this->writeBuffer + this->writeStart;
    int sz = min(toWrite, SerialWriteBufferSize - this->writeStart);
    Serial.write(buf, sz);
    this->writeStart = (this->writeStart + sz) % SerialWriteBufferSize;
    this->writeBufferLen -= sz;
    toWrite -= sz;
  }
}
