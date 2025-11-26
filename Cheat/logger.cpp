#include <iostream>
#include <windows.h>

extern FILE* g_LogFile;

void Log(const char* fmt, ...) {
	static bool initialized = false;
	if (!initialized) {
		fopen_s(&g_LogFile, "log.txt", "w");
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
	vprintf(fmt, ap);
	vfprintf(g_LogFile, fmt, ap);
	va_end(ap);

	//if (++counter % 200 == 0)   // flush only every 200 line
	fflush(g_LogFile);
}