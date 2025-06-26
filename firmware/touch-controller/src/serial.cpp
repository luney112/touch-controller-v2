#include "serial.h"
#include "led.h"
#include <Arduino.h>
#include <stdarg.h>

// Calculated by:
// LedData[32+2+1] + AirData[1+2+1] = 39bytes
// 1khz send rate (ideal) = 39000 bytes/s
// 460800 baud = 46080 bytes/s real
constexpr unsigned long SerialBaudRate = 460800;

constexpr uint8_t FrameEsc = 0x5C;   // Backslash
constexpr uint8_t FrameStart = 0x24; // Dollar sign
constexpr uint8_t FrameEnd = 0x0A;   // LineFeed

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

uint8_t writeUint32ToBuffer(uint8_t *buffer, uint32_t value) {
  uint32_t netValue = htonl(value);
  memcpy(buffer, &netValue, sizeof(netValue));
  return sizeof(netValue);
}

void SerialController::writeDebugState() {
  // uint8_t buffer[128];
  // uint8_t cnt = 0;

  // cnt += writeUint32ToBuffer(buffer + cnt, this->debugState.writeBufferOverflowCount);
  // cnt += writeUint32ToBuffer(buffer + cnt, this->debugState.serialReadLatencyUs);
  // cnt += writeUint32ToBuffer(buffer + cnt, this->debugState.serialWriteLatencyUs);
  // cnt += writeUint32ToBuffer(buffer + cnt, this->debugState.sensorProcessingLatencyUs);
  // cnt += writeUint32ToBuffer(buffer + cnt, this->debugState.airLoopLatencyUs);
  // cnt += writeUint32ToBuffer(buffer + cnt, this->debugState.airLoopTimeTotalUs);
  // cnt += writeUint32ToBuffer(buffer + cnt, this->debugState.airLoopCount);

  // writeFramed(FramedPacketHeader_DebugState, buffer, cnt);

  writeDebugLogf("Debug state: "
                 "OverflowCount=%d, ReadLatency=%dus, WriteLatency=%dus, SensorLatency=%dus, AirLoopLatency=%dms, "
                 "AirLoopTotal=%dms, AirLoopCount=%d",
                 this->debugState.writeBufferOverflowCount, this->debugState.serialReadLatencyUs,
                 this->debugState.serialWriteLatencyUs, this->debugState.sensorProcessingLatencyUs,
                 this->debugState.airLoopLatencyUs / 1000, this->debugState.airLoopTimeTotalUs / 1000,
                 this->debugState.airLoopCount);
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

bool SerialController::writeFramed(uint8_t header, uint8_t *buf, int sz) {
  // Calculate required size
  int requiredSize = 2; // Start byte + Header byte
  for (int i = 0; i < sz; i++) {
    if (buf[i] == FrameEsc || buf[i] == FrameStart || buf[i] == FrameEnd) {
      requiredSize += 2; // Byte + Escape byte
    } else {
      requiredSize += 1; // Just the byte
    }
  }
  requiredSize += 1; // End byte

  if (requiredSize > availableToWrite()) {
    this->debugState.writeBufferOverflowCount++; // Increment overflow counter
    return false;
  }

  // Write all bytes to the buffer
  writeByte(FrameStart);
  writeByte(header);
  for (int i = 0; i < sz; i++) {
    if (buf[i] == FrameEsc || buf[i] == FrameStart || buf[i] == FrameEnd) {
      writeByte(FrameEsc);
    }
    writeByte(buf[i]);
  }
  writeByte(FrameEnd);
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
