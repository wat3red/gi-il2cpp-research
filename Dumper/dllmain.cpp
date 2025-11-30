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

#include "lib/minhook/include/MinHook.h"
#include "il2cpp_types.h"
#include "logger.h"

#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "ws2_32.lib")

// Globals
uintptr_t g_base = 0;
FILE* g_log_file = nullptr;
bool g_blockPackets = true;

std::unordered_map<Il2CppType*, std::string> g_cached_types;

// Function pointers
typedef int (WINAPI* send_t)(SOCKET, const char*, int, int);
send_t o_send = nullptr;

typedef int (WINAPI* WSASend_t)(SOCKET, LPWSABUF, DWORD, LPDWORD, DWORD, LPWSAOVERLAPPED, LPWSAOVERLAPPED_COMPLETION_ROUTINE);
WSASend_t o_WSASend = nullptr;

typedef int (WINAPI* connect_t)(SOCKET, const sockaddr*, int);
connect_t o_connect = nullptr;

MethodInfo* (*il2cpp_class_get_methods)(Il2CppClass* klass, void** iter) = nullptr;
const char* (*il2cpp_class_get_name)(Il2CppClass* klass) = nullptr;
const char* (*il2cpp_class_get_namespace)(Il2CppClass* klass) = nullptr;
FieldInfo* (*il2cpp_class_get_fields)(Il2CppClass* klass, void** iter) = nullptr;
Il2CppClass* (*il2cpp_class_from_type)(const Il2CppType* type) = nullptr;

const char* (*il2cpp_method_get_name)(MethodInfo* method) = nullptr;
const char* (*il2cpp_method_get_param_name)(MethodInfo* method, uint32_t index) = nullptr;
uintptr_t(*il2cpp_method_get_params)(MethodInfo* method) = nullptr;
Il2CppType* (*il2cpp_method_get_return_type)(MethodInfo* method) = nullptr;

const char* (*il2cpp_field_get_name)(FieldInfo* field) = nullptr;
int (*il2cpp_field_get_flags)(FieldInfo* field) = nullptr;
Il2CppType* (*il2cpp_field_get_type)(FieldInfo* field) = nullptr;
int32_t(*il2cpp_field_get_offset)(FieldInfo* field) = nullptr;

void(*il2cpp_type_get_name_tmp)(void* out_str_struct, Il2CppType* type, int format) = nullptr;
void(*il2cpp_free_temp_str)(void* out_str_struct) = nullptr;

Il2CppClass* (*MetadataCache__GetTypeInfoFromTypeDefinitionIndex)(int32_t typeDefinitionIndex) = nullptr;

int WINAPI h_send(SOCKET s, const char* buf, int len, int flags)
{
	if (g_blockPackets)
		return len; // притворяемся, что отправили

	return o_send(s, buf, len, flags);
}

int WINAPI h_WSASend(
	SOCKET s, LPWSABUF buffers, DWORD bufferCount,
	LPDWORD bytesSent, DWORD flags,
	LPWSAOVERLAPPED overlapped,
	LPWSAOVERLAPPED_COMPLETION_ROUTINE completion
) {
	if (g_blockPackets) {
		if (bytesSent) *bytesSent = buffers->len;
		return 0;
	}

	return o_WSASend(s, buffers, bufferCount, bytesSent, flags, overlapped, completion);
}

int WINAPI h_connect(SOCKET s, const sockaddr* name, int namelen)
{
	if (g_blockPackets) {
		WSASetLastError(WSAECONNREFUSED);
		return SOCKET_ERROR;
	}

	return o_connect(s, name, namelen);
}

std::string StripNamespaces(const std::string& full)
{
	std::string out;
	out.reserve(full.size());

	for (size_t i = 0; i < full.size(); ++i)
	{
		char c = full[i];

		// Если встретили имя внутри generics (< ... >)
		if (std::isalnum((unsigned char)c) || c == '_')
		{
			size_t start = i;

			// читаем токен (до < > , . whitespace)
			while (i < full.size() &&
				(std::isalnum((unsigned char)full[i]) || full[i] == '_' || full[i] == '.'))
			{
				i++;
			}

			std::string token = full.substr(start, i - start);

			// если есть namespace -> отрезаем всё до последней точки
			size_t dot = token.rfind('.');
			if (dot != std::string::npos)
				token = token.substr(dot + 1);

			out += token;

			i--; // компенсируем повышение i
			continue;
		}

		// управляющие символы (например < > , [] )
		out.push_back(c);
	}

	return out;
}


