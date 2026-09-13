#include <winsock2.h>
#include <windows.h>
#include <filesystem>
#include <thread>
#include <iostream>

#include "external/minhook/include/MinHook.h"
#include "game_api/include.h"
#include "game_api/functions/resolve_funcs.h"
#include "logger/logger.h"
#include "directx_hook.h"
#include "features/features.h"
#include "crash_handler.h"
#include "../Common/network_blocker.h"

#pragma comment(lib, "ws2_32.lib")

uintptr_t g_game_base = 0;
FILE* g_log_file = nullptr;
bool g_block_packets = true;

namespace {

// Unity crash / telemetry reporters. Opening them exclusive-read is enough
// to keep the game from loading them during a research session.
void DisableLogReport() {
	wchar_t filename[MAX_PATH] = {};
	GetModuleFileNameW(nullptr, filename, MAX_PATH);

	auto path = std::filesystem::path(filename);
	path = path.parent_path() / (path.stem().wstring() + L"_Data") / L"Plugins";

	CreateFileW((path / L"Astrolabe.dll").c_str(), GENERIC_READ, 0, nullptr,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	CreateFileW((path / L"MiHoYoMTRSDK.dll").c_str(), GENERIC_READ, 0, nullptr,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
}

void InitConsole() {
	AllocConsole();

	FILE* out = nullptr;
	FILE* in = nullptr;
	FILE* err = nullptr;
	freopen_s(&out, "CONOUT$", "w", stdout);
	freopen_s(&in, "CONIN$", "r", stdin);
	freopen_s(&err, "CONOUT$", "w", stderr);

	std::ios::sync_with_stdio(true);
	std::cin.clear();
	std::cout.clear();
	std::cerr.clear();
}

DWORD WINAPI StartThread(LPVOID) {
	MH_Initialize();

	InitConsole();
	DisableLogReport();

	g_game_base = reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr));
	Log("Game Base: 0x%p\n", reinterpret_cast<void*>(g_game_base));

	// Give the client a moment before we start talking to the network again.
	std::thread([] {
		Sleep(11'000);
		g_block_packets = false;
	}).detach();

	while (!FindWindowA("UnityWndClass", nullptr))
		Sleep(100);

	InitSDK();

	dx_hook::HookPresent();
	features::InitAllFeatures();

	return 0;
}

} // namespace

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
	if (reason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(module);
		InstallCrashHandler();

		g_game_base = reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr));
		static NetworkBlocker blocker;

		CreateThread(nullptr, 0, StartThread, nullptr, 0, nullptr);
	}
	else if (reason == DLL_PROCESS_DETACH) {
		RemoveCrashHandler();

		if (g_log_file) {
			fflush(g_log_file);
			fclose(g_log_file);
			g_log_file = nullptr;
		}

		MH_Uninitialize();
	}
	return TRUE;
}
