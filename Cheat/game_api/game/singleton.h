#pragma once
#include "../unity/unity_objects.h"
#include "molemole_types.h"

namespace MoleMole {

	struct SingletonManager {
		static Il2CppObject* GetSingletonInstance(const char* typeName);
	};

}
