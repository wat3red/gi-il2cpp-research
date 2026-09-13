#include "common.h"
#include "utils.h"
#include "hooks.h"
#include "il2cpp_bridge.h"
#include "dumper.h"

#include <thread>

#include "../Common/network_blocker.h"

DWORD WINAPI StartThread(LPVOID) {
	Utils::CreateConsole();
	Utils::DisableLogReport();

	Config::GameBase = reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr));
	Utils::Log("Game Base: 0x%p\n", reinterpret_cast<void*>(Config::GameBase));

	Il2Cpp::Initialize();

	// Wait for the Unity window, then give the client time to finish booting.
	while (!FindWindowA("UnityWndClass", nullptr))
		Sleep(100);

	Sleep(15000);
	Utils::Log("Starting dump...\n");

	// Pick one:
	// Dumper::DumpFull();
	// Dumper::GenerateSDK();
	Dumper::DumpForIDA();

	return 0;
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
	if (reason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(module);

		static NetworkBlocker blocker;

		if (Hooks::Init())
			CreateThread(nullptr, 0, StartThread, nullptr, 0, nullptr);
	}
	else if (reason == DLL_PROCESS_DETACH) {
		Hooks::Uninit();
	}
	return TRUE;
}
