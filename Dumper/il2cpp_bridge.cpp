// il2cpp_bridge.cpp
#include "il2cpp_bridge.h"
#include "utils.h"

// Define Global Config
uintptr_t Config::GameBase = 0;
bool Config::BlockPackets = true;

namespace Il2Cpp {
	// Definition of function pointers
	MethodInfo* (*class_get_methods)(Il2CppClass* klass, void** iter) = nullptr;
	const char* (*class_get_name)(Il2CppClass* klass) = nullptr;
	const char* (*class_get_namespace)(Il2CppClass* klass) = nullptr;
	FieldInfo* (*class_get_fields)(Il2CppClass* klass, void** iter) = nullptr;
	Il2CppClass* (*class_from_type)(const Il2CppType* type) = nullptr;

	const char* (*method_get_name)(MethodInfo* method) = nullptr;
	uint8_t(*method_get_param_count)(MethodInfo* method) = nullptr;
	const char* (*method_get_param_name)(MethodInfo* method, uint32_t index) = nullptr;
	Il2CppType* (*method_get_param)(MethodInfo* method, uint32_t index) = nullptr;
	Il2CppType* (*method_get_return_type)(MethodInfo* method) = nullptr;

	const char* (*field_get_name)(FieldInfo* field) = nullptr;
	int (*field_get_flags)(FieldInfo* field) = nullptr;
	Il2CppType* (*field_get_type)(FieldInfo* field) = nullptr;
	int32_t(*field_get_offset)(FieldInfo* field) = nullptr;

	void (*type_get_name_tmp)(void* out_str_struct, Il2CppType* type, int format) = nullptr;
	void (*free_temp_str)(void* out_str_struct) = nullptr;
	Il2CppClass* (*type_get_class_or_element_class)(Il2CppType* type) = nullptr;

	Il2CppClass* (*MetadataCache__GetTypeInfoFromTypeDefinitionIndex)(int32_t typeDefinitionIndex) = nullptr;
	uint64_t(*vm__GetEnumFieldValue)(Il2CppClass* enumType, FieldInfo* field) = nullptr;

	// Cache for type names
	std::unordered_map<Il2CppType*, std::string> g_cached_types;

	void Initialize() {
		uintptr_t b = Config::GameBase;

		// E8 ? ? ? ? 48 85 C0 74 ? 48 8D 5D
		class_get_methods = (decltype(class_get_methods))(b + 0x446130);
		// E8 ? ? ? ? 45 33 F6 C7 85
		class_get_name = (decltype(class_get_name))(b + 0xA790);
		// E8 ? ? ? ? 49 C7 C7 ? ? ? ? 4D 8B C7
		class_get_namespace = (decltype(class_get_namespace))(b + 0x3E75E0);
		// E8 ? ? ? ? 48 85 C0 75 ? E9 ? ? ? ? 89 E8
		class_get_fields = (decltype(class_get_fields))(b + 0x445B30);
		// E8 ? ? ? ? 48 89 C6 44 0F B7 B0
		class_from_type = (decltype(class_from_type))(b + 0x442EA0);
		// E8 ? ? ? ? 48 89 C3 EB ? 83 BD
		field_get_name = (decltype(field_get_name))(b + 0x451F50);
		// E8 ? ? ? ? 48 8B CB 41 89 46
		field_get_flags = (decltype(field_get_flags))(b + 0x3E7930);
		// E8 ? ? ? ? 48 8B C8 49 89 46 ? E8
		field_get_type = (decltype(field_get_type))(b + 0x3E7970);
		// E8 ? ? ? ? 49 03 45
		field_get_offset = (decltype(field_get_offset))(b + 0x3E7960);
		// E8 ? ? ? ? 48 8B CE 48 2B C6
		method_get_name = (decltype(method_get_name))(b + 0x3E7EC0);
		// E8 ? ? ? ? 3B C5 75
		method_get_param_count = (decltype(method_get_param_count))(b + 0x3E7F10);
		// direct: 56 48 83 EC ? 0F B6 41 ? 39 D0 76 ? 89 D6 48 8B 51 ? 48 85 D2 74 ? 48 B8 ? ? ? ? ? ? ? ? ? ? ? 74 ? 89 F1 ? ? ? ? ? ? ? ? 48 83 C4
		method_get_param_name = (decltype(method_get_param_name))(b + 0x3E8020);
		// E8 ? ? ? ? 48 8B C8 E8 ? ? ? ? 4C 8B 4E
		method_get_param = (decltype(method_get_param))(b + 0x3E7F20);
		// E8 ? ? ? ? 48 83 C4 ? 48 89 C7 0F B6 47
		method_get_return_type = (decltype(method_get_return_type))(b + 0x45D360);
		// E8 ? ? ? ? 4C 8D 05 ? ? ? ? 48 8D 4D ? 48 8D 55 ? E8 ? ? ? ? 48 89 E9 4C 8D 45
		type_get_name_tmp = (decltype(type_get_name_tmp))(b + 0x45C620);
		// E8 ? ? ? ? B3 ? E9 ? ? ? ? 4C 8B B6
		free_temp_str = (decltype(free_temp_str))(b + 0x8DFF80);
		// 0F BE 41 ? 83 F8 ? 74
		type_get_class_or_element_class = (decltype(type_get_class_or_element_class))(b + 0x3E8380); // Return class if type is not an array. Return element type if it is an array.
		// E8 ? ? ? ? 0F B7 A8
		MetadataCache__GetTypeInfoFromTypeDefinitionIndex = (decltype(MetadataCache__GetTypeInfoFromTypeDefinitionIndex))(b + 0x452110);
		// direct: 41 56 56 57 53 48 83 EC ? 48 89 D7 49 89 CE 48 8B 42 ? 48 BA
		vm__GetEnumFieldValue = (decltype(vm__GetEnumFieldValue))(b + 0x44F970);
	}

