#include <iostream>
#include <windows.h>

extern FILE* g_LogFile;

void Log(const char* fmt, ...) {
	static bool initialized = false;
	if (!initialized) {
		fopen_s(&g_LogFile, "il2cpp_dump.cs", "w");
		if (!g_LogFile) {
			printf("Failed to open dump file!\n");
			return;
		}
		else {
			setvbuf(g_LogFile, nullptr, _IOFBF, 4 * 1024 * 1024);
			initialized = true;
		}
	}

	static size_t counter = 0;
	if (!g_LogFile) return;

	va_list ap;
	va_start(ap, fmt);
	vfprintf(g_LogFile, fmt, ap);
	va_end(ap);
}