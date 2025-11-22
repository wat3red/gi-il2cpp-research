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
        if (GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath))) {
            SHELLEXECUTEINFO sei = { sizeof(sei) };
            sei.lpVerb = L"runas";
            sei.lpFile = szPath;
            sei.hwnd = NULL;
            sei.nShow = SW_NORMAL;

            // Pass command line arguments
            std::wstring parameters = GetCommandLineW();
            if (parameters.length() > 0) {
                sei.lpParameters = parameters.c_str() + wcslen(szPath) + 2;
            }

            if (ShellExecuteEx(&sei)) {
                return true;
            }
        }
        return false;
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

class NetworkManager {
public:
    static bool SetInterfaceState(const std::wstring& adapterName, bool enable) {
        // netsh interface set interface "Wi-Fi" admin=enabled
        std::wstring command = L"netsh interface set interface \"" + adapterName +
            L"\" admin=" + (enable ? L"enabled" : L"disabled");

        return ExecuteCommand(command);
    }

private:
    static bool ExecuteCommand(const std::wstring& command) {
        STARTUPINFO si = { sizeof(si) };
        PROCESS_INFORMATION pi;

        // Create a command line
        std::wstring cmdLine = L"cmd.exe /c " + command;

        if (CreateProcess(NULL, &cmdLine[0], NULL, NULL, FALSE,
            CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {

            WaitForSingleObject(pi.hProcess, INFINITE);

            DWORD exitCode;
            GetExitCodeProcess(pi.hProcess, &exitCode);

            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            return (exitCode == 0);
        }

        return false;
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
    std::wstring m_adapterName;
    std::wstring m_gamePath;
    std::wstring m_dllPath;

public:
    GameLauncher(const std::wstring& adapter, const std::wstring& game, const std::wstring& dll)
        : m_adapterName(adapter), m_gamePath(game), m_dllPath(dll) {
    }

    bool Launch() {
        // Step 1: Disable internet
        std::wcout << L"[*] Disabling internet..." << std::endl;
        if (!NetworkManager::SetInterfaceState(m_adapterName, false)) {
            std::wcout << L"Failed to disable network adapter." << std::endl;
            return false;
        }

        // Step 2: Launch game with DLL injection
        std::wcout << L"[*] Launching Genshin Impact..." << std::endl;
        if (!ProcessManager::StartProcessAndInjectDLL(m_gamePath, m_dllPath)) {
            std::wcout << L"Failed to launch game. Re-enabling internet..." << std::endl;
            NetworkManager::SetInterfaceState(m_adapterName, true);
            return false;
        }

        // Step 3: Random delay between 4 and 7 seconds
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        int delay = (std::rand() % 4) + 4;
        std::wcout << L"[*] Waiting " << delay << L" seconds before reconnecting internet..." << std::endl;

        Sleep(delay * 1000);

        // Step 4: Re-enable internet
        std::wcout << L"[*] Re-enabling internet..." << std::endl;
        if (!NetworkManager::SetInterfaceState(m_adapterName, true)) {
            std::wcout << L"Warning: Failed to re-enable network adapter." << std::endl;
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
    std::wstring adapterName = L"Wi-Fi";
    std::wstring gamePath = L"D:\\HoYoPlay\\games\\Genshin Impact game\\GenshinImpact.exe";
    std::wstring dllPath = L"D:\\Desktop\\GICheat\\x64\\Debug\\GICheat.dll"; // Change this to your DLL path

    // Use command line arguments if provided
    if (argc > 1) dllPath = argv[1];
    if (argc > 2) gamePath = argv[2];
    if (argc > 3) adapterName = argv[3];

    // Launch the game
    GameLauncher launcher(adapterName, gamePath, dllPath);
    if (launcher.Launch()) {
        std::wcout << L"Game launched successfully!" << std::endl;
    }
    else {
        std::wcout << L"Failed to launch game." << std::endl;
        return 1;
    }

    //std::wcout << L"Press any key to exit..." << std::endl;
    //std::cin.get();

    return 0;
}