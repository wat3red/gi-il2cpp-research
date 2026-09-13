#pragma once

#include <winsock2.h>
#include <windows.h>
#include <string>
#include <vector>

// Polls the TCP table and tears down connections to known telemetry /
// anti-cheat sink addresses. Shared by Cheat and Dumper so the logic
// lives in one place instead of being copy-pasted.
class NetworkBlocker {
public:
	NetworkBlocker();
	~NetworkBlocker();

	NetworkBlocker(const NetworkBlocker&) = delete;
	NetworkBlocker& operator=(const NetworkBlocker&) = delete;

private:
	bool IsBlocked(DWORD remote_addr, unsigned short remote_port) const;
	void MonitorConnections();
	static DWORD WINAPI MonitorThread(LPVOID param);

	HANDLE thread_ = nullptr;
	bool running_ = false;
	std::vector<std::string> blocked_ips_;
};
