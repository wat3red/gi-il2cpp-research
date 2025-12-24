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
#include <stdio.h>
#include <regex>

#include "lib/minhook/include/MinHook.h"
#include "il2cpp_types.h"
#include "logger.h"
#include "dump.h"

#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "ws2_32.lib")

// Globals
uintptr_t g_base = 0;
FILE* g_structs_file = nullptr;
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
Il2CppType* (*il2cpp_method_get_param)(MethodInfo* method, uint32_t index) = nullptr;
Il2CppType* (*il2cpp_method_get_return_type)(MethodInfo* method) = nullptr;

const char* (*il2cpp_field_get_name)(FieldInfo* field) = nullptr;
int (*il2cpp_field_get_flags)(FieldInfo* field) = nullptr;
Il2CppType* (*il2cpp_field_get_type)(FieldInfo* field) = nullptr;
int32_t(*il2cpp_field_get_offset)(FieldInfo* field) = nullptr;

void(*il2cpp_type_get_name_tmp)(void* out_str_struct, Il2CppType* type, int format) = nullptr;
void(*il2cpp_free_temp_str)(void* out_str_struct) = nullptr;

Il2CppClass* (*MetadataCache__GetTypeInfoFromTypeDefinitionIndex)(int32_t typeDefinitionIndex) = nullptr;
uint64_t(*il2cpp__vm__GetEnumFieldValue)(Il2CppClass* enumType, FieldInfo* field) = nullptr;

