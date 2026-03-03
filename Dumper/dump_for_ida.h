using namespace Dumper;

// Forward declare for context-aware type generation during IDA dump
static std::unordered_map<std::string, std::string> g_IDAArrayTypes;
static std::unordered_set<std::string> g_IDAGeneratedArrays;

std::string GetCTypeForIDA(Il2CppType* type) {
	if (!type) return "";

	uint8_t typeEnum = Il2Cpp::GetTypeEnum(type);

	switch (typeEnum) {
	case IL2CPP_TYPE_VOID:    return "void";
	case IL2CPP_TYPE_BOOLEAN: return "bool";
	case IL2CPP_TYPE_I1:      return "int8_t";
	case IL2CPP_TYPE_U1:      return "uint8_t";
	case IL2CPP_TYPE_I2:      return "int16_t";
	case IL2CPP_TYPE_U2:      return "uint16_t";
	case IL2CPP_TYPE_CHAR:    return "uint16_t";
	case IL2CPP_TYPE_I4:      return "int32_t";
	case IL2CPP_TYPE_U4:      return "uint32_t";
	case IL2CPP_TYPE_R4:      return "float";
	case IL2CPP_TYPE_I8:      return "int64_t";
	case IL2CPP_TYPE_U8:      return "uint64_t";
	case IL2CPP_TYPE_R8:      return "double";
	case IL2CPP_TYPE_I:
	case IL2CPP_TYPE_U:       return "intptr_t";
	case IL2CPP_TYPE_STRING:  return "Il2CppString*";
	case IL2CPP_TYPE_OBJECT:  return "Il2CppObject*";
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
					while (FieldInfo* f = Il2Cpp::class_get_fields(klass, &iter)) {
						if (strcmp(Il2Cpp::field_get_name(f), "value__") == 0) valueField = f;
					}

					if (!valueField) return "void";
					return GetCTypeForIDA(Il2Cpp::field_get_type(valueField));
				}
			}

			std::string safeName = GetIl2CppClassName(klass);
			safeName = SanitizeType(safeName);
			return safeName;
		}
		return "void";
	}
	case IL2CPP_TYPE_ARRAY:
	case IL2CPP_TYPE_SZARRAY:
	{
		Il2CppClass* elemType = Il2Cpp::type_get_class_or_element_class(type);
		std::string elem = GetCTypeForIDA(Il2Cpp::GetClassType(elemType));

		if (elem.empty())
			elem = "void_ptr";

		std::string arrayTypeName = "Il2CppArray_" + elem;
		g_IDAArrayTypes[arrayTypeName] = elem;
		return arrayTypeName + "*";
	}

	case IL2CPP_TYPE_CLASS:
	case IL2CPP_TYPE_GENERICINST:
	{
		Il2CppClass* klass = Il2Cpp::class_from_type(type);
		if (klass) {
			std::string safeName = GetIl2CppClassName(klass);
			return safeName + "*";
		}
		return "void*";
	}
	case IL2CPP_TYPE_PTR: return "void*";
	default: return "void*";
	}
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
struct Il2CppString { Il2CppObject* obj; int32_t length; char chars[1]; };)"""
	);
	std::vector<Il2CppClass*> allClasses;

	Utils::Log("Collecting classes for struct dump...\n");
	CollectAllClasses(allClasses);

	Utils::Log("Discovering inflated generic instances...\n");
	CollectInflatedClasses(allClasses);

	Utils::Log("Sorting classes by dependency...\n");
	SortClasses(allClasses);

	// PASS 1: Collect all array types used
	Utils::Log("Collecting array types...\n");
	g_IDAArrayTypes.clear();
	g_IDAGeneratedArrays.clear();

	for (Il2CppClass* cls : allClasses) {
		void* iter = nullptr;
		while (FieldInfo* field = Il2Cpp::class_get_fields(cls, &iter)) {
			Il2CppType* fType = Il2Cpp::field_get_type(field);
			if (fType) GetCTypeForIDA(fType);
		}
	}

	// Forward Declarations
	Utils::Log("Writing forward declarations...\n");
	for (Il2CppClass* cls : allClasses) {
		std::string fullName = GetIl2CppClassName(cls);

		if (!fullName.empty() && ((Il2Cpp::GetClassType(cls)->type == IL2CPP_TYPE_GENERICINST) || (Il2Cpp::GetClassType(cls)->type == IL2CPP_TYPE_CLASS)))
			fprintf(hFile, "typedef struct %s %s;\n", fullName.c_str(), fullName.c_str());
	}
	fprintf(hFile, "\n");

	// PASS 2: Generate array type definitions
	Utils::Log("Writing array type definitions...\n");
	for (const auto& pair : g_IDAArrayTypes) {
		const std::string& arrayTypeName = pair.first;
		const std::string& elemType = pair.second;

		fprintf(hFile, "struct %s : Il2CppObject {\n", arrayTypeName.c_str());
		fprintf(hFile, "    void* bounds;\n");
		fprintf(hFile, "    int max_length;\n");
		fprintf(hFile, "    %s array[65535];\n", elemType.c_str());
		fprintf(hFile, "};\n\n");

		g_IDAGeneratedArrays.insert(arrayTypeName);
	}

	bool firstMethod = true;
	fprintf(mFile, "{\n  \"ImageBase\": %llu,\n  \"Methods\": [\n", Config::GameBase);

	// Definitions
	Utils::Log("Writing definitions...\n");

	for (Il2CppClass* cls : allClasses) {
		std::string name = Il2Cpp::class_get_name(cls);
		std::string ns = Il2Cpp::class_get_namespace(cls);
		std::string fullName = GetIl2CppClassName(cls);
		if (fullName.empty()) continue;

		std::string parentName = "Il2CppObject";
		Il2CppClass* parentClass = Il2Cpp::GetClassParent(cls);
		if (parentClass)
			parentName = GetIl2CppClassName(parentClass);

		// STRUCT HANDLING
		void* iter = nullptr;
		// Don't generate body for Enums (yet)
		if (parentName == "System_Enum") continue;
		if (parentName == "System_ValueType") parentName = "";

		// Struct Header
		if (parentName.empty())
			fprintf(hFile, "// Namespace: %s\nstruct %s {\n", ns.c_str(), fullName.c_str());
		else
			fprintf(hFile, "// Namespace: %s\nstruct %s : %s {\n", ns.c_str(), fullName.c_str(), parentName.c_str());

		// FIELDS
		while (FieldInfo* field = Il2Cpp::class_get_fields(cls, &iter)) {
			int flags = Il2Cpp::field_get_flags(field);
			if (flags & FIELD_ATTRIBUTE_STATIC) continue;

			std::string fName = SanitizeIdentifier(Il2Cpp::field_get_name(field));
			if (fName.empty()) continue;

			std::string typeStr = GetCTypeForIDA(Il2Cpp::field_get_type(field));
			int32_t offset = Il2Cpp::field_get_offset(field) - ((parentName.empty()) ? sizeof(Il2CppObject) : 0);

			fprintf(hFile, "    %s %s; // 0x%X\n", typeStr.c_str(), fName.c_str(), offset);
		}

		fprintf(hFile, "};\n\n");

		// -------------------------------------------------------
		// ===== METHODS =====

		iter = nullptr;
		while (MethodInfo* m = Il2Cpp::class_get_methods(cls, &iter)) {
			uint64_t rva = Il2Cpp::GetMethodPointer(m) - Config::GameBase;
			if (!rva) continue;

			std::string ret = GetCTypeForIDA(Il2Cpp::method_get_return_type(m));
			std::string mName = Il2Cpp::method_get_name(m);

			bool isStatic = (Il2Cpp::GetMethodFlags(m) & METHOD_ATTRIBUTE_STATIC) != 0;

			std::vector<std::pair<std::string, std::string>> params;
			uint8_t pc = Il2Cpp::method_get_param_count(m);
			for (uint8_t i = 0; i < pc; i++) {
				params.emplace_back(
					GetCTypeForIDA(Il2Cpp::method_get_param(m, i)),
					SanitizeIdentifier(Il2Cpp::method_get_param_name(m, i))
				);
			}

			if (!firstMethod)
				fprintf(mFile, ",\n");
			firstMethod = false;

			fprintf(mFile,
				"    {\n"
				"      \"Class\": \"%s\",\n"
				"      \"ClassCTypeName\": \"%s\",\n"
				"      \"Name\": \"%s\",\n"
				"      \"RVA\": %llu,\n"
				"      \"ReturnType\": \"%s\",\n"
				"      \"IsStatic\": %s,\n"
				"      \"Params\": [",
				SanitizeIdentifier(name.c_str()).c_str(),
				GetCTypeForIDA(Il2Cpp::GetClassType(cls)).c_str(),
				SanitizeIdentifier(mName.c_str()).c_str(),
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

	fprintf(mFile, "\n    ]\n}");
	fclose(hFile);

	Utils::Log("IDA dump completed (ida.h + ida_methods.json)\n");
}
