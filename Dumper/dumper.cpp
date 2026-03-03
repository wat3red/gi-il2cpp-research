// dumper.cpp
#include "dumper.h"
#include "utils.h"

#include <unordered_set>
#include <set>
#include <functional> // Required for std::function

namespace Dumper {
	static const std::unordered_set<std::string> g_Keywords = {
		"auto","break","bool","case","char","const","continue","default","do","double",
		"else","enum","extern","float","for","goto","if","inline","int","long",
		"register","restrict","return","short","signed","sizeof","static","struct",
		"switch","typedef","union","unsigned","void","volatile","while","xor","not","and","or",
		"class","public","private","protected","template","typename","using",
		"namespace","operator","new","delete","this","virtual","override","final",
		"try","catch","throw","nullptr","true","false",

		// MSVC / IDA sensitive
		"__int8","__int16","__int32","__int64",
		"__fastcall","__stdcall","__thiscall","__cdecl",
		"__cppobj", "_int8","_int16","_int32","_int64","__callback","__except","__try",

		"Call" // protecting our macro
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

		while (scanIndex < allClasses.size()) {
			Il2CppClass* cls = allClasses[scanIndex];
			scanIndex++;

			if (!cls) continue;

			// If this class is an Open Generic (e.g., List<T>), its methods use generic
			// placeholders (VAR) which we cannot resolve to concrete classes.
			// We only want to scan inflated instances (e.g., List<int>).

			if (Il2Cpp::ClassIsGeneric(cls)) 
				continue;

			// 1. Scan Fields (Your existing logic is generally safe here)
			void* iter = nullptr;
			while (FieldInfo* field = Il2Cpp::class_get_fields(cls, &iter)) {
				Il2CppType* fType = Il2Cpp::field_get_type(field);
				if (!fType) continue;

				//if (strcmp(Il2Cpp::field_get_name(field), "ACPGNGFFBHG") == 0)

				if (fType->type == IL2CPP_TYPE_GENERICINST ||
					fType->type == IL2CPP_TYPE_VALUETYPE ||
					fType->type == IL2CPP_TYPE_CLASS) {

					Il2CppClass* targetClass = Il2Cpp::class_from_type(fType);
					if (targetClass && uniqueSet.find(targetClass) == uniqueSet.end()) {
						uniqueSet.insert(targetClass);
						allClasses.push_back(targetClass);
					}
				}
			}

			// 2. Scan Parent (Existing logic)
			Il2CppClass* parentClass = Il2Cpp::GetClassParent(cls);
			if (parentClass && uniqueSet.find(parentClass) == uniqueSet.end()) {
				// Logic to check parent type properties if needed
				// Usually adding the parent class directly is safe and desirable
				uniqueSet.insert(parentClass);
				allClasses.push_back(parentClass);
			}

			// 3. Scan Methods (The Crash Fix)
			iter = nullptr;
			while (MethodInfo* method = Il2Cpp::class_get_methods(cls, &iter)) {
				// --- SAFEGUARD 2: Skip Generic Method Definitions ---
				// If the method itself is generic (e.g. "T GetComponent<T>()"),
				// we cannot resolve T. Skip it.
				
				if (Il2Cpp::GetMethodIsGenric(method)) continue;

				// Check Return Type
				Il2CppType* retType = Il2Cpp::method_get_return_type(method);
				if (retType && (retType->type == IL2CPP_TYPE_GENERICINST ||
					retType->type == IL2CPP_TYPE_VALUETYPE ||
					retType->type == IL2CPP_TYPE_CLASS)) {

					// Safety check for generic instance data
					if ( !retType->data.generic_class) continue;

					Il2CppClass* retClass = Il2Cpp::class_from_type(retType);
					if (retClass && uniqueSet.find(retClass) == uniqueSet.end()) {
						uniqueSet.insert(retClass);
						allClasses.push_back(retClass);
					}
				}

				// Check Parameters
				uint32_t paramCount = Il2Cpp::method_get_param_count(method);
				for (uint32_t i = 0; i < paramCount; i++) {
					Il2CppType* paramType = Il2Cpp::method_get_param(method, i);

					if (paramType &&
						(paramType->type == IL2CPP_TYPE_GENERICINST ||
							paramType->type == IL2CPP_TYPE_VALUETYPE ||
							paramType->type == IL2CPP_TYPE_CLASS)) {

						if (paramType->type == IL2CPP_TYPE_GENERICINST && !paramType->data.generic_class) continue;

						Il2CppClass* paramClass = Il2Cpp::class_from_type(paramType);
						if (paramClass && uniqueSet.find(paramClass) == uniqueSet.end()) {
							uniqueSet.insert(paramClass);
							allClasses.push_back(paramClass);
						}
					}
				}
			}
		}
	}

