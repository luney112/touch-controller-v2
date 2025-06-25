#include <Windows.h>
#include <stdio.h>
#include <mutex>

#include "serial.h"
#include "log.h"

HANDLE serial_port = INVALID_HANDLE_VALUE;
HANDLE serial_read_thread = INVALID_HANDLE_VALUE;
std::mutex serial_mux;

uint8_t latest_beam_data = 0;
uint8_t latest_slider_data[32] = { 0 };
std::mutex slider_mux;

HRESULT serial_init(const char led_com[12], DWORD baud)
{
	dlog("Initializing serial");

	if (serial_port != INVALID_HANDLE_VALUE) {
		return S_OK;
	}

	BOOL status = true;

	serial_port = CreateFileA(led_com,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if (serial_port == INVALID_HANDLE_VALUE)
		dlog("Chunithm Serial LEDs: Failed to open COM port (Attempted on %S)\n", led_com);
	else
		dlog("Chunithm Serial LEDs: COM port success!\n");

	DCB dcb_serial_params = { 0 };
	dcb_serial_params.DCBlength = sizeof(dcb_serial_params);
	status &= GetCommState(serial_port, &dcb_serial_params);

	dcb_serial_params.BaudRate = baud;
	dcb_serial_params.ByteSize = 8;
	dcb_serial_params.StopBits = ONESTOPBIT;
	dcb_serial_params.Parity = NOPARITY;
	status &= SetCommState(serial_port, &dcb_serial_params);

	COMMTIMEOUTS timeouts = { 0 };
	timeouts.ReadIntervalTimeout = 5;
	timeouts.ReadTotalTimeoutConstant = 5;
	timeouts.ReadTotalTimeoutMultiplier = 5;
	timeouts.WriteTotalTimeoutConstant = 5;
	timeouts.WriteTotalTimeoutMultiplier = 5;

	status &= SetCommTimeouts(serial_port, &timeouts);

	if (!status)
	{
		return E_FAIL;
	}

	dlog("Initializing serial: Success");

	serial_read_thread = (HANDLE)_beginthreadex(
		NULL,
		0,
		read_and_process,
		NULL,
		0,
		NULL);

	return S_OK;
}

HRESULT serial_reconnect(const char led_com[12], DWORD baud) {
	dlog("Reconnecting to serial");

	BOOL status = true;

	serial_port = CreateFileA(led_com,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if (serial_port == INVALID_HANDLE_VALUE)
		dlog("Chunithm Serial LEDs: Failed to open COM port (Attempted on %S)\n", led_com);
	else
		dlog("Chunithm Serial LEDs: COM port success!\n");

	DCB dcb_serial_params = { 0 };
	dcb_serial_params.DCBlength = sizeof(dcb_serial_params);
	status &= GetCommState(serial_port, &dcb_serial_params);

	dcb_serial_params.BaudRate = baud;
	dcb_serial_params.ByteSize = 8;
	dcb_serial_params.StopBits = ONESTOPBIT;
	dcb_serial_params.Parity = NOPARITY;
	status &= SetCommState(serial_port, &dcb_serial_params);

	COMMTIMEOUTS timeouts = { 0 };
	timeouts.ReadIntervalTimeout = 5;
	timeouts.ReadTotalTimeoutConstant = 5;
	timeouts.ReadTotalTimeoutMultiplier = 5;
	timeouts.WriteTotalTimeoutConstant = 5;
	timeouts.WriteTotalTimeoutMultiplier = 5;

	status &= SetCommTimeouts(serial_port, &timeouts);

	if (!status)
	{
		return E_FAIL;
	}

	dlog("Initializing serial: Success");

	return S_OK;
}

void process_debug_log(const uint8_t* cmd_buffer, int cmd_size) {
	char buffer[SerialReadBufferSize + 1] = { 0 };
	memcpy_s(buffer, sizeof(buffer), cmd_buffer, cmd_size);
	buffer[cmd_size] = 0;
	dlog("DebugLog: %s", buffer);
}

void process_slider_data(const uint8_t* cmd_buffer, int cmd_size) {
	if (cmd_size != 32) {
		dlog("Expected 32 bytes for slider data but got %d", cmd_size);
		return;
	}
	slider_mux.lock();
	memcpy(latest_slider_data, cmd_buffer, cmd_size);
	slider_mux.unlock();
}

void process_air_sensor_data(const uint8_t* cmd_buffer, int cmd_size) {
	if (cmd_size != 1) {
		dlog("Expected 1 byte for air sensor data but got %d", cmd_size);
		return;
	}
	latest_beam_data = *cmd_buffer;
}

void process_cmd_buffer(const uint8_t* cmd_buffer, int cmd_size) {
	if (cmd_size == 0) {
		return;
	}
	switch (cmd_buffer[0]) {
	case FramedPacketHeader_DebugLog:
		process_debug_log(cmd_buffer + 1, cmd_size - 1);
		break;
	case FramedPacketHeader_SliderData:
		process_slider_data(cmd_buffer + 1, cmd_size - 1);
		break;
	case FramedPacketHeader_AirSensorData:
		process_air_sensor_data(cmd_buffer + 1, cmd_size - 1);
		break;
	default:
		// Do nothing
		break;
	}
}

unsigned int __stdcall read_and_process(void* ctx) {
	uint8_t buffer[SerialReadBufferSize];

	uint8_t cmd_buffer[SerialReadBufferSize];
	int cmd_size = 0;
	bool last_byte_esc = false;
	while (true) {
		DWORD read = serial_read(buffer, SerialReadBufferSize);

		for (unsigned int i = 0; i < read; i++) {
			const uint8_t b = buffer[i];
			if (b == FrameStart && !last_byte_esc) {
				cmd_size = 0; // Reset buffer, discard everything
			}
			else if (b == FrameEnd && !last_byte_esc) {
				process_cmd_buffer(cmd_buffer, cmd_size);
				cmd_size = 0; // Reset after processing
			}
			else if (b == FrameEsc && !last_byte_esc) {
				last_byte_esc = true;
			}
			else {
				if (cmd_size >= SerialReadBufferSize) {
					cmd_size = 0; // Error state: Reset buffer, discard everything
				}
				cmd_buffer[cmd_size++] = b;
				last_byte_esc = false;
			}
		}
		Sleep(1);
	}
}

DWORD serial_read(const uint8_t* buffer, int max_size) {
	if (serial_port == INVALID_HANDLE_VALUE) {
		return 0;
	}

	if (max_size <= 0) {
		return 0;
	}

	DWORD bytes_read = 0;

	BOOL status = ReadFile(
		serial_port,
		(void*)buffer,
		max_size,
		&bytes_read,
		NULL);

	if (!status) {
		DWORD last_err = GetLastError();
		dlog("Serial port read failed -- %d\n", last_err);
	}

	return bytes_read;
}

bool serial_write_led_data(const uint8_t* buffer, int size) {
	return serial_write_framed(buffer, FramedPacketHeader_LedData, size);
}

bool serial_write_framed(const uint8_t* buffer, uint8_t header, int size) {
	if (size > SerialWriteMaxSize) {
		return false;
	}
	uint8_t encoded[SerialWriteBufferSize] = { 0 };
	int encoded_size = 0;

	encoded[encoded_size++] = FrameStart;
	encoded[encoded_size++] = header;
	for (int i = 0; i < size; i++) {
		if (buffer[i] == FrameEsc || buffer[i] == FrameStart || buffer[i] == FrameEnd) {
			encoded[encoded_size++] = FrameEsc;
		}
		encoded[encoded_size++] = buffer[i];
	}
	encoded[encoded_size++] = FrameEnd;

	return serial_write(encoded, encoded_size);
}

bool serial_write(const uint8_t* buffer, int size)
{
	if (size <= 0) {
		return true;
	}

	if (serial_port == INVALID_HANDLE_VALUE) {
		return false;
	}

	serial_mux.lock();

	DWORD bytes_written = 0;

	BOOL status = WriteFile(
		serial_port,
		buffer,
		size,
		&bytes_written,
		NULL);

	if (!status) {
		DWORD last_err = GetLastError();
		dlog("Serial port write failed -- %d\n", last_err);
	}

	serial_mux.unlock();
	return status;
}
