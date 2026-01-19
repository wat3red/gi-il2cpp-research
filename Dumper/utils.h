// utils.h
#pragma once
#include "common.h"

namespace Utils {
	// Setup console for logging
	void CreateConsole();

	// Disable Unity crash/log reporting
	void DisableLogReport();

	// String manipulation
	std::string StripNamespaces(const std::string& full);

	void Log(const char* fmt, ...);
}