	std::string GetIl2CppClassName(Il2CppClass* cls);
	std::string GetCType(Il2CppType* type);

	std::string GenerateRawClassName(Il2CppClass* cls) {
		if (!cls) return "void";

		std::string name = Il2Cpp::class_get_name(cls);
		std::string ns = Il2Cpp::class_get_namespace(cls);

		std::string fullName;
		if (!ns.empty()) fullName = ns + "_" + name;
		else fullName = name;

		Il2CppGenericClass* genericClass = Il2Cpp::GetClassGenericClass(cls);

		if (genericClass) {
			fullName += "_Gen";

			Il2CppGenericContext* genericContext = Il2Cpp::GetGenericContext(genericClass);
			if (genericContext) {
				Il2CppGenericInst* inst = Il2Cpp::GenericContextGetClassInst(genericContext);
				if (inst) {
					for (uint32_t i = 0; i < inst->type_argc; i++) {
						const Il2CppType* t = inst->type_argv[i];

						Il2CppClass* argClass = Il2Cpp::class_from_type((Il2CppType*)t);
						std::string argName = argClass ? GetIl2CppClassName(argClass) : GetCType((Il2CppType*)t);

						argName = SanitizeType(argName);
						fullName += "_" + argName;
					}
				}
			}
		}
		return SanitizeType(fullName);
	}

	std::string GetIl2CppClassName(Il2CppClass* cls) {
		// Map strict pointer -> Unique Name
		static std::unordered_map<Il2CppClass*, std::string> g_ClassNames;
		// Set of names already taken to detect collisions
		static std::unordered_set<std::string> g_TakenNames;

		if (!cls) return "void";

		// 1. Check Cache: Have we already named this specific pointer?
		auto it = g_ClassNames.find(cls);
		if (it != g_ClassNames.end()) {
			return it->second;
		}

		// 2. Generate Base Name
		std::string name = GenerateRawClassName(cls);

		if (name.empty()) return "";

		// 3. Collision Resolution
		// If this exact string name is already used by a DIFFERENT pointer, suffix it.
		if (g_TakenNames.count(name)) {
			int id = 1;
			std::string newName;
			do {
				newName = name + "_" + std::to_string(id);
				id++;
			} while (g_TakenNames.count(newName)); // Ensure the suffixed name isn't taken too
			name = newName;
		}

		// 4. Register
		g_TakenNames.insert(name);
		g_ClassNames[cls] = name;

		return name;
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

				return safeName;
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
				return safeName + "*";
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

			if (Il2Cpp::GetClassType(cls)->type == IL2CPP_TYPE_VAR ||
				Il2Cpp::GetClassType(cls)->type == IL2CPP_TYPE_PTR ||
				Il2Cpp::GetClassType(cls)->type == IL2CPP_TYPE_MVAR) return;

			// Mark visited immediately to prevent infinite recursion (cycles)
			visited.insert(cls);

			// Must define Parent first (Inheritance)
			Il2CppClass* parent = Il2Cpp::GetClassParent(cls);
			if (parent) visit(parent);

			// Must define ValueType fields first (Embedded structs)
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

			if (!Il2Cpp::ClassIsGeneric(cls)) {
				iter = nullptr;
				while (MethodInfo* method = Il2Cpp::class_get_methods(cls, &iter))
				{
					if (Il2Cpp::GetMethodIsGenric(method)) continue;

					// Helper to check and visit type
					auto tryVisitType = [&](Il2CppType* t) {
						if (!t) return;
						if (t->type == IL2CPP_TYPE_VALUETYPE ||
							t->type == IL2CPP_TYPE_GENERICINST) {

							Il2CppClass* tClass = Il2Cpp::class_from_type(t);
							if (tClass) visit(tClass);
						}
						};

					tryVisitType(Il2Cpp::method_get_return_type(method));

					uint32_t paramCount = Il2Cpp::method_get_param_count(method);
					for (uint32_t i = 0; i < paramCount; i++) {
						tryVisitType(Il2Cpp::method_get_param(method, i));
					}
				}
			}

			// Add self to list after dependencies are satisfied
			sorted.push_back(cls);
			};

		// process all classes
		for (auto cls : classes) {
			visit(cls);
		}

