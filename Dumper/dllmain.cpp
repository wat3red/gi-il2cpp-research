// dllmain.cpp
#include "common.h"
#include "utils.h"
#include "hooks.h"
#include "il2cpp_bridge.h"
#include "dumper.h"
#include <thread>

DWORD WINAPI StartThread(LPVOID) {
    Utils::CreateConsole();
    Utils::DisableLogReport();

    Config::GameBase = (uintptr_t)GetModuleHandle(NULL);
    Utils::Log("Game Base: 0x%p\n", (void*)Config::GameBase);

    // Initialize API pointers
    Il2Cpp::Initialize();

    // Start packet unblocking timer
    std::thread blockPacketsThread([]() {
        Sleep(10000);
        Config::BlockPackets = false;
        Utils::Log("Packets unblocked.\n");
        });
    blockPacketsThread.detach();

    // Wait for Unity Window
    while (!FindWindowA("UnityWndClass", nullptr)) {
        Sleep(100);
    }

    Sleep(15000); // Wait for game initialization
    Utils::Log("Starting dump...\n");

    // Choose what to dump here
    // Dumper::DumpFull();
    //Dumper::GenerateSDK();
    Dumper::DumpForIDA();
    

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        if (Hooks::Init()) {
            CreateThread(NULL, 0, StartThread, NULL, 0, NULL);
        }
    }
    else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
        Hooks::Uninit();
        // Clean up file handles/consoles if needed
    }
    return TRUE;
}