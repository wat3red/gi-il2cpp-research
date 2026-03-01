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

void DisableLogReport()
{
	wchar_t filename[MAX_PATH] = {};
	GetModuleFileName(NULL, filename, MAX_PATH);

	auto path = std::filesystem::path(filename);
	path = path.parent_path() / (path.stem().string() + "_Data") / "Plugins";

	CreateFileW((path / "Astrolabe.dll").c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	CreateFileW((path / "MiHoYoMTRSDK.dll").c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
}

void InitConsole() {
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
}


// Thread entry: initialize console, open dump file, resolve function ptrs, hook
DWORD WINAPI StartThread(LPVOID)
{
	MH_Initialize();

	InitConsole();
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


//#pragma comment(lib, "Dbghelp.lib")
//#pragma comment(lib, "Psapi.lib")

static LONG g_inCrashHandler = 0;

// Exception codes worth logging (extend as needed)
static bool ShouldLog(DWORD code) {
	switch (code) {
	case EXCEPTION_ACCESS_VIOLATION:
	/*case EXCEPTION_ILLEGAL_INSTRUCTION:
	case EXCEPTION_STACK_OVERFLOW:
	case EXCEPTION_INT_DIVIDE_BY_ZERO:
	case EXCEPTION_PRIV_INSTRUCTION:
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:*/
	//case 0xE06D7363: // C++ exception (SEH wrapper)
		return true;
	default:
		return false;
	}
}

static const char* ExceptionName(DWORD code) {
	switch (code) {
	case EXCEPTION_ACCESS_VIOLATION:        return "ACCESS_VIOLATION";
	case EXCEPTION_ILLEGAL_INSTRUCTION:     return "ILLEGAL_INSTRUCTION";
	case EXCEPTION_STACK_OVERFLOW:          return "STACK_OVERFLOW";
	case EXCEPTION_INT_DIVIDE_BY_ZERO:      return "INT_DIVIDE_BY_ZERO";
	case EXCEPTION_PRIV_INSTRUCTION:        return "PRIV_INSTRUCTION";
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:   return "ARRAY_BOUNDS_EXCEEDED";
	case 0xE06D7363:                        return "CPP_EXCEPTION";
	default:                                return "UNKNOWN";
	}
}

// Resolve a VA to "ModuleName.dll+0xOFFSET [symbol+disp] (file:line)"
// For il2cpp, symbol resolution usually fails -> module+offset is the money shot
static void ResolveAddress(uintptr_t addr, char* out, size_t outSz) {
	// --- module + offset (always works) ---
	HMODULE hMod = nullptr;
	GetModuleHandleExA(
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
		GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		(LPCSTR)addr, &hMod);

	char modName[MAX_PATH] = "<unknown>";
	if (hMod) {
		GetModuleBaseNameA(GetCurrentProcess(), hMod, modName, sizeof(modName));
	}

	uintptr_t offset = hMod ? (addr - (uintptr_t)hMod) : addr;

	// --- symbol (works if PDB/export available) ---
	constexpr size_t kSymInfoSz = sizeof(SYMBOL_INFO) + MAX_SYM_NAME;
	alignas(SYMBOL_INFO) char symBuf[kSymInfoSz]{};
	auto* sym = reinterpret_cast<SYMBOL_INFO*>(symBuf);
	sym->SizeOfStruct = sizeof(SYMBOL_INFO);
	sym->MaxNameLen = MAX_SYM_NAME;

	DWORD64      disp64 = 0;
	DWORD        dispLn = 0;
	IMAGEHLP_LINE64 line{};
	line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

	bool hasSymbol = SymFromAddr(GetCurrentProcess(), (DWORD64)addr, &disp64, sym) != 0;
	bool hasLine = hasSymbol &&
		SymGetLineFromAddr64(GetCurrentProcess(), (DWORD64)addr, &dispLn, &line) != 0;

	if (hasLine)
		_snprintf_s(out, outSz, _TRUNCATE,
			"%s+0x%llX  [%s+0x%llX]  (%s:%lu)",
			modName, (unsigned long long)offset,
			sym->Name, (unsigned long long)disp64,
			line.FileName, line.LineNumber);
	else if (hasSymbol)
		_snprintf_s(out, outSz, _TRUNCATE,
			"%s+0x%llX  [%s+0x%llX]",
			modName, (unsigned long long)offset,
			sym->Name, (unsigned long long)disp64);
	else
		_snprintf_s(out, outSz, _TRUNCATE,
			"%s+0x%llX",
			modName, (unsigned long long)offset);
}

LONG WINAPI VectoredHandler(PEXCEPTION_POINTERS pEx) {
	DWORD code = pEx->ExceptionRecord->ExceptionCode;

	if (!ShouldLog(code))
		return EXCEPTION_CONTINUE_SEARCH;

	// Re-entrancy guard (e.g. stack overflow inside this handler)
	if (InterlockedCompareExchange(&g_inCrashHandler, 1, 0) != 0)
		return EXCEPTION_CONTINUE_SEARCH;

	CONTEXT ctx = *pEx->ContextRecord; // local copy so StackWalk64 can mutate it

	// ── Header ──────────────────────────────────────────────────────────────
	Log("========== CRASH ==========\n");
	Log("Exception : 0x%08X (%s)\n", code, ExceptionName(code));
	Log("Address   : 0x%p\n", (void*)pEx->ExceptionRecord->ExceptionAddress);

	if (code == EXCEPTION_ACCESS_VIOLATION && pEx->ExceptionRecord->NumberParameters >= 2) {
		const char* op = (pEx->ExceptionRecord->ExceptionInformation[0] == 1) ? "write" : "read";
		Log("AV Detail : %s at 0x%p\n",
			op, (void*)pEx->ExceptionRecord->ExceptionInformation[1]);
	}

	// ── Registers (x64) ─────────────────────────────────────────────────────
	Log("Registers :\n");
	Log("  RAX=%016llX  RBX=%016llX  RCX=%016llX  RDX=%016llX\n",
		ctx.Rax, ctx.Rbx, ctx.Rcx, ctx.Rdx);
	Log("  RSI=%016llX  RDI=%016llX  RBP=%016llX  RSP=%016llX\n",
		ctx.Rsi, ctx.Rdi, ctx.Rbp, ctx.Rsp);
	Log("  R8 =%016llX  R9 =%016llX  R10=%016llX  R11=%016llX\n",
		ctx.R8, ctx.R9, ctx.R10, ctx.R11);
	Log("  R12=%016llX  R13=%016llX  R14=%016llX  R15=%016llX\n",
		ctx.R12, ctx.R13, ctx.R14, ctx.R15);
	Log("  RIP=%016llX  EFL=%08X\n", ctx.Rip, ctx.EFlags);

	// ── Stack trace ─────────────────────────────────────────────────────────
	Log("Stack trace:\n");

	STACKFRAME64 sf{};
	sf.AddrPC.Offset = ctx.Rip;
	sf.AddrPC.Mode = AddrModeFlat;
	sf.AddrFrame.Offset = ctx.Rbp;
	sf.AddrFrame.Mode = AddrModeFlat;
	sf.AddrStack.Offset = ctx.Rsp;
	sf.AddrStack.Mode = AddrModeFlat;

	HANDLE hProcess = GetCurrentProcess();
	HANDLE hThread = GetCurrentThread();

	// ── RIP=0 recovery ──────────────────────────────────────────────────────────
	// When a call through a null pointer fires, RIP=0 and the real return address
	// is the top of the stack (RSP+0). Seed the frame manually so StackWalk64
	// can unwind the actual call chain.
	if (sf.AddrPC.Offset == 0 && ctx.Rsp != 0) {
		uintptr_t retAddr = 0;
		SIZE_T bytesRead = 0;

		if (ReadProcessMemory(hProcess, (LPCVOID)ctx.Rsp, &retAddr, sizeof(retAddr), &bytesRead)
			&& bytesRead == sizeof(retAddr)
			&& retAddr != 0)
		{
			Log("  [RIP=0 recovery] return addr from RSP: 0x%016llX\n",
				(unsigned long long)retAddr);

			// Log frame 0 as the null call site context (RSP itself)
			char resolved[512];
			ResolveAddress(retAddr, resolved, sizeof(resolved));
			Log("  #0   0x%016llX  %s  <-- CALLER of null ptr\n",
				(unsigned long long)retAddr, resolved);

			// Advance past the return address and let StackWalk64 continue
			ctx.Rip = retAddr;
			ctx.Rsp += sizeof(uintptr_t);
			sf.AddrPC.Offset = retAddr;
			sf.AddrStack.Offset = ctx.Rsp;
			sf.AddrFrame.Offset = ctx.Rbp;
		}
		else {
			Log("  [RIP=0] Could not recover return address from RSP=0x%016llX\n",
				(unsigned long long)ctx.Rsp);
		}
	}
	// ── Normal walk from frame 1 onward ─────────────────────────────────────────
	constexpr int kMaxFrames = 62; // 64 total - 2 already handled above
	char resolved[512];

	for (int frame = 1; frame <= kMaxFrames; ++frame) {
		BOOL ok = StackWalk64(
			IMAGE_FILE_MACHINE_AMD64,
			hProcess, hThread,
			&sf, &ctx,
			nullptr,
			SymFunctionTableAccess64,
			SymGetModuleBase64,
			nullptr);

		if (!ok || sf.AddrPC.Offset == 0)
			break;

		ResolveAddress((uintptr_t)sf.AddrPC.Offset, resolved, sizeof(resolved));
		Log("  #%-2d  0x%016llX  %s\n",
			frame, (unsigned long long)sf.AddrPC.Offset, resolved);
	}

	Log("===========================\n");

	InterlockedExchange(&g_inCrashHandler, 0);
	return EXCEPTION_CONTINUE_SEARCH; // let the game/OS handle it normally
}
// DLL entry
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lp_reserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		SymInitialize(GetCurrentProcess(), NULL, TRUE);
		SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
		SymInitialize(GetCurrentProcess(),
			"srv*C:\\symbols*https://msdl.microsoft.com/download/symbols",
			TRUE);

		AddVectoredExceptionHandler(1, VectoredHandler);

		DisableThreadLibraryCalls(hModule);
		g_game_base = (uintptr_t)GetModuleHandle(NULL);
		g_blocker = new NetworkBlocker();
		CreateThread(NULL, 0, StartThread, NULL, 0, NULL);
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		SymCleanup(GetCurrentProcess());

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
