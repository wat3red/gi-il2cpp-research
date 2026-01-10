#pragma once

#include "il2cpp_types.h"

#include <logger/logger.h>
#include <windows.h>
#include <cstdio>

namespace Il2Cpp {
	namespace Class {
		Il2CppClass* FromName(const char* namespaceName, const char* className);
	}
	namespace Method {
		MethodInfo* Find(Il2CppClass* klass, const char* methodName, int paramCount);
		MethodInfo* Find(const char* namespaceName, const char* className, const char* methodName, int paramCount);
		void* GetMethodPointer(MethodInfo* method);

		template<typename Ret, typename... Args>
		Ret Call(const char* namespaceName,
			const char* className,
			const char* methodName,
			int param_count,
			Args... args)
		{
			// 1. Locate Method Info
			MethodInfo* method = Find(namespaceName, className, methodName, param_count);
			if (!method) {
				Log("[Il2Cpp::Error] Could not find method: %s.%s::%s with %d params\n",
					namespaceName, className, methodName, param_count);
				return Ret{};
			}

			// 2. Locate Method Pointer
			void* methodPointer = GetMethodPointer(method);
			if (!methodPointer) {
				Log("[Il2Cpp::Error] Method pointer is null for: %s.%s::%s\n",
					namespaceName, className, methodName);
				return Ret{};
			}

			// 3. Structured Exception Handling (SEH)
			// Note: __try/__except catches hardware exceptions like Access Violations (0xC0000005)
			__try {
				Log("[Il2Cpp::Debug] Invoking %s.%s::%s at %p\n",
					namespaceName, className, methodName, methodPointer);

				typedef Ret(*MethodSnippet)(Args...);
				return reinterpret_cast<MethodSnippet>(methodPointer)(args...);
			}
			__except (EXCEPTION_EXECUTE_HANDLER) {
				// Get the exception code (e.g., Access Violation, Stack Overflow)
				unsigned int code = GetExceptionCode();

				Log("[Il2Cpp::Critical] SEH Exception 0x%08X occurred while calling %s.%s::%s\n",
					code, namespaceName, className, methodName);

				return Ret{};
			}
		}

	}
	namespace Field {
		FieldInfo* Find(Il2CppClass* klass, const char* fieldName);
	}
}