	uint8_t GetTypeEnum(Il2CppType* type) {
		// 0F B6 46 ? C1 E0 ? 3D ? ? ? ? 75 ? 8B 05
		return *(uint8_t*)((uintptr_t)type + 0xA);
	}

	uint32_t GetClassSize(Il2CppClass* klass) {
		// 0F B7 9F ? ? ? ? 48 89 F9
		return (uint32_t)((*(int16_t*)((uintptr_t)klass + 0xB4)) - 4);
	}

	Il2CppClass* GetClassParent(Il2CppClass* klass) {
#define METADATA_BASE_POINTER 0x4C7B1F0 // 48 8B 05 ? ? ? ? ? ? ? 4D 39 C8 75 ? 48 83 C1
		uint32_t parentToken = *(uint32_t*)((uintptr_t)klass + 0xA4); // 41 8B 87 ? ? ? ? 41 BF 00 00 00 00
		if (!parentToken) return nullptr;
		return (Il2CppClass*)(**(uintptr_t**)(Config::GameBase + METADATA_BASE_POINTER) + parentToken);
#undef METADATA_BASE_POINTER
	}

	Il2CppGenericClass* GetClassGenericClass(Il2CppClass* klass) {
		// 49 83 BF ? ? ? ? 00 0F 85 ? ? ? ? 48 89 75
		// if ((*(_BYTE*)(delegateType + 0xCA) & 1) != 0 || *(_QWORD*)(delegateType + 0x88))
		return *(Il2CppGenericClass**)((uintptr_t)klass + 0x88);
	}

	Il2CppGenericContext* GetGenericContext(Il2CppGenericClass* genericClass)
	{
		// In InitLocked 
		// 48 83 45 ? ? 31 F6
		return (Il2CppGenericContext*)((uintptr_t)genericClass + 0x8);
	}

	Il2CppGenericInst* GenericContextGetClassInst(Il2CppGenericContext* genericContext)
	{
		// In Object::Box
		// E8 ? ? ? ? 48 89 C7 F6 80 ? ? ? ? ? 75 ? 48 8D 05 ? ? ? ? 48 89 45 ? 48 8B 0D ? ? ? ? FF 15 ? ? ? ? 48 8D 55 ? 48 89 F9 E8 ? ? ? ? 48 8B 45 ? ? ? ? FF 15 ? ? ? ? 0F B7 87
		return *(Il2CppGenericInst**)(genericContext);
	}

	Il2CppType* GetClassType(Il2CppClass* klass) {
		// 48 83 C6 ? 48 8D 7D ? 48 89 F9
		return (Il2CppType*)((uintptr_t)klass + 0x70);
	}

	int16_t GetClassGenericContainerIndex(Il2CppClass* klass) {
		// if (cls->genericContainerIndex) continue;
		// v16 = *(__int16*)(*(_QWORD*)(v46 + 0x90) + 0x36LL) ^ 0xFFFF8DC0;
		return *(int16_t*)((uintptr_t)klass + 0x36) ^ 0xFFFF8DC0;
	}

	/*	uint8_t GetMethodParamCount(MethodInfo* method) {
			// E8 ? ? ? ? 3B C5 75
			return *(uint8_t*)((uintptr_t)method + 0x2E);
		}*/

	int16_t GetMethodSlot(MethodInfo* method) {
		// 48 C7 40 ? 00 00 00 00 ? ? ? 66 C7 40
		return *(int16_t*)((uintptr_t)method + 0x28);
	}

	uint16_t GetMethodFlags(MethodInfo* method) {
		return *(uint16_t*)((uintptr_t)method + 0x2A);
	}

	bool GetMethodIsGenric(MethodInfo* method) {
		// 41 F6 46 ? ? 0F 84 ? ? ? ? 48 89 D7
		return ((*(uint8_t*)(method + 0x2F) & 0x10) != 0);
	}

	uintptr_t GetMethodPointer(MethodInfo* method) {
		// 48 83 78 ? 00 74 ? 48 83 C4 ? 5E 5D
		// i think this is better: FF 50 ? 48 8B 4C 24 ? 48 89 FA
		return *(uintptr_t*)((uintptr_t)method + 0x8);
	}

	std::string GetTypeName(Il2CppType* type) {
		if (!type) return "unknown";
		if (g_cached_types.count(type)) return g_cached_types[type];

		// v10 is an array of 4 qwords in pseudocode -> we'll use a small struct
		uint64_t out[4] = { 0 };

		// Call the inlined formatter: out <- formatted string representation of 'type'
		type_get_name_tmp(out, type, 0);

		const char* cstr = nullptr;
		// pseudocode checks v10[3] >= 0x10 then uses v10[0] else uses inline buffer
		if (out[3] >= 0x10)
			cstr = (const char*)out[0];
		else
			cstr = reinterpret_cast<const char*>(&out[0]);

		std::string result = cstr ? std::string(cstr) : std::string("unknown");

		// Free temp if allocator used (sub_8D3AD0)
		free_temp_str(out);

		g_cached_types[type] = result;
		return result;
	}
}