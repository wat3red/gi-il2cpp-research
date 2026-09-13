#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>

#include <tlhelp32.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <psapi.h>

#pragma comment(lib, "shlwapi.lib")

namespace {

bool HasAdminRights() {
	HANDLE token = nullptr;
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token))
		return false;

	TOKEN_ELEVATION elevation{};
	DWORD size = 0;
	const BOOL ok = GetTokenInformation(token, TokenElevation, &elevation, sizeof(elevation), &size);
	CloseHandle(token);
	return ok && elevation.TokenIsElevated;
}

bool RequestAdminRights() {
	wchar_t path[MAX_PATH] = {};
	if (!GetModuleFileNameW(nullptr, path, MAX_PATH))
		return false;

	int argc = 0;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

	std::wstring params;
	for (int i = 1; i < argc; ++i) {
		params += L'"';
		params += argv[i];
		params += L"\" ";
	}
	LocalFree(argv);

	SHELLEXECUTEINFOW sei{ sizeof(sei) };
	sei.lpVerb = L"runas";
	sei.lpFile = path;
	sei.lpParameters = params.c_str();
	sei.nShow = SW_NORMAL;
	return ShellExecuteExW(&sei) != 0;
}

DWORD FindProcessId(const std::string& process_name) {
	PROCESSENTRY32W entry{};
	entry.dwSize = sizeof(entry);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE)
		return 0;

	DWORD pid = 0;
	if (Process32FirstW(snapshot, &entry)) {
		do {
			if (process_name == entry.szExeFile) {
				pid = entry.th32ProcessID;
				break;
			}
		} while (Process32NextW(snapshot, &entry));
	}
	CloseHandle(snapshot);
	return pid;
}

