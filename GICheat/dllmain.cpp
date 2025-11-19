#include <stdint.h>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <dbghelp.h>
#include <psapi.h>

#pragma comment(lib, "dbghelp.lib")

#include "lib/minhook/include/MinHook.h"
#include "il2cpp_types.h"

struct Il2CppClassRaw {
	uint32_t parent_index;         // +0x00
	uint32_t name_index;           // +0x04
	uint32_t namespace_index;      // +0x08
};

uintptr_t g_base = 0;
typedef unsigned int uint32_t; // assume 32-bit unsigned int on your platform

// STRING CACHE
std::unordered_map<uint32_t, std::string> g_stringCache;
std::mutex g_cacheMutex;
void* il2cpp_defaults_corlib = 0;

// CLASS DUMPER
std::ofstream g_dumpFile;
std::mutex g_dumpMutex;

// ORIGINAL FUNCTION POINTERS
const char* (*o_DecrpytString)(uint32_t) = nullptr;
void (*o_ClassInit)(__int64, __int64) = nullptr;
void* (*Image_ClassFromName)(void* image, const char* namespaceName, const char* className) = nullptr;
char (*o_InitIl2Cpp)(char* a1) = nullptr;

MethodInfo* (*il2cpp_class_get_methods)(Il2CppClass* klass, void** iter);

extern "C" __declspec(noinline)
bool seh_read_memory(uintptr_t addr, void* out, size_t size)
{
	__try {
		memcpy(out, (void*)addr, size);
		return true;
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		return false;
	}
}

static bool g_symsInit = false;

void InitSymbols()
{
	if (!g_symsInit) {
		SymInitialize(GetCurrentProcess(), NULL, TRUE);
		g_symsInit = true;
	}
}

void PrintStackTrace()
{
	InitSymbols();

	HANDLE process = GetCurrentProcess();
	HANDLE thread = GetCurrentThread();

	CONTEXT context = {};
	RtlCaptureContext(&context);

	STACKFRAME64 frame = {};
	DWORD machine = IMAGE_FILE_MACHINE_AMD64;

	frame.AddrPC.Offset = context.Rip;
	frame.AddrPC.Mode = AddrModeFlat;
	frame.AddrFrame.Offset = context.Rbp;
	frame.AddrFrame.Mode = AddrModeFlat;
	frame.AddrStack.Offset = context.Rsp;
	frame.AddrStack.Mode = AddrModeFlat;

	g_dumpFile << "\n--- STACK TRACE ---\n";

	for (int i = 0; i < 64; i++)
	{
		if (!StackWalk64(machine, process, thread, &frame, &context, NULL,
			SymFunctionTableAccess64, SymGetModuleBase64, NULL))
			break;

		DWORD64 address = frame.AddrPC.Offset;
		if (!address)
			break;

		// ---- Get module base ----
		HMODULE hMod = (HMODULE)SymGetModuleBase64(process, address);

		char moduleName[MAX_PATH] = "<unknown>";
		DWORD64 moduleBase = 0;

		if (hMod) {
			moduleBase = (DWORD64)hMod;
			GetModuleBaseNameA(process, hMod, moduleName, MAX_PATH);
		}

		DWORD64 rva = address - moduleBase;

		// ---- Symbol resolution ----
		char buffer[sizeof(SYMBOL_INFO) + 256];
		PSYMBOL_INFO symbol = (PSYMBOL_INFO)buffer;
		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		symbol->MaxNameLen = 255;

		bool hasSymbol = SymFromAddr(process, address, 0, symbol);

		// ---- Output line ----
		g_dumpFile << "[" << i << "] "
			<< moduleName << " + 0x" << std::hex << rva
			<< " (0x" << address << ")";

		if (hasSymbol)
			g_dumpFile << " -> " << symbol->Name;

		g_dumpFile << std::endl;
	}

	g_dumpFile << "-------------------\n";
}

char __fastcall h_InitIl2Cpp(char* a1) {
	char otp = o_InitIl2Cpp(a1);
	g_dumpFile << "h_InitIl2Cpp\n";

	PrintStackTrace();
	return otp;
}

// HOOKED STRING DECRYPTION
const char* __fastcall h_DecrpytString(uint32_t stringIndex) {
	// Call original
	const char* result = o_DecrpytString(stringIndex);

	if (result && stringIndex != 0xFFFFFFFF) {
		std::lock_guard<std::mutex> lock(g_cacheMutex);

		// Cache the result
		if (g_stringCache.find(stringIndex) == g_stringCache.end()) {
			try {
				std::string str(result);
				g_stringCache[stringIndex] = str;
			}
			catch (...) {
				g_stringCache[stringIndex] = "<error>";
			}
		}
	}

	return result;
}

template<typename T>
bool SafeRead(uintptr_t addr, T& out)
{
	return seh_read_memory(addr, &out, sizeof(T));
}

