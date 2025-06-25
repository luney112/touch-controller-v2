#pragma once

#include <cstdint>

// Calculated by:
// LedData[32+2+1] + AirData[1+2+1] = 39bytes
// 1khz send rate = 39000 bytes/s
// 460800 baud = 46080 bytes/s real
constexpr unsigned long SerialBaudRate = 460800;

constexpr uint8_t FrameEsc = 0x5C;
constexpr uint8_t FrameStart = 0x24;
constexpr uint8_t FrameEnd = 0x0A;

constexpr int SerialReadBufferSize = 256;
constexpr int SerialWriteMaxSize = 256;
constexpr int SerialWriteBufferSize = SerialWriteMaxSize * 2 + 3;

constexpr const char* SerialPortName = "\\\\.\\COM3";

enum FramedPacketHeader {
	FramedPacketHeader_DebugLog = 0x20,
	FramedPacketHeader_LedData = 0x30,
	FramedPacketHeader_SliderData = 0x31,
	FramedPacketHeader_AirSensorData = 0x32,
};

HRESULT serial_init(const char led_com[12], DWORD baud);
HRESULT serial_reconnect(const char led_com[12], DWORD baud);

void process_debug_log(const uint8_t* cmd_buffer, int cmd_size);
void process_slider_data(const uint8_t* cmd_buffer, int cmd_size);
void process_air_sensor_data(const uint8_t* cmd_buffer, int cmd_size);
void process_cmd_buffer(const uint8_t* cmd_buffer, int cmd_size);

unsigned int __stdcall read_and_process(void* ctx);
DWORD serial_read(const uint8_t* buffer, int max_size);

bool serial_write_led_data(const uint8_t* buffer, int size);
bool serial_write_framed(const uint8_t* buffer, uint8_t header, int size);
bool serial_write(const uint8_t* buffer, int size);