bool InjectDLL(HANDLE process, const std::string& dll_path) {
	const SIZE_T size = dll_path.size() + 1;
	void* remote = VirtualAllocEx(process, nullptr, size, MEM_COMMIT, PAGE_READWRITE);
	if (!remote)
		return false;

	if (!WriteProcessMemory(process, remote, dll_path.c_str(), size, nullptr)) {
		VirtualFreeEx(process, remote, 0, MEM_RELEASE);
		return false;
	}

	const auto load_library = reinterpret_cast<LPTHREAD_START_ROUTINE>(
		GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA"));

	HANDLE thread = CreateRemoteThread(process, nullptr, 0, load_library, remote, 0, nullptr);
	if (!thread) {
		VirtualFreeEx(process, remote, 0, MEM_RELEASE);
		return false;
	}

	WaitForSingleObject(thread, INFINITE);
	CloseHandle(thread);
	VirtualFreeEx(process, remote, 0, MEM_RELEASE);
	return true;
}

bool LaunchAndInject(const std::string& game_path, const std::string& dll_path) {
	STARTUPINFOA si{ sizeof(si) };
	PROCESS_INFORMATION pi{};
	char* cmdline = _strdup(game_path.c_str());

	if (!CreateProcessA(nullptr, cmdline, nullptr, nullptr, FALSE,
			CREATE_SUSPENDED, nullptr, nullptr, &si, &pi)) {
		std::cout << "CreateProcess failed: " << GetLastError() << "\n";
		free(cmdline);
		return false;
	}

	std::cout << "Process created. PID: " << pi.dwProcessId << "\n";

	bool ok = InjectDLL(pi.hProcess, dll_path);
	if (ok) {
		std::cout << "DLL injected.\n";
		ResumeThread(pi.hThread);
	}
	else {
		std::cout << "Injection failed, terminating process.\n";
		TerminateProcess(pi.hProcess, 0);
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	free(cmdline);
	return ok;
}

// Telemetry / anti-cheat hosts blocked for the duration of the session.
const char* kBlockedDomains[] = {
	"ys-log-upload-os.hoyoverse.com",
	"osuspider.yuanshen.com",
	"dispatch-hk4e-global-os-euro.hoyoverse.com",
	"sdk-log-upload-os.hoyoverse.com",
	"minor-api-os.hoyoverse.com",
	"overseauspider.yuanshen.com",
};

// DLLs that are renamed out of the way so the client does not load them.
const char* kDisabledPlugins[] = {
	"HoYoNetworkSDK.dll",
	"telemetry.dll",
	"MiHoYoSDKUploader.dll",
	"Astrolabe.dll",
	"MiHoYoMTRSDK.dll",
	"HoYoSDKNetworkFallback.dll",
	"HoYoKProtect.sys",
};

bool ModifyHostsFile(bool restore) {
	char system_dir[MAX_PATH] = {};
	GetSystemDirectoryA(system_dir, MAX_PATH);
	const std::string hosts_path = std::string(system_dir) + "\\drivers\\etc\\hosts";

	std::ifstream in(hosts_path);
	std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	in.close();

	for (const char* domain : kBlockedDomains) {
		const std::string entry = std::string("127.0.0.1 ") + domain;
		if (!restore) {
			if (content.find(entry) == std::string::npos)
				content += "\n" + entry;
		}
		else {
			size_t pos;
			while ((pos = content.find(entry)) != std::string::npos) {
				size_t line_end = content.find('\n', pos);
				if (line_end == std::string::npos)
					line_end = content.size();
				content.erase(pos, line_end - pos + 1);
			}
		}
	}

	std::ofstream out(hosts_path);
	if (!out)
		return false;
	out << content;
	out.close();

	system("ipconfig /flushdns > nul 2>&1");
	return true;
}

void TogglePluginBackups(const std::string& game_dir, bool backup) {
	for (const char* file : kDisabledPlugins) {
		const std::string src = game_dir + "\\" + file;
		const std::string bak = src + ".bak";

		if (backup) {
			if (PathFileExistsA(src.c_str()) && !PathFileExistsA(bak.c_str()))
				MoveFileA(src.c_str(), bak.c_str());
		}
		else {
			if (PathFileExistsA(bak.c_str()))
				MoveFileA(bak.c_str(), src.c_str());
		}
	}
}

std::string DefaultGamePath() {
	// HoYoPlay default install. Override with argv[2].
	const char* candidates[] = {
		"C:\\Program Files\\HoYoPlay\\games\\Genshin Impact game\\GenshinImpact.exe",
		"D:\\HoYoPlay\\games\\Genshin Impact game\\GenshinImpact.exe",
	};
	for (const char* path : candidates) {
		if (PathFileExistsA(path))
			return path;
	}
	return candidates[0];
}

std::string DefaultDllPath() {
	// Prefer the DLL sitting next to the injector; fall back to the VS output.
	const std::filesystem::path self = std::filesystem::current_path();
	const std::filesystem::path next_to_self = self / "Cheat.dll";
	if (std::filesystem::exists(next_to_self))
		return next_to_self.string();

	const std::filesystem::path vs_out = self / "x64" / "Debug" / "Cheat.dll";
	if (std::filesystem::exists(vs_out))
		return vs_out.string();

	return next_to_self.string();
}

} // namespace

int main(int argc, char* argv[]) {
	if (!HasAdminRights()) {
		std::wcout << L"Requesting administrator privileges...\n";
		if (!RequestAdminRights()) {
			std::wcout << L"Failed to elevate. Run as administrator.\n";
			return 1;
		}
		return 0;
	}

	std::string dll_path = argc > 1 ? argv[1] : DefaultDllPath();
	std::string game_path = argc > 2 ? argv[2] : DefaultGamePath();

	std::cout << "Game: " << game_path << "\n";
	std::cout << "DLL:  " << dll_path << "\n";

	if (!ModifyHostsFile(false)) {
		std::cout << "Failed to modify hosts file.\n";
		return 1;
	}

	const std::string game_dir = std::filesystem::path(game_path).parent_path().string();
	TogglePluginBackups(game_dir, true);

	// Remove the kernel driver if present (best-effort).
	{
		char system_dir[MAX_PATH] = {};
		GetSystemDirectoryA(system_dir, MAX_PATH);
		const std::string driver = std::string(system_dir) + "\\drivers\\HoYoKProtect.sys";
		if (!DeleteFileA(driver.c_str()) && GetLastError() != ERROR_FILE_NOT_FOUND)
			std::cout << "Warning: could not delete HoYoKProtect.sys (" << GetLastError() << ")\n";
	}

	const bool launched = LaunchAndInject(game_path, dll_path);

	// Always put hosts + plugin files back.
	ModifyHostsFile(true);
	TogglePluginBackups(game_dir, false);

	return launched ? 0 : 1;
}
