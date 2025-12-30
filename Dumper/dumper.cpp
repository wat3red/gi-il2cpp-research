// dumper.cpp
#include "dumper.h"
#include "utils.h"

#include <unordered_set>
#include <set>
#include <functional> // Required for std::function

namespace Dumper {
	static const std::unordered_set<std::string> g_Keywords = {
		// C
		"auto","break","bool","case","char","const","continue","default","do","double",
		"else","enum","extern","float","for","goto","if","inline","int","long",
		"register","restrict","return","short","signed","sizeof","static","struct",
		"switch","typedef","union","unsigned","void","volatile","while",

		// C++
		"class","public","private","protected","template","typename","using",
		"namespace","operator","new","delete","this","virtual","override","final",
		"try","catch","throw","nullptr","true","false",

		// MSVC / IDA sensitive
		"__int8","__int16","__int32","__int64",
		"__fastcall","__stdcall","__thiscall","__cdecl",
		"__cppobj"
	};

	std::string SanitizeType(const std::string& in)
	{
		if (in.empty())
			return "";

		std::string out;
		out.reserve(in.size());

		for (char c : in) {
			if (c == '[' || c == ']')
				continue;
			else if (isalnum((unsigned char)c) || c == '_' || c == '*')
				out += c;
			else
				out += '_';
		}

		// не может начинаться с цифры
		if (isdigit((unsigned char)out[0]))
			out = "_" + out;

		return out;
	}

	std::string SanitizeIdentifier(const std::string& in)
	{
		std::string out;
		out.reserve(in.size());

		out = SanitizeType(in);

		// keywords
		if (g_Keywords.contains(out))
			out += "_";

		return out;
	}

	void CollectInflatedClasses(std::vector<Il2CppClass*>& allClasses) {
		std::set<Il2CppClass*> uniqueSet(allClasses.begin(), allClasses.end());
		size_t scanIndex = 0;

		// Scan indefinitely until we stop finding new classes
		while (scanIndex < allClasses.size()) {
			Il2CppClass* cls = allClasses[scanIndex];
			scanIndex++;

			if (!cls) continue;

			void* iter = nullptr;
			while (FieldInfo* field = Il2Cpp::class_get_fields(cls, &iter)) {
				Il2CppType* fType = Il2Cpp::field_get_type(field);
				if (!fType) continue;

				// Check if this field points to a class we haven't dumped yet
				if (fType->type == IL2CPP_TYPE_GENERICINST ||
					fType->type == IL2CPP_TYPE_VALUETYPE ||
					fType->type == IL2CPP_TYPE_CLASS) {

					Il2CppClass* targetClass = Il2Cpp::class_from_type(fType);

					// If it's a valid class and we haven't seen it, add it
					if (targetClass && uniqueSet.find(targetClass) == uniqueSet.end()) {

						// Optional: Filter out system primitives if you want
						// if (IsSystemPrimitive(targetClass)) continue;

						uniqueSet.insert(targetClass);
						allClasses.push_back(targetClass);
					}
				}
			}

			Il2CppClass* parentClass = Il2Cpp::GetClassParent(cls);
			if (parentClass) {
				Il2CppType* parentType = Il2Cpp::GetClassType(parentClass);
				// Check if this field points to a class we haven't dumped yet
				// If it's a valid class and we haven't seen it, add it
				if (parentType &&
					(parentType->type == IL2CPP_TYPE_GENERICINST ||
						parentType->type == IL2CPP_TYPE_VALUETYPE ||
						parentType->type == IL2CPP_TYPE_CLASS)) {
					if (parentClass && uniqueSet.find(parentClass) == uniqueSet.end()) {

						// Optional: Filter out system primitives if you want
						// if (IsSystemPrimitive(targetClass)) continue;

						uniqueSet.insert(parentClass);
						allClasses.push_back(parentClass);
					}
				}
			}

		}
	}