int WINAPI h_send(SOCKET s, const char* buf, int len, int flags)
{
	if (g_blockPackets)
		return len;

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

void DisableLogReport()
{
	wchar_t filename[MAX_PATH] = {};
	GetModuleFileName(NULL, filename, MAX_PATH);

	auto path = std::filesystem::path(filename);
	path = path.parent_path() / (path.stem().string() + "_Data") / "Plugins";

	CreateFileW((path / "Astrolabe.dll").c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	CreateFileW((path / "MiHoYoMTRSDK.dll").c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
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

// Helper to sanitize names for C syntax (System.Int32 -> System_Int32)
std::string SanitizeName(std::string name) {
	std::string out;
	for (char c : name) {
		if (isalnum(c) || c == '_') {
			out += c;
		}
		else {
			out += "_";
		}
	}
	// Remove consecutive underscores for cleaner output
	std::string clean;
	bool lastUnder = false;
	for (char c : out) {
		if (c == '_') {
			if (!lastUnder) clean += c;
			lastUnder = true;
		}
		else {
			clean += c;
			lastUnder = false;
		}
	}
	return clean;
}

uint8_t Il2CppTypeGetType(Il2CppType* type) {
	return  *(uint8_t*)((uintptr_t)type + 0xA); // 0F B6 46 ? C1 E0 ? 3D ? ? ? ? 75 ? 8B 05
}

uint32_t CalculateClassSize(Il2CppClass* klass) {
	return *(int16_t*)((uintptr_t)klass + 0xB4) - 4; // 0F B7 9F ? ? ? ? 48 89 F9
}

Il2CppClass* Il2CppClassGetParent(Il2CppClass* klass) {
#define METADATA_BASE_POINTER 0x4C7B1F0 // 48 8B 05 ? ? ? ? ? ? ? 4D 39 C8 75 ? 48 83 C1
	uint32_t parentToken = *(uint32_t*)((uintptr_t)klass + 0xA4); // 41 8B 87 ? ? ? ? 41 BF 00 00 00 00
	if (!parentToken) return nullptr;

	return (Il2CppClass*)(**(uintptr_t**)(g_base + METADATA_BASE_POINTER) + parentToken);
#undef METADATA_BASE_POINTER
}

uint8_t MethodGetParamCount(MethodInfo* method) {
	return *(uint8_t*)((uintptr_t)method + 0x2E);
}

int16_t MethodGetSlot(MethodInfo* method) {
	return *(int16_t*)((uintptr_t)method + 0x28); // 48 C7 40 ? 00 00 00 00 ? ? ? 66 C7 40
}

uint16_t MethodGetFlags(MethodInfo* method) {
	return *(uint16_t*)((uintptr_t)method + 0x2A);
}

uintptr_t MethodGetMethodPointer(MethodInfo* method) {
	return *(uintptr_t*)((uintptr_t)method + 0x8);  // probably 48 83 78 ? 00 74 ? 48 83 C4 ? 5E 5D
}

std::string GetCTypeAndSize(Il2CppType* type, uint32_t& outSize) {
	if (!type) { outSize = 8; return "void*"; }

	uint8_t typeEnum = Il2CppTypeGetType(type);

	outSize = 8; // Default pointer size (x64)
	
	switch (typeEnum) {
	case IL2CPP_TYPE_VOID:    outSize = 0; return "void";
	case IL2CPP_TYPE_BOOLEAN: outSize = 1; return "bool";
	case IL2CPP_TYPE_I1:      outSize = 1; return "int8_t";
	case IL2CPP_TYPE_U1:      outSize = 1; return "uint8_t";
	case IL2CPP_TYPE_I2:      outSize = 2; return "int16_t";
	case IL2CPP_TYPE_U2:      outSize = 2; return "uint16_t";
	case IL2CPP_TYPE_CHAR:    outSize = 2; return "uint16_t"; // C# char = 2 bytes (UTF-16)
	case IL2CPP_TYPE_I4:      outSize = 4; return "int32_t";
	case IL2CPP_TYPE_U4:      outSize = 4; return "uint32_t";
	case IL2CPP_TYPE_R4:      outSize = 4; return "float";
	case IL2CPP_TYPE_I8:      outSize = 8; return "int64_t";
	case IL2CPP_TYPE_U8:      outSize = 8; return "uint64_t";
	case IL2CPP_TYPE_R8:      outSize = 8; return "double";
	case IL2CPP_TYPE_I:
	case IL2CPP_TYPE_U:       outSize = 8; return "intptr_t"; // Native int

	case IL2CPP_TYPE_STRING:  outSize = 8; return "struct Il2CppString*";
	case IL2CPP_TYPE_OBJECT:  outSize = 8; return "struct Il2CppObject*";

		// === СТРУКТУРЫ (Value Types) ===
		// Хранятся "inline", то есть само тело структуры лежит внутри класса.
	case IL2CPP_TYPE_VALUETYPE:
	{
		Il2CppClass* klass = il2cpp_class_from_type(type);
		if (klass) {
			std::string safeName = SanitizeName(il2cpp_class_get_name(klass));
			std::string ns = SanitizeName(il2cpp_class_get_namespace(klass));
			if (!ns.empty()) safeName = ns + "_" + safeName;

			// В Il2Cpp "размер класса" включает заголовок объекта (0x10 байт: vtable + monitor),
			// даже для структур (в их boxed виде).
			// Но когда структура лежит в поле, заголовка нет.
			uint32_t boxedSize = CalculateClassSize(klass);

			// Вычитаем заголовок, чтобы получить реальный размер данных
			if (boxedSize > 0x10)
				outSize = boxedSize - 0x10;
			else
				outSize = 1; // Пустая структура не может быть 0 байт в C++

			// Возвращаем имя БЕЗ звездочки
			return "struct " + safeName;
		}
		return "void*"; // Fallback
	}

	// === КЛАССЫ и МАССИВЫ (Reference Types) ===
	// Всегда являются указателями (8 байт)
	case IL2CPP_TYPE_CLASS:
	case IL2CPP_TYPE_SZARRAY:
	case IL2CPP_TYPE_ARRAY:
	case IL2CPP_TYPE_GENERICINST: // Обычно GenericInst - это класс, но бывает и struct.
		// Для простоты дампера часто считают указателем, 
		// если не хотим лезть в дебри проверки IsValueType для generic.
	{
		Il2CppClass* klass = il2cpp_class_from_type(type);
		if (klass) {
			std::string safeName = SanitizeName(il2cpp_class_get_name(klass));
			std::string ns = SanitizeName(il2cpp_class_get_namespace(klass));
			if (!ns.empty()) safeName = ns + "_" + safeName;

			// Возвращаем указатель
			return "struct " + safeName + "*";
		}
		return "void*";
	}

	case IL2CPP_TYPE_PTR: return "void*";

	default: return "void*";
	}
}

void CollectClasses(std::vector<Il2CppClass*>& allClasses) {
	int32_t i = 0;
	__try {
		Log("Collecting classes for struct dump...\n");
		for (;; ++i) {
			Il2CppClass* cls = MetadataCache__GetTypeInfoFromTypeDefinitionIndex(i);
			if (!cls) break;
			allClasses.push_back(cls);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		Log("Exception occurred while dumping class at index %d\n", i);
	}
}

void DumpStructs() {
	fopen_s(&g_structs_file, "structs.h", "w");
	if (!g_structs_file) return;

	fprintf(g_structs_file, "// Generated by GIRuntimeDumper\n");
	fprintf(g_structs_file, "#pragma once\n\n");
	fprintf(g_structs_file, "#include <cstdint>\n\n");

	// Standard Il2Cpp Types
	fprintf(g_structs_file, "struct Il2CppString { struct Il2CppObject* obj; int32_t length; char chars[1]; };\n\n");

	std::vector<Il2CppClass*> allClasses;

	// 1. Collect all classes
	CollectClasses(allClasses);

	// 2. Forward Declarations (Typedefs)
	// This allows struct A to have a field of type struct B* even if B is defined later.
	Log("Writing forward declarations...\n");
	for (Il2CppClass* cls : allClasses) {
		std::string name = il2cpp_class_get_name(cls);
		std::string ns = il2cpp_class_get_namespace(cls);
		std::string fullName = SanitizeName(ns.empty() ? name : ns + "_" + name);

		fprintf(g_structs_file, "typedef struct %s %s;\n", fullName.c_str(), fullName.c_str());
	}
	fprintf(g_structs_file, "\n");

	// 3. Definitions
	Log("Writing struct definitions...\n");
	for (Il2CppClass* cls : allClasses) {
		std::string name = il2cpp_class_get_name(cls);
		std::string ns = il2cpp_class_get_namespace(cls);
		std::string fullName = SanitizeName(ns.empty() ? name : ns + "_" + name);

		// Resolve Parent
		std::string parentName = "Il2CppObject"; // Default root
		Il2CppClass* parentClass = Il2CppClassGetParent(cls);
		if (parentClass) {
			std::string pName = il2cpp_class_get_name(parentClass);
			std::string pNs = il2cpp_class_get_namespace(parentClass);
			parentName = SanitizeName(pNs.empty() ? pName : pNs + "_" + pName);
		}

		if (parentName.compare("System_Enum") == 0) {
			// 1. Find the backing field (instance field) to determine type (int, byte, etc.)
			void* iter = nullptr;
			FieldInfo* valueField = nullptr;
			int enumCount = 0;
			while (FieldInfo* f = il2cpp_class_get_fields(cls, &iter)) {
				if (!f) break;
				if (strcmp(il2cpp_field_get_name(f), "value__") == 0) valueField = f;
				else ++enumCount;
			}

			if (!valueField) continue;

			uint32_t typeSize = 0;
			std::string backingType = GetCTypeAndSize(il2cpp_field_get_type(valueField), typeSize);
			
			uint8_t typeEnum = Il2CppTypeGetType(il2cpp_field_get_type(valueField));

			fprintf(g_structs_file, "// Namespace: %s\n", ns.c_str());
			fprintf(g_structs_file, "enum class %s : %s {\n", fullName.c_str(), backingType.c_str());

			iter = nullptr;
			int index = 0;
			while (FieldInfo* f = il2cpp_class_get_fields(cls, &iter)) {
				if (!f) break;
				if (f == valueField) continue;

				fprintf(g_structs_file, "    %s = ", il2cpp_field_get_name(f));

				uint64_t raw = il2cpp__vm__GetEnumFieldValue(cls, f);
				switch (typeEnum) {
				case IL2CPP_TYPE_I1:
					fprintf(g_structs_file, "%d", (int8_t)raw);
					break;
				case IL2CPP_TYPE_U1:
					fprintf(g_structs_file, "%u", (uint8_t)raw);
					break;
				case IL2CPP_TYPE_I2:
					fprintf(g_structs_file, "%d", (int16_t)raw);
					break;
				case IL2CPP_TYPE_U2:
					fprintf(g_structs_file, "%u", (uint16_t)raw);
					break;
				case IL2CPP_TYPE_I4:
					fprintf(g_structs_file, "%d", (int32_t)raw);
					break;
				case IL2CPP_TYPE_U4:
					fprintf(g_structs_file, "%u", (uint32_t)raw);
					break;
				case IL2CPP_TYPE_I8:
					fprintf(g_structs_file, "%lld", (int64_t)raw);
					break;
				case IL2CPP_TYPE_U8:
					fprintf(g_structs_file, "%llu", (uint64_t)raw);
					break;
				default:
					// unreachable
					break;
				}

				if (++index < enumCount)
					fprintf(g_structs_file, ",\n");
			}

			fprintf(g_structs_file, "\n};\n\n");
			continue;
		}
		else {
			fprintf(g_structs_file, "// Namespace: %s\n", ns.c_str());
			fprintf(g_structs_file, "struct %s : %s {\n", fullName.c_str(), parentName.c_str());

			// Process Fields
			void* iter = nullptr;
			std::vector<FieldInfo*> fields;
			while (FieldInfo* f = il2cpp_class_get_fields(cls, &iter)) {
				fields.push_back(f);
			}

			for (FieldInfo* field : fields) {
				int32_t offset = il2cpp_field_get_offset(field);
				int flags = il2cpp_field_get_flags(field);

				// Skip static fields for the struct layout (they are global memory, not instance)
				if (flags & FIELD_ATTRIBUTE_STATIC) continue;

				std::string fName = il2cpp_field_get_name(field);

				// Handle auto-property backing field
				std::replace(fName.begin(), fName.end(), '<', '_');
				std::replace(fName.begin(), fName.end(), '>', '_');

				uint32_t typeSize = 0;
				std::string typeStr = GetCTypeAndSize(il2cpp_field_get_type(field), typeSize);

				fprintf(g_structs_file, "    %s %s; // 0x%X\n", typeStr.c_str(), fName.c_str(), offset);
			}
		}

		fprintf(g_structs_file, "};\n\n");
	}

	fclose(g_structs_file);
	g_structs_file = nullptr;
	printf("Struct dump completed: structs.h\n");
}

std::string GetTypeName(Il2CppType* type, int format = 0)
{
	//Log("q\n");

	if (g_cached_types.find(type) != g_cached_types.end())
		return g_cached_types[type];

	if (!type) return "unknown";

	// v10 is an array of 4 qwords in pseudocode -> we'll use a small struct
	uint64_t out[4] = { 0 };
	//Log("w\n");

	// Call the inlined formatter: out <- formatted string representation of 'type'
	il2cpp_type_get_name_tmp(out, type, format);

	//Log("e\n");

	/*Log("0: %X \n", out[0]);
	Log("1: %X \n", out[1]);
	Log("2: %X \n", out[2]);
	Log("3: %X \n", out[3]);*/

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

	//Log("result %s\n", result.c_str());

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
	Il2CppClass* parent = Il2CppClassGetParent(classPtr);
	if (parent) {
		std::string parentName = il2cpp_class_get_name(parent);
		Log("// Namespace: %s\nclass %s : %s\n{\n\t// Fields \n\n", namespaceName.c_str(), className.c_str(), parentName.c_str());
	}
	else {
		Log("// Namespace: %s\nclass %s \n{\n\t// Fields \n\n", namespaceName.c_str(), className.c_str());
	}
	//Log("1\n");

	void* fieldIter = nullptr;
	while (FieldInfo* field = il2cpp_class_get_fields(classPtr, &fieldIter)) {
		//Log("2\n");

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
		//Log("3\n");

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

	//Log("4\n");
	void* methodIter = nullptr;
	while (MethodInfo* method = il2cpp_class_get_methods(classPtr, &methodIter)) {
		//Log("5\n");

		uint8_t paramCount = MethodGetParamCount(method);
		int16_t slot = MethodGetSlot(method);
		uint16_t flags = MethodGetFlags(method);

		//Log("6\n");

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
			//Log("7\n");

			//uintptr_t paramArray = il2cpp_method_get_params(method) + 0x8;
			for (uint8_t i = 0; i < paramCount; i++) {
				//Log("8\n");

				Il2CppType* paramType = il2cpp_method_get_param(method, i);
				if (!paramType) continue;

				const char* paramName = il2cpp_method_get_param_name(method, i);
				if (!paramName) continue;

				//Log("paramName: %s\n", paramName);

				std::string typeName = StripNamespaces(GetTypeName(paramType));

				paramList += typeName + std::string(" ") + (std::string)paramName + (i == paramCount - 1 ? "" : ", ");
			}
		}
		//Log("10\n");

		Il2CppType* returnType = il2cpp_method_get_return_type(method);
		std::string returnTypeName = StripNamespaces(GetTypeName(returnType));

		std::string slotStr;
		if (slot != -1) slotStr = " Slot: " + std::to_string(slot) + ",";

		Log("\t%s%s %s(%s); // FLAGS: 0x%X,%s RVA: 0x%X \n",
			modifiers.c_str(),
			returnTypeName.c_str(),
			il2cpp_method_get_name(method),
			paramList.c_str(),
			flags,
			slotStr.c_str(),
			MethodGetMethodPointer(method) - g_base);
	}

	Log("}\n\n");
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

	// E8 ? ? ? ? 48 85 C0 74 ? 48 8D 5D
	il2cpp_class_get_methods = (decltype(il2cpp_class_get_methods))(g_base + 0x446130);

	// E8 ? ? ? ? 45 33 F6 C7 85
	il2cpp_class_get_name = (decltype(il2cpp_class_get_name))(g_base + 0xA790);

	// E8 ? ? ? ? 49 C7 C7 ? ? ? ? 4D 8B C7
	il2cpp_class_get_namespace = (decltype(il2cpp_class_get_namespace))(g_base + 0x3E75E0);

	// E8 ? ? ? ? 48 85 C0 75 ? E9 ? ? ? ? 89 E8
	il2cpp_class_get_fields = (decltype(il2cpp_class_get_fields))(g_base + 0x445B30);

	// E8 ? ? ? ? 48 89 C6 44 0F B7 B0
	il2cpp_class_from_type = (decltype(il2cpp_class_from_type))(g_base + 0x442EA0);

	// E8 ? ? ? ? 48 89 C3 EB ? 83 BD
	il2cpp_field_get_name = (decltype(il2cpp_field_get_name))(g_base + 0x451F50);

	// E8 ? ? ? ? 48 8B CB 41 89 46
	il2cpp_field_get_flags = (decltype(il2cpp_field_get_flags))(g_base + 0x3E7930);

	// E8 ? ? ? ? 48 8B C8 49 89 46 ? E8
	il2cpp_field_get_type = (decltype(il2cpp_field_get_type))(g_base + 0x3E7970);

	// E8 ? ? ? ? 49 03 45
	il2cpp_field_get_offset = (decltype(il2cpp_field_get_offset))(g_base + 0x3E7960); // Socket::SetSocketOption

	// E8 ? ? ? ? 48 8B CE 48 2B C6
	il2cpp_method_get_name = (decltype(il2cpp_method_get_name))(g_base + 0x3E7EC0);

	// direct: 56 48 83 EC ? 0F B6 41 ? 39 D0 76 ? 89 D6 48 8B 51 ? 48 85 D2 74 ? 48 B8 ? ? ? ? ? ? ? ? ? ? ? 74 ? 89 F1 ? ? ? ? ? ? ? ? 48 83 C4
	il2cpp_method_get_param_name = (decltype(il2cpp_method_get_param_name))(g_base + 0x3E8020);

	// E8 ? ? ? ? 48 8B C8 E8 ? ? ? ? 4C 8B 4E
	il2cpp_method_get_param = (decltype(il2cpp_method_get_param))(g_base + 0x3E7F20);

	// E8 ? ? ? ? 48 83 C4 ? 48 89 C7 0F B6 47
	il2cpp_method_get_return_type = (decltype(il2cpp_method_get_return_type))(g_base + 0x45D360);

	// E8 ? ? ? ? 4C 8D 05 ? ? ? ? 48 8D 4D ? 48 8D 55 ? E8 ? ? ? ? 48 89 E9 4C 8D 45
	il2cpp_type_get_name_tmp = (decltype(il2cpp_type_get_name_tmp))(g_base + 0x45C620);

	// E8 ? ? ? ? B3 ? E9 ? ? ? ? 4C 8B B6
	il2cpp_free_temp_str = (decltype(il2cpp_free_temp_str))(g_base + 0x8DFF80);

	// E8 ? ? ? ? 0F B7 A8
	MetadataCache__GetTypeInfoFromTypeDefinitionIndex = (decltype(MetadataCache__GetTypeInfoFromTypeDefinitionIndex))(g_base + 0x452110);

	// direct: 41 56 56 57 53 48 83 EC ? 48 89 D7 49 89 CE 48 8B 42 ? 48 BA
	il2cpp__vm__GetEnumFieldValue = (decltype(il2cpp__vm__GetEnumFieldValue))(g_base + 0x44F970);

	std::thread blockPacketsThread(([]() { Sleep(10000); g_blockPackets = false; }));
	blockPacketsThread.detach();

	while (!FindWindowA("UnityWndClass", nullptr))
	{
		Sleep(100);
	}

	Sleep(15000);
	printf("Starting dump\n");

	//Il2CppDump();
	DumpStructs();

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
