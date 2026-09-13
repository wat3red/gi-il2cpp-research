#pragma once

#include <windows.h>

// Installs a vectored exception handler that only logs crashes originating
// from our DLL (module+offset + optional symbol), leaving game crashes alone.
void InstallCrashHandler();
void RemoveCrashHandler();
