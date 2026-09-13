#include "network_blocker.h"

#include <ws2tcpip.h>
#include <iphlpapi.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

namespace {

// Known log-upload / anti-cheat endpoints observed during research.
// Not a complete list — only the ones that actually mattered for dumps.
constexpr const char* kBlockedIps[] = {
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
	"43.152.42.60",
	"163.181.92.201",
	"47.245.154.152",
	"47.245.155.149",
};

bool IsBlockedPort(unsigned short port) {
	// High ephemeral range used by the game's UDP/relay traffic.
	return port >= 10000 && port <= 65000;
}

} // namespace

NetworkBlocker::NetworkBlocker() : running_(true) {
	blocked_ips_.assign(std::begin(kBlockedIps), std::end(kBlockedIps));
	thread_ = CreateThread(nullptr, 0, MonitorThread, this, 0, nullptr);
}

NetworkBlocker::~NetworkBlocker() {
	running_ = false;
	if (thread_) {
		WaitForSingleObject(thread_, 5000);
		CloseHandle(thread_);
		thread_ = nullptr;
	}
}

bool NetworkBlocker::IsBlocked(DWORD remote_addr, unsigned short remote_port) const {
	char ip[INET_ADDRSTRLEN] = {};
	inet_ntop(AF_INET, &remote_addr, ip, INET_ADDRSTRLEN);

	for (const auto& blocked : blocked_ips_) {
		if (blocked == ip)
			return true;
	}

	return IsBlockedPort(remote_port);
}

DWORD WINAPI NetworkBlocker::MonitorThread(LPVOID param) {
	static_cast<NetworkBlocker*>(param)->MonitorConnections();
	return 0;
}

void NetworkBlocker::MonitorConnections() {
	while (running_) {
		DWORD size = 0;
		if (GetTcpTable2(nullptr, &size, TRUE) != ERROR_INSUFFICIENT_BUFFER) {
			Sleep(1000);
			continue;
		}

		auto* table = static_cast<MIB_TCPTABLE2*>(malloc(size));
		if (!table) {
			Sleep(1000);
			continue;
		}

		if (GetTcpTable2(table, &size, TRUE) == NO_ERROR) {
			for (DWORD i = 0; i < table->dwNumEntries; ++i) {
				const MIB_TCPROW2& row = table->table[i];
				if (!IsBlocked(row.dwRemoteAddr, ntohs(static_cast<u_short>(row.dwRemotePort))))
					continue;

				MIB_TCPROW kill{};
				kill.dwState = MIB_TCP_STATE_DELETE_TCB;
				kill.dwLocalAddr = row.dwLocalAddr;
				kill.dwLocalPort = row.dwLocalPort;
				kill.dwRemoteAddr = row.dwRemoteAddr;
				kill.dwRemotePort = row.dwRemotePort;
				SetTcpEntry(&kill);
			}
		}

		free(table);
		Sleep(100);
	}
}
