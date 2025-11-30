#include <windows.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <tlhelp32.h>
#include <shellapi.h>

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

class RegistryManager {
public:
	static bool ModifyRegistry() {
		HKEY hKey;
		DWORD dwValue = 0;
		LONG lResult;

		// Open or create the registry key
		lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
			L"SYSTEM\\ControlSet001\\Services\\HoYoProtect",
			0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);

		if (lResult != ERROR_SUCCESS) {
			std::wcout << L"Failed to open registry key. Error: " << lResult << std::endl;
			return false;
		}

		// Set ErrorControl to 0
		lResult = RegSetValueEx(hKey, L"ErrorControl", 0, REG_DWORD,
			(const BYTE*)&dwValue, sizeof(dwValue));
		if (lResult != ERROR_SUCCESS) {
			std::wcout << L"Failed to set ErrorControl. Error: " << lResult << std::endl;
			RegCloseKey(hKey);
			return false;
		}

		// Set Start to 0
		lResult = RegSetValueEx(hKey, L"Start", 0, REG_DWORD,
			(const BYTE*)&dwValue, sizeof(dwValue));
		if (lResult != ERROR_SUCCESS) {
			std::wcout << L"Failed to set Start. Error: " << lResult << std::endl;
			RegCloseKey(hKey);
			return false;
		}

		// Set Type to 0
		lResult = RegSetValueEx(hKey, L"Type", 0, REG_DWORD,
			(const BYTE*)&dwValue, sizeof(dwValue));
		if (lResult != ERROR_SUCCESS) {
			std::wcout << L"Failed to set Type. Error: " << lResult << std::endl;
			RegCloseKey(hKey);
			return false;
		}

		RegCloseKey(hKey);
		std::wcout << L"Registry modifications completed successfully." << std::endl;
		return true;
	}
};

class ProcessManager {
public:
	static bool StartProcessAndInjectDLL(const std::wstring& processPath, const std::wstring& dllPath) {
		STARTUPINFO si = { sizeof(si) };
		PROCESS_INFORMATION pi;

		// Create the process suspended
		if (CreateProcess(processPath.c_str(), NULL, NULL, NULL, FALSE,
			CREATE_SUSPENDED, NULL, NULL, &si, &pi)) {

			std::wcout << L"Process created successfully. PID: " << pi.dwProcessId << std::endl;

			// Inject the DLL
			if (InjectDLL(pi.hProcess, pi.dwThreadId, dllPath)) {
				std::wcout << L"DLL injected successfully." << std::endl;

				// Resume the process
				ResumeThread(pi.hThread);

				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);

				return true;
			}
			else {
				std::wcout << L"Failed to inject DLL. Terminating process." << std::endl;
				TerminateProcess(pi.hProcess, 0);
				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);
				return false;
			}
		}
		else {
			std::wcout << L"Failed to create process. Error: " << GetLastError() << std::endl;
			return false;
		}
	}

private:
	static bool InjectDLL(HANDLE hProcess, DWORD threadId, const std::wstring& dllPath) {
		// Convert wide string to multi-byte for LoadLibraryA
		int dllPathLen = WideCharToMultiByte(CP_UTF8, 0, dllPath.c_str(), -1, NULL, 0, NULL, NULL);
		std::string dllPathA(dllPathLen, 0);
		WideCharToMultiByte(CP_UTF8, 0, dllPath.c_str(), -1, &dllPathA[0], dllPathLen, NULL, NULL);

		// Allocate memory in the target process
		LPVOID pRemoteMemory = VirtualAllocEx(hProcess, NULL, dllPathA.length(),
			MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

		if (!pRemoteMemory) {
			std::wcout << L"Failed to allocate memory in target process." << std::endl;
			return false;
		}

		// Write DLL path to allocated memory
		if (!WriteProcessMemory(hProcess, pRemoteMemory, dllPathA.c_str(), dllPathA.length(), NULL)) {
			std::wcout << L"Failed to write DLL path to target process." << std::endl;
			VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
			return false;
		}

		// Get address of LoadLibraryA in kernel32.dll
		LPVOID pLoadLibrary = (LPVOID)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA");
		if (!pLoadLibrary) {
			std::wcout << L"Failed to get LoadLibraryA address." << std::endl;
			VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
			return false;
		}

		// Create remote thread to load the DLL
		HANDLE hRemoteThread = CreateRemoteThread(hProcess, NULL, 0,
			(LPTHREAD_START_ROUTINE)pLoadLibrary, pRemoteMemory, 0, NULL);

		if (!hRemoteThread) {
			std::wcout << L"Failed to create remote thread." << std::endl;
			VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);
			return false;
		}

		// Wait for the thread to complete
		WaitForSingleObject(hRemoteThread, INFINITE);

		// Cleanup
		CloseHandle(hRemoteThread);
		VirtualFreeEx(hProcess, pRemoteMemory, 0, MEM_RELEASE);

		return true;
	}
};

class GameLauncher {
private:
	std::wstring m_gamePath;
	std::wstring m_dllPath;

public:
	GameLauncher(const std::wstring& game, const std::wstring& dll)
		: m_gamePath(game), m_dllPath(dll) {
	}

	bool Launch() {
		// Step 2: Launch game with DLL injection
		std::wcout << L"[*] Launching Genshin Impact..." << std::endl;
		if (!ProcessManager::StartProcessAndInjectDLL(m_gamePath, m_dllPath)) {
			std::wcout << L"Failed to launch game." << std::endl;
			return false;
		}

		std::wcout << L"[*] Done." << std::endl;
		return true;
	}
};

int wmain(int argc, wchar_t* argv[]) {
	std::wcout << L"Genshin Impact Launcher with DLL Injection" << std::endl;
	std::wcout << L"==========================================" << std::endl;

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

	// Modify registry
	std::wcout << L"Modifying registry settings..." << std::endl;
	if (!RegistryManager::ModifyRegistry()) {
		std::wcout << L"Warning: Failed to modify registry settings." << std::endl;
	}

	// Configuration - you can modify these paths or make them command line arguments
	std::wstring gamePath = L"D:\\HoYoPlay\\games\\Genshin Impact game\\GenshinImpact.exe";
	std::wstring dllPath = L"D:\\Desktop\\GICheat\\x64\\Debug\\Cheat.dll";

	// Use command line arguments if provided
	if (argc > 1) dllPath = argv[1];
	if (argc > 2) gamePath = argv[2];

	// Launch the game
	GameLauncher launcher(gamePath, dllPath);
	if (launcher.Launch()) {
		std::wcout << L"Game launched successfully!" << std::endl;
	}
	else {
		std::wcout << L"Failed to launch game." << std::endl;
		return 1;
	}

	return 0;
}
