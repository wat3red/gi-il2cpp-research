#include <iostream>
#include <windows.h>

extern FILE* g_log_file;

void Log(const char* fmt, ...) {
	static bool initialized = false;
	if (!initialized) {
		fopen_s(&g_log_file, "il2cpp_dump.cs", "w");
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
	vfprintf(g_log_file, fmt, ap);
	va_end(ap);
}