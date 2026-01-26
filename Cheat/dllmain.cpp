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

#include "external/minhook/include/MinHook.h"
#include "game_api/include.h"
#include "game_api/functions/resolve_funcs.h"
#include "logger/logger.h"
#include "directx_hook.h"
#include "features/features.h"

#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "ws2_32.lib")

// Globals
uintptr_t g_game_base = 0;
FILE* g_log_file = nullptr;
bool g_block_packets = true;

// Function pointers
typedef int (WINAPI* send_t)(SOCKET, const char*, int, int);
send_t o_send = nullptr;
typedef int (WINAPI* WSASend_t)(SOCKET, LPWSABUF, DWORD, LPDWORD, DWORD, LPWSAOVERLAPPED, LPWSAOVERLAPPED_COMPLETION_ROUTINE);
WSASend_t o_WSASend = nullptr;
typedef int (WINAPI* connect_t)(SOCKET, const sockaddr*, int);
connect_t o_connect = nullptr;

int WINAPI h_send(SOCKET s, const char* buf, int len, int flags) {
	if (g_block_packets)
		return len; // притворяемся, что отправили

	return o_send(s, buf, len, flags);
}

int WINAPI h_WSASend(
	SOCKET s, LPWSABUF buffers, DWORD bufferCount,
	LPDWORD bytesSent, DWORD flags,
	LPWSAOVERLAPPED overlapped,
	LPWSAOVERLAPPED_COMPLETION_ROUTINE completion
) {
	if (g_block_packets) {
		if (bytesSent) *bytesSent = buffers->len;
		return 0;
	}

	return o_WSASend(s, buffers, bufferCount, bytesSent, flags, overlapped, completion);
}

int WINAPI h_connect(SOCKET s, const sockaddr* name, int namelen) {
	if (g_block_packets) {
		WSASetLastError(WSAECONNREFUSED);
		return SOCKET_ERROR;
	}

	return o_connect(s, name, namelen);
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

bool PatchMemory(void* address, const void* bytes, size_t size) {
	DWORD oldProtect;
	if (!VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldProtect))
		return false;

	std::memcpy(address, bytes, size);
	FlushInstructionCache(GetCurrentProcess(), address, size);

	VirtualProtect(address, size, oldProtect, &oldProtect);
	return true;
}

// Packet Blocker Hooks
bool InitBlockingHooks() {
	if (MH_Initialize() != MH_OK) {
		Log("MinHook init failed!\n");
		return 0;
	}

	MH_CreateHookApi(L"ws2_32", "send", h_send, (LPVOID*)&o_send);
	MH_CreateHookApi(L"ws2_32", "WSASend", h_WSASend, (LPVOID*)&o_WSASend);
	MH_CreateHookApi(L"ws2_32", "connect", h_connect, (LPVOID*)&o_connect);

	MH_EnableHook(MH_ALL_HOOKS);

	// ban :)
	//constexpr uint8_t nops[2] = { 0x90, 0x90 }; 
	//PatchMemory((void*)(g_game_base + 0x992B98), nops, sizeof(nops));

	return 1;
}