std::string GetTypeName(Il2CppType* type, int format = 0)
{
	if (g_cached_types.find(type) != g_cached_types.end())
		return g_cached_types[type];

	if (!type) return "unknown";

	// v10 is an array of 4 qwords in pseudocode -> we'll use a small struct
	uint64_t out[4] = { 0 };

	// Call the inlined formatter: out <- formatted string representation of 'type'
	il2cpp_type_get_name_tmp(out, type, format);

	const char* cstr = nullptr;
	// pseudocode checks v10[3] >= 0x10 then uses v10[0] else uses inline buffer inside 'out'
	// treat out[3] as length/capacity indicator like std::string-small-buffer heuristic
	if (out[3] >= 0x10)
		cstr = (const char*)out[0];
	else
	{
		// if it's small-string, the chars are stored in the out buffer itself;
		// pointer to inline buffer = reinterpret_cast<char*>(&out[0])
		cstr = reinterpret_cast<const char*>(&out[0]);
	}

	std::string result = cstr ? std::string(cstr) : std::string("unknown");

	// free temp if allocator used (pseudocode calls sub_8D3AD0(v10))
	il2cpp_free_temp_str(out);

	g_cached_types[type] = result;

	return result;
}

void DumpClassInfo(int32_t type_def_index, Il2CppClass* classPtr) {
	std::string className = il2cpp_class_get_name(classPtr);
	std::string namespaceName = il2cpp_class_get_namespace(classPtr);

	Log("// TypeDefIndex: %d\n", type_def_index);

	std::stringstream outPut;
	uint32_t parentToken = *(uint32_t*)((uintptr_t)classPtr + 0xAC);
	if (parentToken != 0) {
		Il2CppClass* parentClass = (Il2CppClass*)(**(uintptr_t**)(g_base + 0x4BAF8B0) + parentToken); // metadata_base_pointer
		if (parentClass) {
			std::string parentClassName = il2cpp_class_get_name(parentClass);
			Log("// Namespace: %s\nclass %s : %s\n{\n\t// Fields \n\n", namespaceName.c_str(), className.c_str(), parentClassName.c_str());
		}
	}
	else {
		Log("// Namespace: %s\nclass %s \n{\n\t// Fields \n\n", namespaceName.c_str(), className.c_str());
	}

	void* fieldIter = nullptr;
	while (FieldInfo* field = il2cpp_class_get_fields(classPtr, &fieldIter)) {
		int flags = il2cpp_field_get_flags(field);
		std::string modifiers = "";

		// ===== Access flags =====
		int access = flags & FIELD_ATTRIBUTE_FIELD_ACCESS_MASK;
		switch (access) {
		case FIELD_ATTRIBUTE_PUBLIC:            modifiers += "public"; break;
		case FIELD_ATTRIBUTE_PRIVATE:           modifiers += "private"; break;
		case FIELD_ATTRIBUTE_FAMILY:            modifiers += "protected"; break;
		case FIELD_ATTRIBUTE_ASSEMBLY:          modifiers += "internal"; break;
		case FIELD_ATTRIBUTE_FAM_OR_ASSEM:      modifiers += "protected internal"; break;
		case FIELD_ATTRIBUTE_FAM_AND_ASSEM:     modifiers += "private protected"; break;
		case FIELD_ATTRIBUTE_COMPILER_CONTROLLED: modifiers += "/* compiler-controlled */"; break;
		default: modifiers += "unknown_flag"; break;
		}

		// ===== Other flags =====
		if (flags & FIELD_ATTRIBUTE_STATIC)          modifiers += " static";
		if (flags & FIELD_ATTRIBUTE_INIT_ONLY)       modifiers += " readonly";
		if (flags & FIELD_ATTRIBUTE_LITERAL)         modifiers += " const";
		if (flags & FIELD_ATTRIBUTE_NOT_SERIALIZED)  modifiers += " notserialized";
		if (flags & FIELD_ATTRIBUTE_SPECIAL_NAME)    modifiers += " specialname";
		if (flags & FIELD_ATTRIBUTE_RT_SPECIAL_NAME) modifiers += " rtspecialname";
		if (flags & FIELD_ATTRIBUTE_HAS_FIELD_RVA)   modifiers += " rva";
		if (flags & FIELD_ATTRIBUTE_HAS_DEFAULT)     modifiers += " default";
		if (flags & FIELD_ATTRIBUTE_HAS_FIELD_MARSHAL) modifiers += " marshal";

		const char* fieldName = il2cpp_field_get_name(field);
		int32_t offset = il2cpp_field_get_offset(field);

		Il2CppType* type = il2cpp_field_get_type(field);
		std::string typeName = StripNamespaces(GetTypeName(type));

		Log("\t%s %s %s; // 0x%X, FLAGS: 0x%X\n",
			modifiers.c_str(),
			typeName.c_str(),
			fieldName,
			offset,
			flags);
	}

	Log("\n\t// Methods \n\n");

	void* methodIter = nullptr;
	while (MethodInfo* method = il2cpp_class_get_methods(classPtr, &methodIter)) {
		uint8_t paramCount = *(uint8_t*)((uintptr_t)method + 0x2E);
		int16_t slot = *(int16_t*)((uintptr_t)method + 0x2C);
		uint16_t flags = *(uint16_t*)((uintptr_t)method + 0x2A);

		// Parse access modifiers
		uint16_t accessMask = flags & METHOD_ATTRIBUTE_MEMBER_ACCESS_MASK;
		bool isPublic = (accessMask == METHOD_ATTRIBUTE_PUBLIC);
		bool isPrivate = (accessMask == METHOD_ATTRIBUTE_PRIVATE);
		bool isProtected = (accessMask == METHOD_ATTRIBUTE_FAMILY);
		bool isInternal = (accessMask == METHOD_ATTRIBUTE_ASSEM);
		bool isProtectedInternal = (accessMask == METHOD_ATTRIBUTE_FAM_OR_ASSEM);
		bool isPrivateProtected = (accessMask == METHOD_ATTRIBUTE_FAM_AND_ASSEM);

		// Parse method attributes
		bool isStatic = (flags & METHOD_ATTRIBUTE_STATIC) != 0;
		bool isFinal = (flags & METHOD_ATTRIBUTE_FINAL) != 0;
		bool isVirtual = (flags & METHOD_ATTRIBUTE_VIRTUAL) != 0;
		bool isAbstract = (flags & METHOD_ATTRIBUTE_ABSTRACT) != 0;
		bool isNewSlot = (flags & METHOD_ATTRIBUTE_VTABLE_LAYOUT_MASK) != 0;

		// Build modifiers string
		std::string modifiers = "";

		// Access level
		if (isPublic) modifiers += "public ";
		else if (isPrivate) modifiers += "private ";
		else if (isProtectedInternal) modifiers += "protected internal ";
		else if (isPrivateProtected) modifiers += "private protected ";
		else if (isProtected) modifiers += "protected ";
		else if (isInternal) modifiers += "internal ";

		// Static
		if (isStatic) modifiers += "static ";

		// Abstract/Virtual/Override/Sealed
		if (isAbstract) {
			modifiers += "abstract ";
		}
		else if (isVirtual) {
			if (slot != -1 && !isNewSlot) {
				modifiers += "override ";
			}
			else {
				modifiers += "virtual ";
			}

			if (isFinal) {
				modifiers += "sealed ";
			}
		}
		else if (isFinal && !isStatic) {
			modifiers += "sealed ";
		}

		std::string paramList = "";
		if (paramCount) {
			uintptr_t paramArray = il2cpp_method_get_params(method) + 0x8;
			for (uint8_t i = 0; i < paramCount; i++) {
				// Each parameter is 3 pointers (0x18 bytes)
				uintptr_t param = paramArray + ((i - 1) * 0x18);
				if (!param) continue;

				Il2CppType* paramType = *(Il2CppType**)(paramArray + (i * 0x18));
				if (!paramType) continue;

				const char* paramName = *(const char**)((uintptr_t)param + 0x10);
				if (!paramName) continue;

				std::string typeName = StripNamespaces(GetTypeName(paramType));

				paramList += typeName + std::string(" ") + (std::string)paramName + (i == paramCount - 1 ? "" : ", ");
			}
		}

		Il2CppType* returnType = il2cpp_method_get_return_type(method);
		std::string returnTypeName = StripNamespaces(GetTypeName(returnType));

		Log("\t%s%s %s(%s); // Slot: %d, RVA: 0x%X, FLAGS: 0x%X\n",
			modifiers.c_str(),
			returnTypeName.c_str(),
			il2cpp_method_get_name(method),
			paramList.c_str(),
			slot,
			(*(uintptr_t*)((uintptr_t)method)) - g_base,
			flags);

	}

	Log("}\n\n");
}

