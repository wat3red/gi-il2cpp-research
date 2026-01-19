// utils.cpp
#include "utils.h"

namespace Utils {
	void CreateConsole() {
		AllocConsole();
		FILE* fDummy;
		freopen_s(&fDummy, "CONOUT$", "w", stdout);
		freopen_s(&fDummy, "CONIN$", "r", stdin);
		freopen_s(&fDummy, "CONOUT$", "w", stderr);
		std::ios::sync_with_stdio(true);
	}

	void DisableLogReport() {
		wchar_t filename[MAX_PATH] = {};
		GetModuleFileNameW(NULL, filename, MAX_PATH);

		auto path = std::filesystem::path(filename);
		path = path.parent_path() / (path.stem().string() + "_Data") / "Plugins";

		// Lock report DLLs to prevent loading
		CreateFileW((path / "Astrolabe.dll").c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		CreateFileW((path / "MiHoYoMTRSDK.dll").c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	}

	std::string StripNamespaces(const std::string& full) {
		std::string out;
		out.reserve(full.size());

		for (size_t i = 0; i < full.size(); ++i) {
			char c = full[i];

			// If we encounter a name inside generics (< ... >)
			if (std::isalnum((unsigned char)c) || c == '_') {
				size_t start = i;

				// Read token (until < > , . whitespace)
				while (i < full.size() &&
					(std::isalnum((unsigned char)full[i]) || full[i] == '_' || full[i] == '.')) {
					i++;
				}

				std::string token = full.substr(start, i - start);

				// If namespace exists -> cut everything before the last dot
				size_t dot = token.rfind('.');
				if (dot != std::string::npos)
					token = token.substr(dot + 1);

				out += token;
				i--; // Compensate loop increment
				continue;
			}
			out.push_back(c);
		}
		return out;
	}
	void Log(const char* fmt, ...) {
		static bool initialized = false;
		static FILE* g_log_file;

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
}