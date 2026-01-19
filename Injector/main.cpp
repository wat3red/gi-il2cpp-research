#include <windows.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <tlhelp32.h>
#include <shellapi.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <iphlpapi.h>
#include <tlhelp32.h>
#include <shlwapi.h>
#include <psapi.h>
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "shlwapi.lib")

class AdminPrivileges {
public:
	static bool HasAdminRights() {
		HANDLE hToken = NULL;
		if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
			TOKEN_ELEVATION elevation;
			DWORD dwSize;
			if (GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &dwSize)) {
				CloseHandle(hToken);
				return elevation.TokenIsElevated;
			}
			CloseHandle(hToken);
		}
		return false;
	}

	static bool RequestAdminRights() {
		wchar_t szPath[MAX_PATH];
		if (!GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath)))
			return false;

		int argc;
		LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

		std::wstring params;
		for (int i = 1; i < argc; i++) {
			params += L"\"";
			params += argv[i];
			params += L"\" ";
		}

		LocalFree(argv);

		SHELLEXECUTEINFO sei = { sizeof(sei) };
		sei.lpVerb = L"runas";
		sei.lpFile = szPath;
		sei.lpParameters = params.c_str();
		sei.nShow = SW_NORMAL;

		return ShellExecuteEx(&sei);
	}

};

class GenshinBypass {
private:
	std::string gamePath;
	std::string dllPath;
	std::vector<std::string> filesToBackup = {
		"HoYoNetworkSDK.dll",
		"telemetry.dll",
		"MiHoYoSDKUploader.dll",
		"Astrolabe.dll",
		"MiHoYoMTRSDK.dll",
		"HoYoSDKNetworkFallback.dll",
		"HoYoKProtect.sys"
	};
	std::vector<std::string> blockedDomains = {
		"ys-log-upload-os.hoyoverse.com",
		"osuspider.yuanshen.com",
		"dispatch-hk4e-global-os-euro.hoyoverse.com",
		"sdk-log-upload-os.hoyoverse.com",
		"minor-api-os.hoyoverse.com",
		"overseauspider.yuanshen.com"
		//"https://sdk-log-upload-os.hoyoverse.com/sdk/dataUpload",
		//"https://ys-log-upload-os.hoyoverse.com/sdk/dataUpload",
		//"https://minor-api-os.hoyoverse.com/common/h5log/log/batch",
		//"https://minor-api-os.hoyoverse.com/common/h5log/log/batch?topic=plat_explog_sdk_v2"
	};

	bool IsAdmin() {
		BOOL isAdmin = FALSE;
		PSID adminGroup = NULL;
		SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;

		if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
			DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup)) {
			if (!CheckTokenMembership(NULL, adminGroup, &isAdmin)) {
				isAdmin = FALSE;
			}
			FreeSid(adminGroup);
		}
		return isAdmin != FALSE;
	}

	bool ModifyHostsFile(bool restore = false) {
		char systemDir[MAX_PATH];
		GetSystemDirectoryA(systemDir, MAX_PATH);
		std::string hostsPath = std::string(systemDir) + "\\drivers\\etc\\hosts";

		std::ifstream hostsIn(hostsPath);
		std::string content((std::istreambuf_iterator<char>(hostsIn)), std::istreambuf_iterator<char>());
		hostsIn.close();

		if (!restore) {
			// Add blocking entries
			for (const auto& domain : blockedDomains) {
				if (content.find("127.0.0.1 " + domain) == std::string::npos) {
					content += "\n127.0.0.1 " + domain;
				}
			}
		}
		else {
			// Remove blocking entries
			for (const auto& domain : blockedDomains) {
				size_t pos;
				while ((pos = content.find("127.0.0.1 " + domain)) != std::string::npos) {
					size_t lineEnd = content.find('\n', pos);
					if (lineEnd == std::string::npos) lineEnd = content.length();
					content.erase(pos, lineEnd - pos + 1);
				}
			}
		}

		std::ofstream hostsOut(hostsPath);
		if (!hostsOut) return false;
		hostsOut << content;
		hostsOut.close();

		// Flush DNS
		system("ipconfig /flushdns > nul 2>&1");
		return true;
	}

	bool DeleteDriver() {
		char systemDir[MAX_PATH];
		GetSystemDirectoryA(systemDir, MAX_PATH);
		std::string driverPath = std::string(systemDir) + "\\drivers\\HoYoKProtect.sys";
		return DeleteFileA(driverPath.c_str()) != 0 || GetLastError() == ERROR_FILE_NOT_FOUND;
	}

	bool BackupFiles(bool backup) {
		for (const auto& file : filesToBackup) {
			std::string srcPath = gamePath + "\\" + file;
			std::string bakPath = srcPath + ".bak";

			if (backup) {
				if (PathFileExistsA(srcPath.c_str()) && !PathFileExistsA(bakPath.c_str())) {
					MoveFileA(srcPath.c_str(), bakPath.c_str());
				}
			}
			else {
				if (PathFileExistsA(bakPath.c_str())) {
					MoveFileA(bakPath.c_str(), srcPath.c_str());
				}
			}
		}
		return true;
	}

	DWORD FindProcessId(const std::string& processName) {
		PROCESSENTRY32 pe32;
		pe32.dwSize = sizeof(PROCESSENTRY32);
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnapshot == INVALID_HANDLE_VALUE) return 0;

		if (Process32FirstW(hSnapshot, &pe32)) {
			do {
				if (strcmp((char*)pe32.szExeFile, processName.c_str()) == 0) {
					CloseHandle(hSnapshot);
					return pe32.th32ProcessID;
				}
			} while (Process32Next(hSnapshot, &pe32));
		}
		CloseHandle(hSnapshot);
		return 0;
	}

	bool InjectDLL(HANDLE hProcess, DWORD threadId, const std::string& dllPath) {
		LPVOID pDllPath = VirtualAllocEx(hProcess, 0, dllPath.length() + 1, MEM_COMMIT, PAGE_READWRITE);
		if (!pDllPath) {
			return false;
		}

		if (!WriteProcessMemory(hProcess, pDllPath, dllPath.c_str(), dllPath.length() + 1, NULL)) {
			VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
			return false;
		}

		HMODULE hKernel32 = GetModuleHandleA("kernel32.dll");
		FARPROC pLoadLibrary = GetProcAddress(hKernel32, "LoadLibraryA");

		HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pLoadLibrary, pDllPath, 0, NULL);
		if (!hThread) {
			VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
			return false;
		}

		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);
		VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
		return true;
	}

	bool StartProcessAndInjectDLL(const std::string& processPath, const std::string& dllPath) {
		STARTUPINFOA si = { sizeof(si) };
		PROCESS_INFORMATION pi;

		// Convert string to char* for CreateProcessA
		char* cmdLine = _strdup(processPath.c_str());

		// Create the process suspended
		if (CreateProcessA(NULL, cmdLine, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi)) {
			std::cout << "Process created successfully. PID: " << pi.dwProcessId << std::endl;

			// Inject the DLL
			if (InjectDLL(pi.hProcess, pi.dwThreadId, dllPath)) {
				std::cout << "DLL injected successfully." << std::endl;

				// Resume the process
				ResumeThread(pi.hThread);

				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);
				free(cmdLine);
				return true;
			}
			else {
				std::cout << "Failed to inject DLL. Terminating process." << std::endl;
				TerminateProcess(pi.hProcess, 0);
				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);
				free(cmdLine);
				return false;
			}
		}
		else {
			std::cout << "Failed to create process. Error: " << GetLastError() << std::endl;
			free(cmdLine);
			return false;
		}
	}

