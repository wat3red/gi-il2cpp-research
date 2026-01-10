#pragma once
#include "../il2cpp/il2cpp_types.h"
#include "unity_collections.h"

namespace Unity {

	struct DelegateData : Il2CppObject {
		void* target_type;
		Il2CppString* method_name;
	};

	struct Delegate : Il2CppObject {
		void* method_ptr;
		void* invoke_impl;
		Il2CppObject* target;
	};

	struct MulticastDelegate : Delegate {
		Array<Delegate*>* delegates;
	};

	struct Action : MulticastDelegate {};
}
