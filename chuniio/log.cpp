#include <Windows.h>
#include <stdio.h>

#include "log.h"

void dlog(const char* fmt, ...)
{
	char new_fmt[128];
	sprintf_s(new_fmt, sizeof(new_fmt), "---[ChuniIo]--- %s\n", fmt);

	char buffer[512];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), new_fmt, args);
	OutputDebugStringA(buffer);
	va_end(args);
}
