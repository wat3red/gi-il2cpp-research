using namespace Dumper;
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

		if (!fullName.empty() && ((Il2Cpp::GetClassType(cls)->type == IL2CPP_TYPE_GENERICINST) || (Il2Cpp::GetClassType(cls)->type == IL2CPP_TYPE_CLASS)))
			fprintf(hFile, "typedef struct %s %s;\n", fullName.c_str(), fullName.c_str());
	}
	fprintf(hFile, "\n");

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

			std::string typeStr = GetCType(Il2Cpp::field_get_type(field));
			int32_t offset = Il2Cpp::field_get_offset(field) - ((parentName.empty()) ? sizeof(Il2CppObject) : 0);

			fprintf(hFile, "    %s %s; // 0x%X\n", typeStr.c_str(), fName.c_str(), offset);
		}

		fprintf(hFile, "};\n\n");

		// -------------------------------------------------------
		continue;

		// ===== METHODS =====
		iter = nullptr;
		while (MethodInfo* m = Il2Cpp::class_get_methods(cls, &iter)) {
			uint64_t rva = Il2Cpp::GetMethodPointer(m) - Config::GameBase;
			if (!rva) continue;

			std::string ret = GetCType(Il2Cpp::method_get_return_type(m));
			std::string mName = Il2Cpp::method_get_name(m);

			bool isStatic = (Il2Cpp::GetMethodFlags(m) & METHOD_ATTRIBUTE_STATIC) != 0;

			std::vector<std::pair<std::string, std::string>> params;
			uint8_t pc = Il2Cpp::method_get_param_count(m);
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
				"      \"ClassCTypeName\": \"%s\",\n"
				"      \"Name\": \"%s\",\n"
				"      \"RVA\": %llu,\n"
				"      \"ReturnType\": \"%s\",\n"
				"      \"IsStatic\": %s,\n"
				"      \"Params\": [",
				SanitizeIdentifier(name.c_str()).c_str(),
				GetCType(Il2Cpp::GetClassType(cls)).c_str(),
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