	// Helper to get unique Il2CppClass name including generic args
	std::string GetIl2CppClassName(Il2CppClass* klass) {
		std::string name = Il2Cpp::class_get_name(klass);
		std::string ns = Il2Cpp::class_get_namespace(klass);

		std::string fullName;
		if (!ns.empty()) fullName = ns + "_" + name;
		else fullName = name;

		Il2CppGenericClass* genericClass = Il2Cpp::GetClassGenericClass(klass);

		if (genericClass) {
			fullName += "_Gen";

			Il2CppGenericContext* genericContext = Il2Cpp::GetGenericContext(genericClass);
			if (genericContext) {
				Il2CppGenericInst* inst = Il2Cpp::GenericContextGetClassInst(genericContext);
				if (inst) {
					for (uint32_t i = 0; i < inst->type_argc; i++) {
						const Il2CppType* t = inst->type_argv[i];

						std::string argName = GetIl2CppClassName(Il2Cpp::class_from_type((Il2CppType*)t));
						//std::string argName = GetCType((Il2CppType*)t);

						argName = SanitizeType(argName);
						//std::string argName = GetTypeNaming((Il2CppType*)t);
						fullName += "_" + argName;
					}
				}
			}
		}
		return SanitizeType(fullName); // Sanitize the final combined result once
	}

	std::string GetCType(Il2CppType* type) {
		if (!type) return "";

		uint8_t typeEnum = Il2Cpp::GetTypeEnum(type);

		switch (typeEnum) {
		case IL2CPP_TYPE_VOID:    return "void";
		case IL2CPP_TYPE_BOOLEAN: return "bool";
		case IL2CPP_TYPE_I1:      return "int8_t";
		case IL2CPP_TYPE_U1:      return "uint8_t";
		case IL2CPP_TYPE_I2:      return "int16_t";
		case IL2CPP_TYPE_U2:      return "uint16_t";
		case IL2CPP_TYPE_CHAR:    return "uint16_t"; // UTF-16
		case IL2CPP_TYPE_I4:      return "int32_t";
		case IL2CPP_TYPE_U4:      return "uint32_t";
		case IL2CPP_TYPE_R4:      return "float";
		case IL2CPP_TYPE_I8:      return "int64_t";
		case IL2CPP_TYPE_U8:      return "uint64_t";
		case IL2CPP_TYPE_R8:      return "double";
		case IL2CPP_TYPE_I:
		case IL2CPP_TYPE_U:       return "intptr_t";
		case IL2CPP_TYPE_STRING:  return "struct Il2CppString*";
		case IL2CPP_TYPE_OBJECT:  return "struct Il2CppObject*";
		case IL2CPP_TYPE_VALUETYPE:
		{
			Il2CppClass* klass = Il2Cpp::class_from_type(type);
			if (klass) {
				Il2CppClass* parentClass = Il2Cpp::GetClassParent(klass);
				if (parentClass) {
					std::string pName = Il2Cpp::class_get_name(parentClass);
					std::string pNs = Il2Cpp::class_get_namespace(parentClass);
					std::string parentName = SanitizeType(pNs.empty() ? pName : pNs + "_" + pName);

					if (parentName == "System_Enum") {
						void* iter = nullptr;
						FieldInfo* valueField = nullptr;
						int enumCount = 0;
						while (FieldInfo* f = Il2Cpp::class_get_fields(klass, &iter)) {
							if (strcmp(Il2Cpp::field_get_name(f), "value__") == 0) valueField = f;
							else ++enumCount;
						}

						if (!valueField) return "void";

						return GetCType(Il2Cpp::field_get_type(valueField));
					}
				}

				std::string safeName = GetIl2CppClassName(klass);

				safeName = SanitizeType(safeName);

				return "struct " + safeName;
			}
			return "void";
		}
		case IL2CPP_TYPE_ARRAY:
		case IL2CPP_TYPE_SZARRAY:
		{
			Il2CppClass* elemType = Il2Cpp::type_get_class_or_element_class(type);
			std::string elem = GetCType(Il2Cpp::GetClassType(elemType));

			if (elem.empty())
				elem = "void*";

			return "Il2CppArray<" + elem + ">*";
		}

		case IL2CPP_TYPE_CLASS:
		case IL2CPP_TYPE_GENERICINST:
		{
			Il2CppClass* klass = Il2Cpp::class_from_type(type);
			if (klass) {
				std::string safeName = GetIl2CppClassName(klass);
				return "struct " + safeName + "*";
			}
			return "void*";
		}
		case IL2CPP_TYPE_PTR: return "void*";
		default: return "void*";
		}
	}