std::string GetCachedString(uint32_t index) {
	std::lock_guard<std::mutex> lock(g_cacheMutex);
	auto it = g_stringCache.find(index);
	if (it != g_stringCache.end()) {
		return it->second;
	}
	return "";
}

// Decrypt string from raw metadata
std::string DecryptStringFromRaw(Il2CppClassRaw* raw, uint32_t offset, uint32_t xorKey) {
	if (!raw) return "<null_raw>";

	uintptr_t fieldAddr = (uintptr_t)raw + offset;
	uint32_t encryptedValue = 0;
	if (!SafeRead(fieldAddr, encryptedValue)) {
		return "<read_error>";
	}

	uint32_t xored = encryptedValue ^ xorKey;
	//if (xored != 0xFFFFFFFF && xored < 0x10000000) {
		// Try to get from cache first
	std::string cached = GetCachedString(xored);
	if (!cached.empty()) {
		return cached;
	}

	// Call the decryption function
	if (o_DecrpytString) {
		const char* decrypted = o_DecrpytString(xored);
		if (decrypted) {
			std::string result(decrypted);
			if (!result.empty() && result.length() < 200) {
				return result;
			}
		}
	}
	//}

	return "";
}


void DumpClassInfo(uintptr_t classPtr) {
	std::lock_guard<std::mutex> lock(g_dumpMutex);

	std::string className;
	std::string namespaceName;

	const char* directName = nullptr;

	Il2CppClassRaw* rawPtr = nullptr;
	SafeRead(classPtr + 0x10, rawPtr);

	className = DecryptStringFromRaw(rawPtr, 0x04, 0x15B8D04D);
	namespaceName = DecryptStringFromRaw(rawPtr, 0x08, 0x2E0C9972);

	// Construct full name
	std::string fullName;
	if (!className.empty()) {
		fullName = (namespaceName.empty() ? "" : namespaceName + "::") + className;
	}
	else {
		SafeRead(classPtr + 0x18, directName);
		fullName = (namespaceName.empty() ? "" : namespaceName + "::") + directName + "(failed to retrieve class name by index, fallback)";
	}

	//Sleep(1111111111111);

	printf("%p | %s\n", classPtr, fullName.c_str());

	int a = 0;
	std::cin >> a;

	// Write to file
	void* iter = nullptr;
	if (g_dumpFile.is_open()) {
		il2cpp_class_get_methods(classPtr, iter);

		g_dumpFile << "0x" << std::hex << classPtr << std::dec
			<< " | " << fullName
			<< "\n";
		g_dumpFile.flush();
	}
}

// CLASS INIT HOOK
void __fastcall h_ClassInit(__int64 a1, __int64 a2) {
	o_ClassInit(a1, a2);

	// Dump class info after initialization
	DumpClassInfo((uintptr_t)a1);
}

int Start() {
	AllocConsole();
	FILE* fOut = nullptr;
	FILE* fIn = nullptr;
	FILE* fErr = nullptr;
	freopen_s(&fOut, "CONOUT$", "w", stdout);
	freopen_s(&fIn, "CONIN$", "r", stdin);
	freopen_s(&fErr, "CONOUT$", "w", stderr);

	// make sure C++ iostreams use stdio and are in a good state
	std::ios::sync_with_stdio(true);
	std::cin.clear();
	std::cout.clear();
	std::cerr.clear();

	printf("=== IL2CPP Dumper ===\n\n");

	g_base = (uintptr_t)GetModuleHandle(NULL);
	printf("Game Base: 0x%p\n\n", (void*)g_base);

	// Open dump file
	g_dumpFile.open("il2cpp_complete_dump.txt", std::ios::out | std::ios::trunc);
	if (g_dumpFile.is_open()) {
		g_dumpFile << "IL2CPP Class Dump with Decrypted Namespaces\n";
		g_dumpFile << "===========================================\n\n";
	}

	if (MH_Initialize() != MH_OK) {
		printf("MinHook init failed!\n");
		return -1;
	}

	Image_ClassFromName = (decltype(Image_ClassFromName))(g_base + 0x43F250);
	il2cpp_class_get_methods = (decltype(il2cpp_class_get_methods))(g_base + 0x43A650);
	

	MH_CreateHook((LPVOID)(g_base + 0x45DC40), h_InitIl2Cpp, (void**)&o_InitIl2Cpp);

	// Hook string decryption function
	printf("\nHooking sub_438D10 (string decryption)...\n");
	MH_CreateHook((LPVOID)(g_base + 0x438D10), h_DecrpytString, (void**)&o_DecrpytString);

	// Hook class initialization
	printf("Hooking Class::Init...\n");

	MH_CreateHook((LPVOID)(g_base + 0x43C7A0), h_ClassInit, (void**)&o_ClassInit);

	MH_EnableHook(MH_ALL_HOOKS);

	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(hModule);
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Start, NULL, 0, NULL);
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		if (g_dumpFile.is_open()) {
			g_dumpFile.close();
		}
		MH_Uninitialize();
	}
	return TRUE;
}

