#include <iostream>
#include <windows.h>
#include <io.h>

#include "logger.h"

extern FILE* g_log_file;

void Log(const char* fmt, ...) {
	static bool initialized = false;
	if (!initialized) {
		fopen_s(&g_log_file, "log.txt", "w");
		if (!g_log_file) {
			OutputDebugStringA("Failed to open log file\n");
			return;
		}

		// Large buffer for performance
		setvbuf(g_log_file, nullptr, _IOFBF, 4 * 1024 * 1024);
		initialized = true;
	}

	if (!g_log_file)
		return;

	va_list ap;
	va_start(ap, fmt);

	va_list ap_copy;
	va_copy(ap_copy, ap);

	vprintf(fmt, ap);
	vfprintf(g_log_file, fmt, ap_copy);

	va_end(ap_copy);
	va_end(ap);

	// Ensure data leaves stdio buffer
	fflush(g_log_file);
}