// Thread entry: initialize console, open dump file, resolve function ptrs, hook
DWORD WINAPI StartThread(LPVOID)
{
	AllocConsole();
	FILE* f_out = nullptr;
	FILE* f_in = nullptr;
	FILE* f_err = nullptr;
	freopen_s(&f_out, "CONOUT$", "w", stdout);
	freopen_s(&f_in, "CONIN$", "r", stdin);
	freopen_s(&f_err, "CONOUT$", "w", stderr);
	std::ios::sync_with_stdio(true);
	std::cin.clear();
	std::cout.clear();
	std::cerr.clear();

	//while (true) {
		//Sleep(1000);
	//}

	DisableLogReport();

	g_game_base = (uintptr_t)GetModuleHandle(NULL);

	Log("Game Base: 0x%p\n", (void*)g_game_base);

	std::thread block_packets_thread(([]() { Sleep(11'000); g_block_packets = false; }));
	block_packets_thread.detach();

	while (!FindWindowA("UnityWndClass", nullptr)) {
		Sleep(100);
	}

	InitSDK();

	dx_hook::HookPresent();

	features::InitAllFeatures();

	return 0;
}

#include <iphlpapi.h>
#include <vector>
#include <ws2tcpip.h>
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
class NetworkBlocker {
private:
	HANDLE hThread;
	bool running;

	// IPs and ports to block - customize as needed
	std::vector<std::string> blockedIPs = {
		"123.45.67.8",
		"18.67.250.95",
		"47.253.211.216",
		"8.221.104.125",
		"43.152.42.241",
		"43.152.26.154",
		"18.67.250.64",
		"18.67.250.90",
		"18.67.250.93",
		"2.23.103.27",
		"3.174.180.75",
		"13.33.243.94",
		"3.174.180.114",
		"3.174.180.27",
		"47.245.55.171",
		"8.209.222.121",
		"47.91.86.250",
		"8.211.9.13",
		"8.211.3.101",
		"108.157.113.82",
		"3.174.180.89",
		"108.157.113.70",
		"108.157.113.68",
		"108.157.113.11",
		"13.33.243.127",
		"13.33.243.104",
		"127.65.240.113",
		"43.152.42.60",
		"163.181.92.201",
		"47.245.154.152",
		"47.245.155.149",
		"127.0.0.1"
	};
	std::vector<USHORT> blockedPorts; // Will be filled with range 10000-65000

	bool IsConnectionBlocked(DWORD remoteAddr, USHORT remotePort) {
		// Convert IP to string for checking
		char ipStr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &remoteAddr, ipStr, INET_ADDRSTRLEN);

		// Check if IP is blocked
		for (const auto& blockedIP : blockedIPs) {
			if (strcmp(ipStr, blockedIP.c_str()) == 0) {
				return true;
			}
		}

		// Check if port is in blocked range
		if (remotePort >= 10000 && remotePort <= 65000) {
			return true;
		}

		return false;
	}

	static DWORD WINAPI MonitorThread(LPVOID lpParam) {
		NetworkBlocker* blocker = (NetworkBlocker*)lpParam;
		blocker->MonitorConnections();
		return 0;
	}

	void MonitorConnections() {
		while (running) {
			PMIB_TCPTABLE2 pTcpTable = NULL;
			DWORD dwSize = 0;
			DWORD dwResult = 0;

			// Get TCP table size
			dwResult = GetTcpTable2(NULL, &dwSize, TRUE);
			if (dwResult == ERROR_INSUFFICIENT_BUFFER) {
				pTcpTable = (PMIB_TCPTABLE2)malloc(dwSize);
				if (pTcpTable == NULL) {
					Sleep(1000);
					continue;
				}
			}
			else {
				Sleep(1000);
				continue;
			}

			// Get TCP table
			dwResult = GetTcpTable2(pTcpTable, &dwSize, TRUE);
			if (dwResult == NO_ERROR) {
				for (DWORD i = 0; i < pTcpTable->dwNumEntries; i++) {
					MIB_TCPROW2 row = pTcpTable->table[i];

					// Check if connection should be blocked
					if (IsConnectionBlocked(row.dwRemoteAddr, ntohs(row.dwRemotePort))) {
						// Close the connection
						MIB_TCPROW tcpRow;
						tcpRow.dwState = MIB_TCP_STATE_DELETE_TCB;
						tcpRow.dwLocalAddr = row.dwLocalAddr;
						tcpRow.dwLocalPort = row.dwLocalPort;
						tcpRow.dwRemoteAddr = row.dwRemoteAddr;
						tcpRow.dwRemotePort = row.dwRemotePort;

						SetTcpEntry(&tcpRow);

						char ipStr[INET_ADDRSTRLEN] = { 0 };

						IN_ADDR addr;
						addr.S_un.S_addr = row.dwRemoteAddr;

						InetNtopA(AF_INET, &addr, ipStr, INET_ADDRSTRLEN);

						//std::cout << "Blocked connection to " << ipStr << ":" << ntohs(row.dwRemotePort) << std::endl;
					}
				}
			}

			if (pTcpTable) {
				free(pTcpTable);
			}

			Sleep(100); // Check every 100ms
		}
	}

public:
	NetworkBlocker() : running(true) {
		// Initialize blocked ports range
		for (USHORT port = 10000; port <= 65000; port++) {
			blockedPorts.push_back(port);
		}

		hThread = CreateThread(NULL, 0, MonitorThread, this, 0, NULL);
	}

	~NetworkBlocker() {
		running = false;
		if (hThread) {
			WaitForSingleObject(hThread, 5000);
			CloseHandle(hThread);
		}
	}
};
NetworkBlocker* g_blocker = nullptr;

// DLL entry
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lp_reserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(hModule);
		g_game_base = (uintptr_t)GetModuleHandle(NULL);
		g_blocker = new NetworkBlocker();
		//Sleep(30000);
		//if (InitBlockingHooks())
		CreateThread(NULL, 0, StartThread, NULL, 0, NULL);
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		if (g_blocker) {
			delete g_blocker;
			g_blocker = nullptr;
		}
		
		// flush + close dump file
		if (g_log_file) {
			fflush(g_log_file);
			int fd = _fileno(g_log_file);
			if (fd != -1) {
				intptr_t os_handle = _get_osfhandle(fd);
				if (os_handle != -1 && os_handle != (intptr_t)INVALID_HANDLE_VALUE) {
					FlushFileBuffers((HANDLE)os_handle);
				}
			}
			fclose(g_log_file);
			g_log_file = nullptr;
		}

		MH_Uninitialize();
	}
	return TRUE;
}