public:
	GenshinBypass(const std::string& gamePath, const std::string& dllPath)
		: gamePath(gamePath), dllPath(dllPath) {
	}

	bool ApplyBypass() {
		if (!IsAdmin()) {
			std::cout << "Administrator privileges required!" << std::endl;
			return false;
		}

		std::cout << "Applying bypass..." << std::endl;

		if (!ModifyHostsFile(false)) {
			std::cout << "Failed to modify hosts file!" << std::endl;
			return false;
		}

		if (!DeleteDriver()) {
			std::cout << "Failed to delete driver!" << std::endl;
			return false;
		}

		BackupFiles(true);

		std::cout << "Launching game and injecting DLL..." << std::endl;
		if (!StartProcessAndInjectDLL(gamePath, dllPath)) {
			std::cout << "Failed to launch game or inject DLL!" << std::endl;
			return false;
		}

		std::cout << "Bypass applied successfully!" << std::endl;
		return true;
	}

	bool RestoreSystem() {
		std::cout << "Restoring system..." << std::endl;

		ModifyHostsFile(true);
		BackupFiles(false);

		std::cout << "System restored!" << std::endl;
		return true;
	}
};

int main(int argc, char* argv[]) {
	/*if (argc < 3) {
		std::cout << "Usage: injector.exe <game_path> <dll_path>" << std::endl;
		return 1;
	}*/

	// Check if we have admin privileges
	if (!AdminPrivileges::HasAdminRights()) {
		std::wcout << L"Requesting administrative privileges..." << std::endl;
		if (!AdminPrivileges::RequestAdminRights()) {
			std::wcout << L"Failed to get admin privileges. Please run as administrator." << std::endl;
			return 1;
		}
		return 0; // Current instance will exit, new elevated instance will run
	}

	std::wcout << L"Running with administrative privileges." << std::endl;

	std::string gamePath = "D:\\HoYoPlay\\games\\Genshin Impact game\\GenshinImpact.exe";
	std::string dllPath = "D:\\Desktop\\GICheat\\x64\\Debug\\Cheat.dll";

	// Use command line arguments if provided
	if (argc > 1) dllPath = argv[1];
	if (argc > 2) gamePath = argv[2];

	GenshinBypass bypass(gamePath, dllPath);

	if (!bypass.ApplyBypass()) {
		bypass.RestoreSystem();
		return 1;
	}

	//std::cout << "Press any key to restore..." << std::endl;
	//std::cin.get();

	bypass.RestoreSystem();
	return 0;
}
