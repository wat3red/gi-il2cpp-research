// dllmain.cpp
#include <winsock2.h>
#include <windows.h>
#include <cstdio>
#include <cinttypes>
#include <cstdint>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <dbghelp.h>
#include <psapi.h>
#include <string>
#include <stdarg.h>
#include <io.h>
#include <fcntl.h>
#include <sstream>
#include <filesystem>

#include "lib/minhook/include/MinHook.h"
#include "il2cpp_types.h"
#include "sdk_types.h"
#include "logger.h"

#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "ws2_32.lib")

// Globals
uintptr_t g_Base = 0;
FILE* g_LogFile = nullptr;
bool g_BlockPackets = true;

std::unordered_map<Il2CppType*, std::string> g_cachedTypes;
std::unordered_map<Il2CppClass*, std::string> g_cachedClassNames;

// Function pointers
typedef int (WINAPI* send_t)(SOCKET, const char*, int, int);
send_t o_send = nullptr;
typedef int (WINAPI* WSASend_t)(SOCKET, LPWSABUF, DWORD, LPDWORD, DWORD, LPWSAOVERLAPPED, LPWSAOVERLAPPED_COMPLETION_ROUTINE);
WSASend_t o_WSASend = nullptr;
typedef int (WINAPI* connect_t)(SOCKET, const sockaddr*, int);
connect_t o_connect = nullptr;

int WINAPI h_send(SOCKET s, const char* buf, int len, int flags)
{
	if (g_BlockPackets)
		return len; // притворяемся, что отправили

	return o_send(s, buf, len, flags);
}

int WINAPI h_WSASend(
	SOCKET s, LPWSABUF buffers, DWORD bufferCount,
	LPDWORD bytesSent, DWORD flags,
	LPWSAOVERLAPPED overlapped,
	LPWSAOVERLAPPED_COMPLETION_ROUTINE completion
) {
	if (g_BlockPackets) {
		if (bytesSent) *bytesSent = buffers->len;
		return 0;
	}

	return o_WSASend(s, buffers, bufferCount, bytesSent, flags, overlapped, completion);
}

int WINAPI h_connect(SOCKET s, const sockaddr* name, int namelen)
{
	if (g_BlockPackets) {
		WSASetLastError(WSAECONNREFUSED);
		return SOCKET_ERROR;
	}

	return o_connect(s, name, namelen);
}

void (*o_set_fieldOfView)(Unity::Camera* _this, float value);
void h_set_fieldOfView(Unity::Camera* _this, float value) {
	o_set_fieldOfView(_this, 70.f);

	MoleMole::EntityManager* entityManager = MoleMole::EntityManager::get_EntityManager();

	Log("MoleMole::EntityManager::get_EntityManager() = %p\n", entityManager);

	if (!entityManager) return;

	std::vector<MoleMole::BaseEntity*> entties = entityManager->entities();

	for (MoleMole::BaseEntity* entity : entties) {
		if (!entity) continue;
		Unity::String* name = entity->name();

		Log("entity: %p, l: %d, name: %s\n", entity, name->m_StringLength, name->ToCString());
	}
}

void DisableLogReport()
{
	wchar_t filename[MAX_PATH] = {};
	GetModuleFileName(NULL, filename, MAX_PATH);

	auto path = std::filesystem::path(filename);
	path = path.parent_path() / (path.stem().string() + "_Data") / "Plugins";

	CreateFileW((path / "Astrolabe.dll").c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	CreateFileW((path / "MiHoYoMTRSDK.dll").c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
}

// Thread entry: initialize console, open dump file, resolve function ptrs, hook
DWORD WINAPI StartThread(LPVOID)
{
	AllocConsole();
	FILE* fOut = nullptr;
	FILE* fIn = nullptr;
	FILE* fErr = nullptr;
	freopen_s(&fOut, "CONOUT$", "w", stdout);
	freopen_s(&fIn, "CONIN$", "r", stdin);
	freopen_s(&fErr, "CONOUT$", "w", stderr);
	std::ios::sync_with_stdio(true);
	std::cin.clear();
	std::cout.clear();
	std::cerr.clear();

	DisableLogReport();

	g_Base = (uintptr_t)GetModuleHandle(NULL);
	Log("Game Base: 0x%p\n", (void*)g_Base);

	if (MH_Initialize() != MH_OK) {
		Log("MinHook init failed!\n");
		return 1;
	}

	MH_CreateHook((LPVOID)(g_Base + 0x15126C0), h_set_fieldOfView, (void**)&o_set_fieldOfView);

	// === Packet Blocker Hooks ===
	MH_CreateHookApi(L"ws2_32", "send", h_send, (LPVOID*)&o_send);
	MH_CreateHookApi(L"ws2_32", "WSASend", h_WSASend, (LPVOID*)&o_WSASend);
	MH_CreateHookApi(L"ws2_32", "connect", h_connect, (LPVOID*)&o_connect);

	MH_EnableHook(MH_ALL_HOOKS);

	std::thread blockPacketsThread(([]() { Sleep(10000); g_BlockPackets = false; }));
	blockPacketsThread.detach();

	return 0;
}

// DLL entry
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(hModule);
		CreateThread(NULL, 0, StartThread, NULL, 0, NULL);
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		// flush + close dump file
		if (g_LogFile) {
			fflush(g_LogFile);
			int fd = _fileno(g_LogFile);
			if (fd != -1) {
				intptr_t osHandle = _get_osfhandle(fd);
				if (osHandle != -1 && osHandle != (intptr_t)INVALID_HANDLE_VALUE) {
					FlushFileBuffers((HANDLE)osHandle);
				}
			}
			fclose(g_LogFile);
			g_LogFile = nullptr;
		}

		MH_Uninitialize();
	}
	return TRUE;
}