		// Replace original list with sorted list
		classes = sorted;
	}

#include "dump_for_ida.h"
	
	void DumpFull() {
		FILE* file;
		fopen_s(&file, "il2cpp_dump.cs", "w");
		if (!file) return;

		Utils::Log("Collecting classes for full dump...\n");
		std::vector<Il2CppClass*> allClasses;
		CollectAllClasses(allClasses);

		for (Il2CppClass* cls : allClasses) {
			//Utils::Log("%d\n", __LINE__);

			std::string name = Il2Cpp::class_get_name(cls);
			std::string ns = Il2Cpp::class_get_namespace(cls);
			//Utils::Log("%d\n", __LINE__);

			Il2CppClass* parent = Il2Cpp::GetClassParent(cls);
			std::string parentStr = "";
			//Utils::Log("%d\n", __LINE__);

			if (parent) {
				parentStr = Il2Cpp::class_get_name(parent);
				fprintf(file, "// Namespace: %s\nclass %s : %s\n{\n", ns.c_str(), name.c_str(), parentStr.c_str());
			}
			else
				fprintf(file, "// Namespace: %s\nclass %s \n{\n\t// Fields \n\n", ns.c_str(), name.c_str());
			//Utils::Log("%d\n", __LINE__);

			// Fields
			void* iter = nullptr;
			while (FieldInfo* f = Il2Cpp::class_get_fields(cls, &iter)) {
				int flags = Il2Cpp::field_get_flags(f);
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
				case FIELD_ATTRIBUTE_COMPILER_CONTROLLED: modifiers += "compiler-controlled"; break;
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

				std::string typeName = Utils::StripNamespaces(Il2Cpp::GetTypeName(Il2Cpp::field_get_type(f)));
				int32_t offset = Il2Cpp::field_get_offset(f) - ((parentStr == "System_ValueType") ? sizeof(Il2CppObject) : 0);
				fprintf(file, "\t%s %s %s; // Offset: 0x%X, Flags: 0x%X\n", modifiers.c_str(),
					typeName.c_str(), Il2Cpp::field_get_name(f), offset, flags);
			}

			//Utils::Log("%d\n", __LINE__);


			// Methods
			iter = nullptr;
			fprintf(file, "\n\t// Methods\n");
			while (MethodInfo* m = Il2Cpp::class_get_methods(cls, &iter)) {
				//Utils::Log("%d\n", __LINE__);

				std::string retType = Utils::StripNamespaces(Il2Cpp::GetTypeName(Il2Cpp::method_get_return_type(m)));
				std::string params = "";
				uint8_t count = Il2Cpp::method_get_param_count(m);
				for (uint8_t p = 0; p < count; p++) {
					params += Utils::StripNamespaces(Il2Cpp::GetTypeName(Il2Cpp::method_get_param(m, p)));
					params += " ";
					params += Il2Cpp::method_get_param_name(m, p);
					if (p < count - 1) params += ", ";
				}
				//Utils::Log("%d\n", __LINE__);

				uint16_t flags = Il2Cpp::GetMethodFlags(m);
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
					//Utils::Log("%d\n", __LINE__);

					if (Il2Cpp::GetMethodSlot(m) != -1 && !isNewSlot) {
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
				//Utils::Log("%d\n", __LINE__);

				fprintf(file, "\t%s%s %s(%s); // RVA: 0x%llX\n", modifiers.c_str(), retType.c_str(), Il2Cpp::method_get_name(m), params.c_str(), Il2Cpp::GetMethodPointer(m) - Config::GameBase);
			}
			fprintf(file, "}\n\n");
		}

		Utils::Log("Done creating full dump!\n");
	}

	/*std::string GetMethodArgs(MethodInfo* method, bool onlyNames) {
		std::string args = "";
		uint32_t count = Il2Cpp::method_get_param_count(method);

		uint32_t unnamedCount = 1;

		for (uint32_t i = 0; i < count; i++) {
			Il2CppType* paramType = Il2Cpp::method_get_param(method, i);
			std::string paramName = Il2Cpp::method_get_param_name(method, i);

			if (paramName.empty()) paramName = "unnamed_" + std::to_string(unnamedCount++);

			std::string safeParamName = SanitizeIdentifier(paramName);
			if (i > 0) args += ", ";

			if (onlyNames)
				args += safeParamName;
			else
				args += GetCType(paramType) + " " + safeParamName;
		}

		if (!args.empty())
			args += ", ";

		if (onlyNames)
			args += "method_info";
		else
			args += "MethodInfo* method_info";

		return args;
	}*/

	/*
	std::string GetIl2CppMethodName(std::unordered_map<MethodInfo*, std::string>& methodNames, std::unordered_set<std::string>& takenNames, MethodInfo* method) {
		auto it = methodNames.find(method);
		if (it != methodNames.end()) {
			return it->second;
		}

		std::string name = SanitizeIdentifier(Il2Cpp::method_get_name(method));

		if (takenNames.count(name)) {
			int id = 1;
			std::string newName;
			do {
				newName = name + "_" + std::to_string(id);
				id++;
			} while (takenNames.count(newName));
			name = newName;
		}

		takenNames.insert(name);
		methodNames[method] = name;

		return name;
	}

	void GenerateSDK() {
		FILE* headerFile;
		FILE* implFile;

		fopen_s(&headerFile, "sdk.h", "w");
		fopen_s(&implFile, "sdk.cpp", "w");

		if (!headerFile || !implFile) {
			if (headerFile) fclose(headerFile);
			if (implFile) fclose(implFile);
			return;
		}

		// --- Header File ---
		fprintf(headerFile,
			R"(#pragma once
// Generated by GIRuntimeDumper
#include <cstdint>

extern uintptr_t GameBase;

struct Il2CppObject { void* klass; void* monitor; };
struct MethodInfo;

template <typename T>
struct Il2CppArray : Il2CppObject {
	void* bounds;
	int   max_length;
	T array[65535];
	T& operator [] (int i) { return array[i]; }
	const T& operator [] (int i) const { return array[i]; }
};

struct Il2CppString { Il2CppObject* obj; int32_t length; char chars[1]; };

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

#pragma warning(push)
#pragma warning(disable : 4674)
)");

		// --- Implementation File ---
		fprintf(implFile,
			R"(// Generated by GIRuntimeDumper
#include "sdk.h"

#pragma warning(push)
#pragma warning(disable : 4674)
)");

		std::vector<Il2CppClass*> allClasses;
		Utils::Log("Collecting classes...\n");
		CollectAllClasses(allClasses);
		CollectInflatedClasses(allClasses);

		Utils::Log("Sorting classes...\n");
		SortClasses(allClasses);

		// 1. Forward Declarations (Header)
		Utils::Log("Writing forward declarations...\n");
		for (Il2CppClass* cls : allClasses) {
			std::string fullName = GetIl2CppClassName(cls);
			if (!fullName.empty() && Il2Cpp::GetClassType(cls)->type != IL2CPP_TYPE_VALUETYPE)
				fprintf(headerFile, "typedef struct %s %s;\n", fullName.c_str(), fullName.c_str());
		}
		fprintf(headerFile, "\n");

		// ---------------------------------------------------------
		// PASS 1: Struct Definitions with Method Declarations (Header)
		// ---------------------------------------------------------
		Utils::Log("Writing struct definitions to header...\n");

		for (Il2CppClass* cls : allClasses) {
			std::string name = Il2Cpp::class_get_name(cls);
			std::string ns = Il2Cpp::class_get_namespace(cls);

			if (name == "" && ns == "") continue;

			std::string fullName = GetIl2CppClassName(cls);
			if (fullName.empty()) continue;

			std::string parentName = "";
			Il2CppClass* parentClass = Il2Cpp::GetClassParent(cls);
			if (parentClass)
				parentName = GetIl2CppClassName(parentClass);

			if (parentName == "System_Enum") continue;
			if (fullName == "System_ValueType") parentName = "";

			// Struct Header
			if (parentName.empty())
				fprintf(headerFile, "// Namespace: %s\nstruct %s {\n", ns.c_str(), fullName.c_str());
			else
				fprintf(headerFile, "// Namespace: %s\nstruct %s : %s {\n", ns.c_str(), fullName.c_str(), parentName.c_str());

			// Fields
			fprintf(headerFile, "    // Fields\n");
			void* iter = nullptr;
			while (FieldInfo* field = Il2Cpp::class_get_fields(cls, &iter)) {
				int flags = Il2Cpp::field_get_flags(field);
				if (flags & FIELD_ATTRIBUTE_STATIC) continue;

				std::string fName = SanitizeIdentifier(Il2Cpp::field_get_name(field));
				if (fName.empty()) continue;

				std::string typeStr = GetCType(Il2Cpp::field_get_type(field));
				int32_t offset = Il2Cpp::field_get_offset(field) - ((parentName == "System_ValueType") ? sizeof(Il2CppObject) : 0);
				fprintf(headerFile, "    %s %s; // 0x%X\n", typeStr.c_str(), fName.c_str(), offset);
			}

			std::unordered_map<MethodInfo*, std::string> methodNames;
			std::unordered_set<std::string> takenNames;

			// Method Declarations (Header)
			fprintf(headerFile, "\n    // Methods\n");
			iter = nullptr;
			while (MethodInfo* method = Il2Cpp::class_get_methods(cls, &iter)) {
				uintptr_t methodPtr = Il2Cpp::GetMethodPointer(method);
				if (!methodPtr || !(methodPtr - Config::GameBase)) continue;

				std::string mName = GetIl2CppMethodName(methodNames, takenNames, method);
				if (mName.empty()) continue;

				uint16_t flags = Il2Cpp::GetMethodFlags(method);
				bool isStatic = (flags & METHOD_ATTRIBUTE_STATIC);
				std::string retType = GetCType(Il2Cpp::method_get_return_type(method));
				std::string argsDecl = GetMethodArgs(method, false);

				fprintf(headerFile, "    %s%s %s(%s);\n",
					isStatic ? "static " : "",
					retType.c_str(),
					mName.c_str(),
					argsDecl.c_str()
				);
			}

			fprintf(headerFile, "};\n\n");
		}

		fprintf(headerFile, "#pragma warning(pop)\n");

		// ---------------------------------------------------------
		// PASS 2: Method Implementations (CPP File)
		// ---------------------------------------------------------
		Utils::Log("Writing method implementations to cpp...\n");

		for (Il2CppClass* cls : allClasses) {
			std::string fullName = GetIl2CppClassName(cls);
			if (fullName.empty()) continue;

			std::string parentName = "";
			Il2CppClass* parentClass = Il2Cpp::GetClassParent(cls);
			if (parentClass) parentName = GetIl2CppClassName(parentClass);
			if (parentName == "System_Enum") continue;

			std::unordered_map<MethodInfo*, std::string> methodNames;
			std::unordered_set<std::string> takenNames;

			void* iter = nullptr;
			while (MethodInfo* method = Il2Cpp::class_get_methods(cls, &iter)) {
				uintptr_t methodPtr = Il2Cpp::GetMethodPointer(method);
				if (!methodPtr) continue;
				uintptr_t rva = methodPtr - Config::GameBase;
				if (!rva) continue;

				std::string mName = GetIl2CppMethodName(methodNames, takenNames, method);
				if (mName.empty()) continue;

				uint16_t flags = Il2Cpp::GetMethodFlags(method);
				bool isStatic = (flags & METHOD_ATTRIBUTE_STATIC);
				Il2CppType* retIl2CppType = Il2Cpp::method_get_return_type(method);
				std::string retType = GetCType(retIl2CppType);
				std::string argsCall = GetMethodArgs(method, true);
				std::string argsDecl = GetMethodArgs(method, false);
				bool hasReturnBuffer = HasReturnBuffer(method);

				// Method Definition
				fprintf(implFile, "%s %s::%s(%s) {\n",
					retType.c_str(),
					fullName.c_str(),
					mName.c_str(),
					argsDecl.c_str()
				);

				// Body
				if (hasReturnBuffer) {
					fprintf(implFile, "    %s __ret{};\n", retType.c_str());
					if (isStatic)
						fprintf(implFile, "    CallStatic<void>(0x%llX, &__ret%s%s);\n", rva, argsCall.empty() ? "" : ", ", argsCall.c_str());
					else
						fprintf(implFile, "    Call<void>(0x%llX, &__ret, this%s%s);\n", rva, argsCall.empty() ? "" : ", ", argsCall.c_str());
					fprintf(implFile, "    return __ret;\n");
				}
				else {
					if (isStatic)
						fprintf(implFile, "    return CallStatic<%s>(0x%llX%s%s);\n", retType.c_str(), rva, argsCall.empty() ? "" : ", ", argsCall.c_str());
					else
						fprintf(implFile, "    return Call<%s>(0x%llX, this%s%s);\n", retType.c_str(), rva, argsCall.empty() ? "" : ", ", argsCall.c_str());
				}

				fprintf(implFile, "}\n\n");
			}
		}

		fprintf(implFile, "#pragma warning(pop)\n");

		fclose(headerFile);
		fclose(implFile);
		Utils::Log("SDK generation completed! (sdk.h and sdk.cpp)\n");
	}
	*/
}