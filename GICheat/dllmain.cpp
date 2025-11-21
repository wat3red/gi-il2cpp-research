// dllmain.cpp

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

#include "lib/minhook/include/MinHook.h"
#include "il2cpp_types.h"

#pragma comment(lib, "dbghelp.lib")

// Globals
uintptr_t g_base = 0;
FILE* g_dumpFile = nullptr;
std::mutex g_dumpMutex;

// Function pointers
void (*o_ClassInit)(__int64, __int64) = nullptr;
MethodInfo* (*il2cpp_class_get_methods)(Il2CppClass* klass, void** iter) = nullptr;
const char* (*il2cpp_class_get_name)(Il2CppClass* klass) = nullptr;
const char* (*il2cpp_class_get_namespace)(Il2CppClass* klass) = nullptr;
const char* (*il2cpp_method_get_name)(MethodInfo* method) = nullptr;
const char* (*il2cpp_method_get_param_name)(MethodInfo* method, uint32_t index) = nullptr;
uintptr_t(*DecryptParameters)(__int64 method) = nullptr;  // sub_451910
Il2CppClass* (*Class_FromIl2CppType)(const Il2CppType* type) = nullptr;

// Logging helper: thread-safe, flushes libc buffers and OS buffers to disk
void Log(const char* fmt, ...)
{
	static size_t counter = 0;

	std::lock_guard<std::mutex> lock(g_dumpMutex);
	if (!g_dumpFile) return;

	va_list ap;
	va_start(ap, fmt);
	vfprintf(g_dumpFile, fmt, ap);
	va_end(ap);

	if (++counter % 200 == 0)   // flush only every 200 lines
		fflush(g_dumpFile);
}

void DumpClassInfo(Il2CppClass* classPtr) {
	std::string className = il2cpp_class_get_name(classPtr);
	std::string namespaceName = il2cpp_class_get_namespace(classPtr);

	Log("\n// Namespace: %s\nclass %s \n{\n", namespaceName.c_str(), className.c_str());

	// sizeof MethodInfo = 0x38
	void* iter = nullptr;
	while (MethodInfo* method = il2cpp_class_get_methods(classPtr, &iter)) {
		uint8_t paramCount = *(uint8_t*)((uintptr_t)method + 0x2E);

		std::string paramList = "";
		if (paramCount) {
			uintptr_t decryptedParams = DecryptParameters((uintptr_t)method);
			uintptr_t paramArray = decryptedParams + 0x8;

			for (uint8_t i = 0; i < paramCount; i++) {
				// Each parameter is 3 pointers (0x18 bytes)
				uintptr_t param = paramArray + ((i - 1) * 0x18);
				if (!param) continue;

				const Il2CppType* paramType = *(Il2CppType**)(paramArray + (i * 0x18));
				if (!paramType) continue;

				const char* paramName = *(const char**)((uintptr_t)param + 0x10);
				if (!paramName) continue;

				Il2CppClass* paramClass = Class_FromIl2CppType(paramType);
				if (!paramClass) continue;

				paramList += il2cpp_class_get_name(paramClass) + std::string(" ") + (std::string)paramName + (i == paramCount - 1 ? "" : ", ");
			}
		}

		Log("\t%s(%s) // RVA: %X\n", il2cpp_method_get_name(method), paramList.c_str(), (uintptr_t)method->methodPointer - g_base);
	}
}

void __fastcall h_ClassInit(__int64 a1, __int64 a2)
{
	// Call original

	o_ClassInit(a1, a2);


	// Dump class info after initialization
	DumpClassInfo((Il2CppClass*)a1);
}

// Thread entry: initialize console, open dump file, resolve function ptrs, hook
DWORD WINAPI StartThread(LPVOID)
{
	AllocConsole();
	FILE* fOut = nullptr;
	FILE* fIn = nullptr;
	FILE* fErr = nullptr;
	freopen_s(&fOut, "CONOUT$", "w", stdout);
	freopen_s(&fIn, "CONIN$", "r", stdin);
	freopen_s(&fErr, "CONOUT$", "w", stderr);
	std::ios::sync_with_stdio(true);
	std::cin.clear();
	std::cout.clear();
	std::cerr.clear();

	printf("=== IL2CPP Dumper ===\n\n");
	g_base = (uintptr_t)GetModuleHandle(NULL);
	printf("Game Base: 0x%p\n\n", (void*)g_base);

	// open dump file (unbuffered to minimize lost data). If you want performance,
	// change setvbuf call to _IOLBF or remove it and rely on manual flushes.
	fopen_s(&g_dumpFile, "il2cpp_complete_dump.txt", "w");
	if (!g_dumpFile) {
		printf("Failed to open dump file!\n");
	}
	else {
		// disable stdio buffering (safe but slower). Change to _IOLBF for line buffering.
		setvbuf(g_dumpFile, NULL, _IONBF, 0);
		Log("[*] Dump file opened. Game Base: 0x%p\n", (void*)g_base);
	}

	if (MH_Initialize() != MH_OK) {
		printf("MinHook init failed!\n");
		return 1;
	}

	// Resolve known offsets (update offsets for your target)
	il2cpp_class_get_methods = (decltype(il2cpp_class_get_methods))(g_base + 0x43A650);
	il2cpp_class_get_name = (decltype(il2cpp_class_get_name))(g_base + 0x5A30);
	il2cpp_class_get_namespace = (decltype(il2cpp_class_get_namespace))(g_base + 0x3DC160);
	il2cpp_method_get_name = (decltype(il2cpp_method_get_name))(g_base + 0x3DCA60);
	DecryptParameters = (decltype(DecryptParameters))(g_base + 0x451910); // sub_451910
	Class_FromIl2CppType = (decltype(Class_FromIl2CppType))(g_base + 0x4373F0);
	il2cpp_method_get_param_name = (decltype(il2cpp_method_get_param_name))(g_base + 0x3DCAC0);

	// Hook class initialization
	Log("Hooking Class::Init...\n");
	MH_CreateHook((LPVOID)(g_base + 0x43C7A0), h_ClassInit, (void**)&o_ClassInit);
	MH_EnableHook(MH_ALL_HOOKS);

	return 0;
}

// -----------------------------------------------------------------------------
// DLL entry
// -----------------------------------------------------------------------------
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(hModule);
		CreateThread(NULL, 0, StartThread, NULL, 0, NULL);
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		// flush + close dump file
		if (g_dumpFile) {
			fflush(g_dumpFile);
			int fd = _fileno(g_dumpFile);
			if (fd != -1) {
				intptr_t osHandle = _get_osfhandle(fd);
				if (osHandle != -1 && osHandle != (intptr_t)INVALID_HANDLE_VALUE) {
					FlushFileBuffers((HANDLE)osHandle);
				}
			}
			fclose(g_dumpFile);
			g_dumpFile = nullptr;
		}

		MH_Uninitialize();
	}
	return TRUE;
}