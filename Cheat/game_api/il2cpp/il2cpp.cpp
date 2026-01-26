#include "il2cpp.h"
#include "../functions/resolve_funcs.h"	
#include <logger/logger.h>	

#include <windows.h>	
#include <unordered_map>	

struct AssemblyVector {
	Il2CppAssembly** begin;
	Il2CppAssembly** end;
	Il2CppAssembly** cap;
};

Il2CppAssembly** il2cpp_domain_get_assemblies(size_t* size) {
	// xref: 48 8B 3D ? ? ? ? 48 39 DF
	auto* vec = reinterpret_cast<AssemblyVector*>(g_game_base + 0x4E195F8);
	*size = static_cast<size_t>(vec->end - vec->begin);
	return vec->begin;
}

Il2CppClass* Il2Cpp::Class::FromName(const char* namespaceName, const char* className) {
	struct ClassKey {
		std::string ns;
		std::string name;

		bool operator==(const ClassKey& o) const {
			return ns == o.ns && name == o.name;
		}
	};

	struct ClassKeyHash {
		size_t operator()(const ClassKey& k) const noexcept {
			return std::hash<std::string>()(k.ns) ^
				(std::hash<std::string>()(k.name) << 1);
		}
	};

	static std::unordered_map<ClassKey, Il2CppClass*, ClassKeyHash> cache;

	ClassKey key{ namespaceName, className };

	// O(1) fast path
	if (auto it = cache.find(key); it != cache.end())
		return it->second;

	size_t asmCount = 0;
	Il2CppAssembly** assemblies = il2cpp_domain_get_assemblies(&asmCount);

	for (size_t i = 0; i < asmCount; ++i) {
		Il2CppAssembly* asmbl = assemblies[i];
		if (!asmbl)
			continue;

		Il2CppImage* image = il2cpp_assembly_get_image(asmbl);
		if (!image)
			continue;

		/*const char* imgName = il2cpp_image_get_name(image);
		if (!imgName || strcmp(imgName, assemblyName) != 0)
			continue;*/

		if (Il2CppClass* klass = il2cpp_class_from_name(image, namespaceName, className)) {
			cache.emplace(key, klass);
			Log("Found class %s.%s\n", namespaceName, className);
			return klass;
		}
	}

	cache.emplace(key, nullptr);
	Log("Didn't find class named %s.%s\n", namespaceName, className);

	return nullptr;
}

/*static const std::vector<Il2CppClass*>& GetAllClasses()
{
	static std::vector<Il2CppClass*> classes;
	static bool initialized = false;

	if (initialized)
		return classes;

	__try {
		for (int32_t i = 0; ; ++i) {
			Il2CppClass* cls = MetadataCache_GetTypeInfoFromTypeDefinitionIndex(i);
			if (!cls)
				break;

			classes.push_back(cls);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		// swallow: partial metadata is acceptable
	}

	initialized = true;
	return classes;
}*/

void* Il2Cpp::Method::GetMethodPointer(MethodInfo* method) {
	// 48 83 78 ? 00 74 ? 48 83 C4 ? 5E 5D
	// i think this is better: FF 50 ? 48 8B 4C 24 ? 48 89 FA
	return *(void**)((uintptr_t)method + 0x8);
}

/*Il2CppClass* Il2Cpp::Class::FromName(const char* namespaceName, const char* className)
{
	struct ClassKey {
		std::string ns;
		std::string name;

		bool operator==(const ClassKey& o) const {
			return ns == o.ns && name == o.name;
		}
	};

	struct ClassKeyHash {
		size_t operator()(const ClassKey& k) const noexcept {
			return std::hash<std::string>()(k.ns) ^
				(std::hash<std::string>()(k.name) << 1);
		}
	};

	static std::unordered_map<ClassKey, Il2CppClass*, ClassKeyHash> cache;

	ClassKey key{ namespaceName, className };

	// O(1) fast path
	if (auto it = cache.find(key); it != cache.end())
		return it->second;

	const auto& classes = GetAllClasses();

	for (Il2CppClass* cls : classes) {
		if (!cls)
			continue;

		const char* name = il2cpp_class_get_name(cls);
		const char* ns = il2cpp_class_get_namespace(cls);

		if (!name || !ns)
			continue;

		if (strcmp(name, className) == 0 &&
			strcmp(ns, namespaceName) == 0)
		{
			cache.emplace(key, cls);
			Log("Found class %s.%s\n", namespaceName, className);

			return cls;
		}
	}

	cache.emplace(key, nullptr);
	Log("Didn't find class named %s.%s\n", namespaceName, className);
	return nullptr;
}*/

MethodInfo* Il2Cpp::Method::Find(Il2CppClass* klass, const char* method_name, int param_count) {
	//Log("Il2Cpp::Method::Find: starting to search for method %s, klass = %p, param_count = %d\n", method_name, klass, param_count);

	void* iter = nullptr;
	while (MethodInfo* method = il2cpp_class_get_methods(klass, &iter)) {
		//Log("iterating over method %s with param_count = %d, RVA = 0x%X\n", il2cpp_method_get_name(method), il2cpp_method_get_param_count(method), (uintptr_t)GetMethodPointer(method) - g_game_base);

		if ((strcmp(method_name, il2cpp_method_get_name(method)) == 0) &&
			(il2cpp_method_get_param_count(method) == param_count)) {
			//Log("Il2Cpp::Method::Find: found method %s.%s.%s at 0x%X\n", il2cpp_class_get_namespace(klass), il2cpp_class_get_name(klass), method_name, method);

			return method;
		}
	}

	//Log("Il2Cpp::Method::Find: didn't find method %s.%s.%s\n", il2cpp_class_get_namespace(klass), il2cpp_class_get_name(klass), method_name);

	return nullptr;
}

MethodInfo* Il2Cpp::Method::Find(const char* namespace_name, const char* class_name, const char* method_name, int param_count) {
	Il2CppClass* klass = Il2Cpp::Class::FromName(namespace_name, class_name);
	if (!klass) return nullptr;

	MethodInfo* method = Il2Cpp::Method::Find(klass, method_name, param_count);
	return method;
}

int32_t Il2Cpp::Field::GetOffset(Il2CppClass* klass, const char* fieldName) {
	FieldInfo* fieldInfo = Il2Cpp::Field::Find(klass, fieldName);

	if (fieldInfo)
		return Il2Cpp::Field::GetOffset(fieldInfo);

	Log("Il2Cpp::Field::GetOffset: didn't find %s in %s", fieldName, il2cpp_class_get_name(klass));
	return -1;
}

int32_t Il2Cpp::Field::GetOffset(FieldInfo* field) {
	return il2cpp_field_get_offset(field);
}

FieldInfo* Il2Cpp::Field::Find(Il2CppClass* klass, const char* fieldName) {
	return il2cpp_class_get_field_from_name(klass, fieldName);
}
