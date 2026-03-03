// dllmain.cpp
#include "common.h"
#include "utils.h"
#include "hooks.h"
#include "il2cpp_bridge.h"
#include "dumper.h"
#include <thread>


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

DWORD WINAPI StartThread(LPVOID) {
	Utils::CreateConsole();
	Utils::DisableLogReport();

	Config::GameBase = (uintptr_t)GetModuleHandle(NULL);
	Utils::Log("Game Base: 0x%p\n", (void*)Config::GameBase);

	// Initialize API pointers
	Il2Cpp::Initialize();

	// Start packet unblocking timer
	/*std::thread blockPacketsThread([]() {
		Sleep(10000);
		Config::BlockPackets = false;
		Utils::Log("Packets unblocked.\n");
		});
	blockPacketsThread.detach();*/

	// Wait for Unity Window
	while (!FindWindowA("UnityWndClass", nullptr)) {
		Sleep(100);
	}

	Sleep(15000); // Wait for game initialization
	Utils::Log("Starting dump...\n");

	// Choose what to dump here
	//Dumper::DumpFull();
	//Dumper::GenerateSDK();
	Dumper::DumpForIDA();

	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(hModule);
		g_blocker = new NetworkBlocker();

		if (Hooks::Init()) {
			CreateThread(NULL, 0, StartThread, NULL, 0, NULL);
		}
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		Hooks::Uninit();

		if (g_blocker) {
			delete g_blocker;
			g_blocker = nullptr;
		}
	}
	return TRUE;
}