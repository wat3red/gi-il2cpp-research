// il2cpp_bridge.cpp
#include "il2cpp_bridge.h"
#include "utils.h"
#include "memory/signature.h"

// Define Global Config
uintptr_t Config::GameBase = 0;
bool Config::BlockPackets = true;

namespace Il2Cpp
{
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
		class_get_methods = (decltype(class_get_methods))((uintptr_t)Mem::Signature("E8 ? ? ? ? 48 85 C0 74 ? 48 8D 5D").ScanXref());
		class_get_name = (decltype(class_get_name))((uintptr_t)Mem::Signature("E8 ? ? ? ? 48 8B F8 C7 45 ? ? ? ? ? 48 8B 45").ScanXref());
		class_get_namespace = (decltype(class_get_namespace))((uintptr_t)Mem::Signature("E8 ? ? ? ? 48 8B 15 ? ? ? ? 48 8B C8 48 8B F8").ScanXref());
		class_get_fields = (decltype(class_get_fields))((uintptr_t)Mem::Signature("E8 ? ? ? ? 48 85 C0 75 ? E9 ? ? ? ? 89 E8").ScanXref());
		class_from_type = (decltype(class_from_type))((uintptr_t)Mem::Signature("E8 ? ? ? ? 48 89 C6 44 0F B7 B0").ScanXref());
		field_get_name = (decltype(field_get_name))((uintptr_t)Mem::Signature("E8 ? ? ? ? 48 89 C3 EB ? 83 BD").ScanXref());
		field_get_flags = (decltype(field_get_flags))((uintptr_t)Mem::Signature("E8 ? ? ? ? 48 8B CB 41 89 46").ScanXref());
		field_get_type = (decltype(field_get_type))((uintptr_t)Mem::Signature("E8 ? ? ? ? 48 8B C8 49 89 46 ? E8").ScanXref());
		field_get_offset = (decltype(field_get_offset))((uintptr_t)Mem::Signature("E8 ? ? ? ? 49 03 45").ScanXref());
		method_get_name = (decltype(method_get_name))((uintptr_t)Mem::Signature("E8 ? ? ? ? 48 8B CE 48 2B C6").ScanXref());
		method_get_param_count = (decltype(method_get_param_count))((uintptr_t)Mem::Signature("E8 ? ? ? ? 3B C5 75").ScanXref());
		method_get_param_name = (decltype(method_get_param_name))((uintptr_t)Mem::Signature("56 48 83 EC ? 0F B6 41 ? 39 D0 76 ? 89 D6 48 8B 51 ? 48 85 D2 74 ? 48 B8 ? ? ? ? ? ? ? ? 48 03 42 ? 74 ? 89 F1 ? ? ? ? ? ? ? ? 48 83 C4").Scan());
		method_get_param = (decltype(method_get_param))((uintptr_t)Mem::Signature("E8 ? ? ? ? 48 8B C8 E8 ? ? ? ? 4C 8B 4E").ScanXref());
		method_get_return_type = (decltype(method_get_return_type))((uintptr_t)Mem::Signature("E8 ? ? ? ? 48 83 C4 ? 48 89 C7 0F B6 47").ScanXref());
		type_get_name_tmp = (decltype(type_get_name_tmp))((uintptr_t)Mem::Signature("E8 ? ? ? ? 4C 8D 05 ? ? ? ? 48 8D 4D ? 48 8D 55 ? E8 ? ? ? ? 48 89 E9 4C 8D 45").ScanXref());
		free_temp_str = (decltype(free_temp_str))((uintptr_t)Mem::Signature("E8 ? ? ? ? B3 ? E9 ? ? ? ? 4C 8B B6").ScanXref());
		type_get_class_or_element_class = (decltype(type_get_class_or_element_class))((uintptr_t)Mem::Signature("0F BE 41 ? 83 F8 ? 74").Scan()); // Return class if type is not an array. Return element type if it is an array.
		MetadataCache__GetTypeInfoFromTypeDefinitionIndex = (decltype(MetadataCache__GetTypeInfoFromTypeDefinitionIndex))((uintptr_t)Mem::Signature("E8 ? ? ? ? 0F B7 A8").ScanXref());
		vm__GetEnumFieldValue = (decltype(vm__GetEnumFieldValue))((uintptr_t)Mem::Signature("E8 ? ? ? ? 48 39 44 24 ? 8B 54 24").ScanXref());
	}

	uint8_t GetTypeEnum(Il2CppType* type) {
		// unchanged since 6.2
		// 0F B6 46 ? C1 E0 ? 3D ? ? ? ? 75 ? 8B 05
		return *(uint8_t*)((uintptr_t)type + 0xA);
	}

	uint32_t GetClassSize(Il2CppClass* klass) {
		// 0F B7 9F ? ? ? ? 48 89 F9
		return (uint32_t)((*(int16_t*)((uintptr_t)klass + 0xBC)) - 4);
	}

	Il2CppClass* GetClassParent(Il2CppClass* klass) {
#define METADATA_BASE_POINTER 0x4EEA930 // 48 8B 05 ? ? ? ? ? ? ? 4D 39 C8 75 ? 48 83 C1
		uint32_t parentToken = *(uint32_t*)((uintptr_t)klass + 0xA0); // 41 8B 86 ? ? ? ? 41 BE 00 00 00 00
		if (!parentToken) return nullptr;
		return (Il2CppClass*)(**(uintptr_t**)(Config::GameBase + METADATA_BASE_POINTER) + parentToken);
#undef METADATA_BASE_POINTER
	}

	Il2CppGenericClass* GetClassGenericClass(Il2CppClass* klass) {
		// 49 83 BF ? ? ? ? 00 0F 85 ? ? ? ? 48 89 75
		// if ((*(_BYTE*)(delegateType + 0xCA) & 1) != 0 || *(_QWORD*)(delegateType + 0x88))
		return *(Il2CppGenericClass**)((uintptr_t)klass + 0x88);
	}

	Il2CppGenericContext* GetGenericContext(Il2CppGenericClass* genericClass) {
		// In InitLocked 
		// 48 83 45 ? ? 31 F6
		return (Il2CppGenericContext*)((uintptr_t)genericClass + 0x8);
	}

	Il2CppGenericInst* GenericContextGetClassInst(Il2CppGenericContext* genericContext) {
		// In Object::Box
		// E8 ? ? ? ? 48 89 C7 F6 80 ? ? ? ? ? 75 ? 48 8D 05 ? ? ? ? 48 89 45 ? 48 8B 0D ? ? ? ? FF 15 ? ? ? ? 48 8D 55 ? 48 89 F9 E8 ? ? ? ? 48 8B 45 ? ? ? ? FF 15 ? ? ? ? 0F B7 87
		return *(Il2CppGenericInst**)(genericContext);
	}

	Il2CppType* GetClassType(Il2CppClass* klass) {
		// 48 83 C6 ? 48 8D 7D ? 48 89 F9
		return (Il2CppType*)((uintptr_t)klass + 0x68);
	}

	int16_t GetClassGenericContainerIndex(Il2CppClass* klass) {
		// search for "The number of generic arguments provided doesn't equal the arity of the generic type definition."
		// if (cls->genericContainerIndex) continue;
		// v17 = (unsigned int)(__int16)(*(_WORD*)(*(_QWORD*)(v49 + 0x40) + 0x3ELL) - 0x2A41);
		return *(int16_t*)((uintptr_t)klass + 0x3E) - 0x2A41;
	}

	int16_t GetMethodSlot(MethodInfo* method) {
		// unchanged since 6.2
		// 48 C7 40 ? 00 00 00 00 ? ? ? 66 C7 40
		return *(int16_t*)((uintptr_t)method + 0x28);
	}

	uint16_t GetMethodFlags(MethodInfo* method) {
		// 0F B7 43 ? 89 C1
		return *(uint16_t*)((uintptr_t)method + 0x28);
	}

	bool GetMethodIsGenric(MethodInfo* method) {
		// 41 F6 46 ? ? 0F 84 ? ? ? ? 48 89 D7
		return ((*(uint8_t*)(method + 0x2F) & 4) != 0);
	}

	uintptr_t GetMethodPointer(MethodInfo* method) {
		// to find whole function: 55 56 48 83 EC ? 48 8D 6C 24 ? 48 C7 45 ? ? ? ? ? F6 41
		// 6.3 find access to the field: 48 83 78 ? 00 74 ? 48 83 C4 ? 5E 5D
		return *(uintptr_t*)((uintptr_t)method);
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

	static size_t GetIl2CppTypeSize(const Il2CppType* type) {
		if (!type) return 0;

		switch (type->type) {
		case IL2CPP_TYPE_BOOLEAN: return 1;
		case IL2CPP_TYPE_I1: case IL2CPP_TYPE_U1: return 1;
		case IL2CPP_TYPE_I2: case IL2CPP_TYPE_U2: return 2;
		case IL2CPP_TYPE_CHAR: return 2;
		case IL2CPP_TYPE_I4: case IL2CPP_TYPE_U4: case IL2CPP_TYPE_R4: return 4;
		case IL2CPP_TYPE_I8: case IL2CPP_TYPE_U8: case IL2CPP_TYPE_R8: return 8;
		case IL2CPP_TYPE_PTR: case IL2CPP_TYPE_CLASS: case IL2CPP_TYPE_STRING:
		case IL2CPP_TYPE_OBJECT: case IL2CPP_TYPE_SZARRAY: case IL2CPP_TYPE_ARRAY:
			return sizeof(void*);
		default:
			break;
		}
		if (type->type == IL2CPP_TYPE_VALUETYPE) {
			Il2CppClass* klass = Il2Cpp::class_from_type(type);
			if (!klass) return 0;

			size_t valSize = Il2Cpp::GetClassSize(klass);

			if (valSize > 0) return valSize;
		}

		return 0;
	}

	bool MethodHasReturnBuffer(MethodInfo* method) {
		Il2CppType* ret = Il2Cpp::method_get_return_type(method);
		if (!ret) return false;

		Il2CppClass* cls = Il2Cpp::class_from_type(ret);
		if (!cls) return false;

		// F6 41 ? ? 0F 85 ? ? ? ? 4C 89 C3
		//bool byref = ((*(uint8_t*)((uint8_t*)ret + 0xB) & 0x40) != 0);

		if (ret->type != IL2CPP_TYPE_VALUETYPE)
			return false;

		Il2CppClass* parentClass = Il2Cpp::GetClassParent(cls);

		if (strcmp(Il2Cpp::class_get_name(parentClass), "Enum") == 0 && strcmp(Il2Cpp::class_get_namespace(parentClass), "System") == 0)
			return false;

		size_t size = GetIl2CppTypeSize(ret);
		if (size == 0)
			return false;

		return size > 8;
	}
}