void DisableLogReport()
{
	wchar_t filename[MAX_PATH] = {};
	GetModuleFileName(NULL, filename, MAX_PATH);

	auto path = std::filesystem::path(filename);
	path = path.parent_path() / (path.stem().string() + "_Data") / "Plugins";

	CreateFileW((path / "Astrolabe.dll").c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	CreateFileW((path / "MiHoYoMTRSDK.dll").c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
}

void Il2CppDump() {
	for (int32_t i = 0; ; ++i) {
		__try {
			Il2CppClass* classPtr = MetadataCache__GetTypeInfoFromTypeDefinitionIndex(i);
			if (!classPtr) break;
			DumpClassInfo(i, classPtr);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			printf("Exception occurred while dumping class at index %d\n", i);
			break;
		}
	}
}

// === Packet Blocker Hooks ===
bool InitBlockingHooks() {
	if (MH_Initialize() != MH_OK) {
		Log("MinHook init failed!\n");
		return 0;
	}

	MH_CreateHookApi(L"ws2_32", "send", h_send, (LPVOID*)&o_send);
	MH_CreateHookApi(L"ws2_32", "WSASend", h_WSASend, (LPVOID*)&o_WSASend);
	MH_CreateHookApi(L"ws2_32", "connect", h_connect, (LPVOID*)&o_connect);

	MH_EnableHook(MH_ALL_HOOKS);

	return 1;
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

	DisableLogReport();

	g_base = (uintptr_t)GetModuleHandle(NULL);
	Log("Game Base: 0x%p\n", (void*)g_base);

	// Resolve known offsets (update offsets for your target)
	il2cpp_class_get_methods = (decltype(il2cpp_class_get_methods))(g_base + 0x43A650);
	il2cpp_class_get_name = (decltype(il2cpp_class_get_name))(g_base + 0x5A30);
	il2cpp_class_get_namespace = (decltype(il2cpp_class_get_namespace))(g_base + 0x3DC160);
	il2cpp_class_get_fields = (decltype(il2cpp_class_get_fields))(g_base + 0x43A050);
	il2cpp_class_from_type = (decltype(il2cpp_class_from_type))(g_base + 0x4373F0);

	il2cpp_field_get_name = (decltype(il2cpp_field_get_name))(g_base + 0x446290);
	il2cpp_field_get_flags = (decltype(il2cpp_field_get_flags))(g_base + 0x3DC4D0);
	il2cpp_field_get_type = (decltype(il2cpp_field_get_type))(g_base + 0x3DC510);
	il2cpp_field_get_offset = (decltype(il2cpp_field_get_offset))(g_base + 0x3DC500); // Socket::SetSocketOption

	il2cpp_method_get_name = (decltype(il2cpp_method_get_name))(g_base + 0x3DCA60);
	il2cpp_method_get_param_name = (decltype(il2cpp_method_get_param_name))(g_base + 0x3DCAC0);
	il2cpp_method_get_params = (decltype(il2cpp_method_get_params))(g_base + 0x451910);
	il2cpp_method_get_return_type = (decltype(il2cpp_method_get_return_type))(g_base + 0x451660);

	il2cpp_type_get_name_tmp = (decltype(il2cpp_type_get_name_tmp))(g_base + 0x450930);
	il2cpp_free_temp_str = (decltype(il2cpp_free_temp_str))(g_base + 0x8D3AD0);

	MetadataCache__GetTypeInfoFromTypeDefinitionIndex = (decltype(MetadataCache__GetTypeInfoFromTypeDefinitionIndex))(g_base + 0x446450);

	//MH_EnableHook(MH_ALL_HOOKS);

	std::thread blockPacketsThread(([]() { Sleep(10000); g_blockPackets = false; }));
	blockPacketsThread.detach();

	while (!FindWindowA("UnityWndClass", nullptr))
	{
		Sleep(100);
	}

	Sleep(15000);
	printf("Starting dump\n");
	Il2CppDump();

	return 0;
}

// DLL entry
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(hModule);
		if (InitBlockingHooks())
			CreateThread(NULL, 0, StartThread, NULL, 0, NULL);
	}
	else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
		// flush + close dump file
		if (g_log_file) {
			fflush(g_log_file);
			int fd = _fileno(g_log_file);
			if (fd != -1) {
				intptr_t osHandle = _get_osfhandle(fd);
				if (osHandle != -1 && osHandle != (intptr_t)INVALID_HANDLE_VALUE) {
					FlushFileBuffers((HANDLE)osHandle);
				}
			}
			fclose(g_log_file);
			g_log_file = nullptr;
		}

		MH_Uninitialize();
	}
	return TRUE;
}