	void CollectAllClasses(std::vector<Il2CppClass*>& output) {
		__try {
			for (int32_t i = 0; ; ++i) {
				Il2CppClass* cls = Il2Cpp::MetadataCache__GetTypeInfoFromTypeDefinitionIndex(i);
				if (!cls) break;
				output.push_back(cls);
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			Utils::Log("Exception raised, collected %d classes\n", output.size());
		}
	}


	void SortClasses(std::vector<Il2CppClass*>& classes) {
		std::vector<Il2CppClass*> sorted;
		std::unordered_set<Il2CppClass*> visited;

		// Reserve to prevent frequent reallocations
		sorted.reserve(classes.size());

		// Recursive Lambda to visit dependencies first
		std::function<void(Il2CppClass*)> visit = [&](Il2CppClass* cls) {
			// If null or already visited, skip
			if (!cls || visited.count(cls)) return;

			// Mark visited immediately to prevent infinite recursion (cycles)
			visited.insert(cls);

			// 1. Must define Parent first (Inheritance)
			Il2CppClass* parent = Il2Cpp::GetClassParent(cls);
			if (parent) visit(parent);

			// 2. Must define ValueType fields first (Embedded structs)
			// (Pointers don't matter, but structs do)
			void* iter = nullptr;
			while (FieldInfo* field = Il2Cpp::class_get_fields(cls, &iter)) {
				Il2CppType* fType = Il2Cpp::field_get_type(field);
				if (!fType) continue;

				// Only strict dependency is VALUETYPE. 
				// CLASS/GENERICINST are usually pointers, so forward decl is enough.
				if (fType->type == IL2CPP_TYPE_VALUETYPE) {
					Il2CppClass* fieldClass = Il2Cpp::class_from_type(fType);
					if (fieldClass) visit(fieldClass);
				}
			}

			// 3. Add self to list after dependencies are satisfied
			sorted.push_back(cls);
			};

		// process all classes
		for (auto cls : classes) {
			visit(cls);
		}

		// Replace original list with sorted list
		classes = sorted;
	}

	void DumpForIDA() {
		FILE* hFile;
		fopen_s(&hFile, "ida.h", "w");
		if (!hFile) return;

		FILE* mFile;
		fopen_s(&mFile, "ida_methods.json", "w");
		if (!mFile) return;

		fprintf(hFile,
			R"""(typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef long long int64_t;
typedef unsigned long long uint64_t;
typedef long long intptr_t;
typedef unsigned long long uintptr_t;

struct Il2CppObject { void* klass; void* monitor; };
template <typename T>
struct Il2CppArray : Il2CppObject {
	void* bounds;
	int   max_length;
	T array[65535];

	T& operator [] (int i)
	{
		return array[i];
	}

	const T& operator [] (int i) const
	{
		return array[i];
	}

	bool Contains(T item)
	{
		for (int i = 0; i < max_length; i++)
		{
			if (array[i] == item) return true;
		}
		return false;
	}
};

struct Il2CppString { Il2CppObject* obj; int32_t length; char chars[1]; };)"""
);
		std::vector<Il2CppClass*> allClasses;

		Utils::Log("Collecting classes for struct dump...\n");
		CollectAllClasses(allClasses);

		Utils::Log("Discovering inflated generic instances...\n");
		CollectInflatedClasses(allClasses);

		Utils::Log("Sorting classes by dependency...\n");
		SortClasses(allClasses);

		// Forward Declarations
		Utils::Log("Writing forward declarations...\n");
		for (Il2CppClass* cls : allClasses) {
			std::string fullName = GetIl2CppClassName(cls);
			if (!fullName.empty() && Il2Cpp::GetClassType(cls)->type != IL2CPP_TYPE_VALUETYPE)
				fprintf(hFile, "typedef struct %s %s;\n", fullName.c_str(), fullName.c_str());
		}
		fprintf(hFile, "\n");

		bool firstMethod = true;
		fprintf(mFile, "{\n  \"ImageBase\": %llu,\n  \"Methods\": [\n", Config::GameBase);

		// Definitions
		Utils::Log("Writing definitions...\n");
		std::unordered_set<std::string> definedNames;

		for (Il2CppClass* cls : allClasses) {
			std::string name = Il2Cpp::class_get_name(cls);
			std::string ns = Il2Cpp::class_get_namespace(cls);
			std::string fullName = GetIl2CppClassName(cls);
			if (fullName.empty()) continue;

			if (definedNames.count(fullName)) continue;
			definedNames.insert(fullName);

			std::string parentName = "Il2CppObject";
			Il2CppClass* parentClass = Il2Cpp::GetClassParent(cls);
			if (parentClass)
				parentName = GetIl2CppClassName(parentClass);

			// STRUCT HANDLING
			void* iter = nullptr;
			// Don't generate body for Enums (yet)
			if (parentName != "System_Enum") {
				fprintf(hFile, "// Namespace: %s\nstruct %s : %s {\n", ns.c_str(), fullName.c_str(), parentName.c_str());

				// FIELDS
				while (FieldInfo* field = Il2Cpp::class_get_fields(cls, &iter)) {
					int flags = Il2Cpp::field_get_flags(field);
					if (flags & FIELD_ATTRIBUTE_STATIC) continue;

					std::string fName = SanitizeIdentifier(Il2Cpp::field_get_name(field));
					if (fName.empty()) continue;

					std::string typeStr = GetCType(Il2Cpp::field_get_type(field));
					int32_t offset = Il2Cpp::field_get_offset(field);

					fprintf(hFile, "    %s %s; // 0x%X\n", typeStr.c_str(), fName.c_str(), offset);
				}

				fprintf(hFile, "};\n\n");
			}

			// ===== METHODS =====
			iter = nullptr;
			while (MethodInfo* m = Il2Cpp::class_get_methods(cls, &iter)) {
				uint64_t rva = Il2Cpp::GetMethodPointer(m) - Config::GameBase;
				if (!rva) continue;

				std::string ret = GetCType(Il2Cpp::method_get_return_type(m));
				std::string mName = Il2Cpp::method_get_name(m);

				bool isStatic = (Il2Cpp::GetMethodFlags(m) & METHOD_ATTRIBUTE_STATIC) != 0;

				std::vector<std::pair<std::string, std::string>> params;
				uint8_t pc = Il2Cpp::GetMethodParamCount(m);
				for (uint8_t i = 0; i < pc; i++) {
					params.emplace_back(
						GetCType(Il2Cpp::method_get_param(m, i)),
						Il2Cpp::method_get_param_name(m, i)
					);
				}

				if (!firstMethod)
					fprintf(mFile, ",\n");
				firstMethod = false;

				fprintf(mFile,
					"    {\n"
					"      \"Class\": \"%s\",\n"
					"      \"Name\": \"%s\",\n"
					"      \"RVA\": %llu,\n"
					"      \"ReturnType\": \"%s\",\n"
					"      \"IsStatic\": %s,\n"
					"      \"Params\": [",
					name.c_str(),
					mName.c_str(),
					rva,
					ret.c_str(),
					isStatic ? "true" : "false"
				);

				for (size_t i = 0; i < params.size(); i++) {
					fprintf(mFile,
						"%s{\"Type\":\"%s\",\"Name\":\"%s\"}",
						i ? "," : "",
						params[i].first.c_str(),
						params[i].second.c_str()
					);
				}

				fprintf(mFile, "]\n    }");
			}

		}

		fclose(hFile);
		Utils::Log("IDA dump completed (ida.h + ida_methods.json)\n");
	}

	void DumpFull() {
		Utils::Log("Collecting classes for full dump...\n");
		std::vector<Il2CppClass*> allClasses;
		CollectAllClasses(allClasses);

		for (Il2CppClass* cls : allClasses) {
			// Simplified for brevity, logic remains identical to original
			std::string name = Il2Cpp::class_get_name(cls);
			std::string ns = Il2Cpp::class_get_namespace(cls);

			Il2CppClass* parent = Il2Cpp::GetClassParent(cls);
			std::string parentStr = parent ? Il2Cpp::class_get_name(parent) : "";

			Utils::Log("// Namespace: %s\nclass %s : %s\n{\n", ns.c_str(), name.c_str(), parentStr.c_str());
			//Utils::Log("// TypeDefIndex: %d\n// Namespace: %s\nclass %s : %s\n{\n", i, ns.c_str(), name.c_str(), parentStr.c_str());

			// Fields
			void* iter = nullptr;
			while (FieldInfo* f = Il2Cpp::class_get_fields(cls, &iter)) {
				int flags = Il2Cpp::field_get_flags(f);
				std::string typeName = Utils::StripNamespaces(Il2Cpp::GetTypeName(Il2Cpp::field_get_type(f)));
				Utils::Log("\t%s %s; // Offset: 0x%X, Flags: 0x%X\n", typeName.c_str(), Il2Cpp::field_get_name(f), Il2Cpp::field_get_offset(f), flags);
			}

			// Methods
			iter = nullptr;
			Utils::Log("\n\t// Methods\n");
			while (MethodInfo* m = Il2Cpp::class_get_methods(cls, &iter)) {
				std::string retType = Utils::StripNamespaces(Il2Cpp::GetTypeName(Il2Cpp::method_get_return_type(m)));
				std::string params = "";
				uint8_t count = Il2Cpp::GetMethodParamCount(m);
				for (uint8_t p = 0; p < count; p++) {
					params += Utils::StripNamespaces(Il2Cpp::GetTypeName(Il2Cpp::method_get_param(m, p)));
					params += " ";
					params += Il2Cpp::method_get_param_name(m, p);
					if (p < count - 1) params += ", ";
				}

				Utils::Log("\t%s %s(%s); // RVA: 0x%llX\n", retType.c_str(), Il2Cpp::method_get_name(m), params.c_str(), Il2Cpp::GetMethodPointer(m) - Config::GameBase);
			}
			Utils::Log("}\n\n");
		}
	}

	std::string GetMethodArgs(MethodInfo* method, bool includeNames) {
		std::string args = "";
		uint32_t count = Il2Cpp::GetMethodParamCount(method);

		for (uint32_t i = 0; i < count; i++) {
			Il2CppType* paramType = Il2Cpp::method_get_param(method, i);
			const char* paramName = Il2Cpp::method_get_param_name(method, i);

			// Исправление имен зарезервированных слов C++
			std::string safeParamName = paramName ? paramName : ("p" + std::to_string(i));
			if (safeParamName == "auto") safeParamName = "_auto";
			if (safeParamName == "register") safeParamName = "_register";
			if (safeParamName == "template") safeParamName = "_template";

			if (i > 0) args += ", ";
			args += GetCType(paramType);

			if (includeNames) {
				args += " " + safeParamName;
			}
		}
		return args;
	}

	// Генерация списка имен для вызова: "p0, p1"
	std::string GetMethodCallArgs(MethodInfo* method) {
		std::string args = "";
		uint32_t count = Il2Cpp::GetMethodParamCount(method);
		for (uint32_t i = 0; i < count; i++) {
			const char* paramName = Il2Cpp::method_get_param_name(method, i);
			std::string safeParamName = paramName ? paramName : ("p" + std::to_string(i));

			if (safeParamName == "auto") safeParamName = "_auto";
			if (safeParamName == "register") safeParamName = "_register";
			if (safeParamName == "template") safeParamName = "_template";

			if (i > 0) args += ", ";
			args += safeParamName;
		}
		return args;
	}

	void GenerateSDK() {
		FILE* file;
		fopen_s(&file, "sdk.h", "w");
		if (!file) return;

		const char* header = R"(// Generated by GIRuntimeDumper
#pragma once
#include <cstdint>

// User must define this in their codebase!
extern uintptr_t GameBase;

struct Il2CppObject;

// --- SDK Helpers ---
template<typename R, typename... Args>
inline R Call(uintptr_t rva, void* instance, Args... args) {
    typedef R(*FuncType)(void*, Args...);
    FuncType func = reinterpret_cast<FuncType>(GameBase + rva);
    return func(instance, args...);
}

template<typename R, typename... Args>
inline R CallStatic(uintptr_t rva, Args... args) {
    typedef R(*FuncType)(Args...);
    FuncType func = reinterpret_cast<FuncType>(GameBase + rva);
    return func(args...);
}

struct Il2CppString { struct Il2CppObject* obj; int32_t length; char chars[1]; };
struct Il2CppObject { void* vtable; void* monitor; };

)";
		fprintf(file, "%s", header);

		Utils::Log("Collecting classes for struct dump...\n");
		std::vector<Il2CppClass*> allClasses;
		CollectAllClasses(allClasses);

		// Forward Declarations
		Utils::Log("Writing forward declarations...\n");
		for (Il2CppClass* cls : allClasses) {
			std::string name = Il2Cpp::class_get_name(cls);
			std::string ns = Il2Cpp::class_get_namespace(cls);
			std::string fullName = SanitizeType(ns.empty() ? name : ns + "_" + name);
			fprintf(file, "typedef struct %s %s;\n", fullName.c_str(), fullName.c_str());
		}
		fprintf(file, "\n");

		// Definitions
		Utils::Log("Writing definitions...\n");
		for (Il2CppClass* cls : allClasses) {
			std::string name = Il2Cpp::class_get_name(cls);
			std::string ns = Il2Cpp::class_get_namespace(cls);
			std::string fullName = SanitizeType(ns.empty() ? name : ns + "_" + name);

			std::string parentName = "Il2CppObject";
			Il2CppClass* parentClass = Il2Cpp::GetClassParent(cls);
			if (parentClass) {
				std::string pName = Il2Cpp::class_get_name(parentClass);
				std::string pNs = Il2Cpp::class_get_namespace(parentClass);
				parentName = SanitizeType(pNs.empty() ? pName : pNs + "_" + pName);
			}

			// ENUM HANDLING
			if (parentName == "System_Enum") {
				void* iter = nullptr;
				FieldInfo* valueField = nullptr;
				int enumCount = 0;
				while (FieldInfo* f = Il2Cpp::class_get_fields(cls, &iter)) {
					if (strcmp(Il2Cpp::field_get_name(f), "value__") == 0) valueField = f;
					else ++enumCount;
				}

				if (!valueField) continue;

				std::string backingType = GetCType(Il2Cpp::field_get_type(valueField));
				uint8_t typeEnum = Il2Cpp::GetTypeEnum(Il2Cpp::field_get_type(valueField));

				fprintf(file, "// Namespace: %s\nenum class %s : %s {\n", ns.c_str(), fullName.c_str(), backingType.c_str());

				iter = nullptr;
				int index = 0;
				while (FieldInfo* f = Il2Cpp::class_get_fields(cls, &iter)) {
					if (f == valueField) continue;
					fprintf(file, "    %s = ", Il2Cpp::field_get_name(f));
					uint64_t raw = Il2Cpp::vm__GetEnumFieldValue(cls, f);

					// Format based on type (simplified)
					switch (typeEnum) {
					case IL2CPP_TYPE_I1:
						fprintf(file, "%d", (int8_t)raw);
						break;
					case IL2CPP_TYPE_U1:
						fprintf(file, "%u", (uint8_t)raw);
						break;
					case IL2CPP_TYPE_I2:
						fprintf(file, "%d", (int16_t)raw);
						break;
					case IL2CPP_TYPE_U2:
						fprintf(file, "%u", (uint16_t)raw);
						break;
					case IL2CPP_TYPE_I4:
						fprintf(file, "%d", (int32_t)raw);
						break;
					case IL2CPP_TYPE_U4:
						fprintf(file, "%u", (uint32_t)raw);
						break;
					case IL2CPP_TYPE_I8:
						fprintf(file, "%lld", (int64_t)raw);
						break;
					case IL2CPP_TYPE_U8:
						fprintf(file, "%llu", (uint64_t)raw);
						break;
					default:
						// unreachable
						break;
					}

					if (++index < enumCount) fprintf(file, ",\n");
				}
				fprintf(file, "\n};\n\n");
			}
			// STRUCT HANDLING
			else {
				fprintf(file, "// Namespace: %s\nstruct %s : %s {\n", ns.c_str(), fullName.c_str(), parentName.c_str());

				// FIELDS
				void* iter = nullptr;
				while (FieldInfo* field = Il2Cpp::class_get_fields(cls, &iter)) {
					int flags = Il2Cpp::field_get_flags(field);
					if (flags & FIELD_ATTRIBUTE_STATIC) continue;

					std::string fName = Il2Cpp::field_get_name(field);
					std::replace(fName.begin(), fName.end(), '<', '_');
					std::replace(fName.begin(), fName.end(), '>', '_');
					std::replace(fName.begin(), fName.end(), '.', '_');

					std::string typeStr = GetCType(Il2Cpp::field_get_type(field));
					int32_t offset = Il2Cpp::field_get_offset(field);

					fprintf(file, "    %s %s; // 0x%X\n", typeStr.c_str(), fName.c_str(), offset);
				}
				fprintf(file, "\n");

				// METHODS
				iter = nullptr;
				while (MethodInfo* method = Il2Cpp::class_get_methods(cls, &iter)) {
					uintptr_t methodPtr = Il2Cpp::GetMethodPointer(method);
					if (!methodPtr) continue;

					uintptr_t rva = methodPtr - Config::GameBase;
					if (!rva) continue;

					std::string mName = Il2Cpp::method_get_name(method);
					// Очистка имен операторов и прочего
					if (mName.find("op_") == 0 || mName.find('<') != std::string::npos || mName == ".ctor") {
						mName = "m_" + SanitizeType(mName);
					}

					uint16_t flags = Il2Cpp::GetMethodFlags(method);
					bool isStatic = (flags & METHOD_ATTRIBUTE_STATIC);

					std::string retType = GetCType(Il2Cpp::method_get_return_type(method));
					std::string argsDecl = GetMethodArgs(method, true);
					std::string argsCall = GetMethodCallArgs(method);

					fprintf(file, "    %s%s %s(%s) {\n",
						isStatic ? "static " : "",
						retType.c_str(),
						mName.c_str(),
						argsDecl.c_str()
					);

					if (isStatic) {
						fprintf(file, "        return CallStatic<%s>(0x%llX%s%s);\n",
							retType.c_str(),
							rva,
							argsCall.empty() ? "" : ", ",
							argsCall.c_str()
						);
					}
					else {
						fprintf(file, "        return Call<%s>(0x%llX, this%s%s);\n",
							retType.c_str(),
							rva,
							argsCall.empty() ? "" : ", ",
							argsCall.c_str()
						);
					}
					fprintf(file, "    }\n");
				}

				fprintf(file, "};\n\n");
			}
		}
		fclose(file);
		Utils::Log("Struct dump completed: structs.h\n");
	}
}