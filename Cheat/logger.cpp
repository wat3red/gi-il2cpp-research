#include <iostream>
#include <windows.h>
#include <io.h>

extern FILE* g_log_file;

void Log(const char* fmt, ...) {
	static bool initialized = false;
	if (!initialized) {
		fopen_s(&g_log_file, "log.txt", "w");
		if (!g_log_file) {
			printf("Failed to open dump file!\n");
			return;
		}
		else {
			setvbuf(g_log_file, nullptr, _IOFBF, 4 * 1024 * 1024);
			initialized = true;
		}
	}

	static size_t counter = 0;
	if (!g_log_file) return;

	va_list ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	vfprintf(g_log_file, fmt, ap);
	va_end(ap);

	//if (++counter % 200 == 0)   // flush only every 200 line
	//fflush(g_log_file);

	fflush(g_log_file);
	int fd = _fileno(g_log_file);
	if (fd != -1) {
		intptr_t os_handle = _get_osfhandle(fd);
		if (os_handle != -1 && os_handle != (intptr_t)INVALID_HANDLE_VALUE) {
			FlushFileBuffers((HANDLE)os_handle);
		}
	}
}