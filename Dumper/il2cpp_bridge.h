// il2cpp_bridge.h
#pragma once
#include "common.h"
#include "il2cpp_types.h" // Your existing types header

namespace Il2Cpp {
	// === Function Pointers ===
	extern MethodInfo* (*class_get_methods)(Il2CppClass* klass, void** iter);
	extern const char* (*class_get_name)(Il2CppClass* klass);
	extern const char* (*class_get_namespace)(Il2CppClass* klass);
	extern FieldInfo* (*class_get_fields)(Il2CppClass* klass, void** iter);
	extern Il2CppClass* (*class_from_type)(const Il2CppType* type);

	extern const char* (*method_get_name)(MethodInfo* method);
	extern uint8_t(*method_get_param_count)(MethodInfo* method);
	extern const char* (*method_get_param_name)(MethodInfo* method, uint32_t index);
	extern Il2CppType* (*method_get_param)(MethodInfo* method, uint32_t index);
	extern Il2CppType* (*method_get_return_type)(MethodInfo* method);

	extern const char* (*field_get_name)(FieldInfo* field);
	extern int (*field_get_flags)(FieldInfo* field);
	extern Il2CppType* (*field_get_type)(FieldInfo* field);
	extern int32_t(*field_get_offset)(FieldInfo* field);

	extern void (*type_get_name_tmp)(void* out_str_struct, Il2CppType* type, int format);
	extern void (*free_temp_str)(void* out_str_struct);
	extern Il2CppClass* (*type_get_class_or_element_class)(Il2CppType* type);

	extern Il2CppClass* (*MetadataCache__GetTypeInfoFromTypeDefinitionIndex)(int32_t typeDefinitionIndex);
	extern uint64_t(*vm__GetEnumFieldValue)(Il2CppClass* enumType, FieldInfo* field);

	// === Helper Functions ===
	void Initialize();

	// Memory readers (reading direct offsets)
	uint8_t GetTypeEnum(Il2CppType* type);
	uint32_t GetClassSize(Il2CppClass* klass);
	Il2CppClass* GetClassParent(Il2CppClass* klass);
	Il2CppType* GetClassType(Il2CppClass* klass);
	//int16_t GetClassGenericContainerIndex(Il2CppClass* klass);
	bool ClassIsGeneric(Il2CppClass* klass);
	Il2CppGenericClass* GetClassGenericClass(Il2CppClass* klass);
	Il2CppGenericContext* GetGenericContext(Il2CppGenericClass* genericClass);
	Il2CppGenericInst* GenericContextGetClassInst(Il2CppGenericContext* genericContext);

	//uint8_t GetMethodParamCount(MethodInfo* method);
	int16_t GetMethodSlot(MethodInfo* method);
	uint16_t GetMethodFlags(MethodInfo* method);
	bool GetMethodIsGenric(MethodInfo* method);
	uintptr_t GetMethodPointer(MethodInfo* method);

	// Formatter
	std::string GetTypeName(Il2CppType* type);
}