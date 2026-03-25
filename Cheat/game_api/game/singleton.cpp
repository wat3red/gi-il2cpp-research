#include "singleton.h"
#include "../functions/resolve_funcs.h"
#include "../unity/unity_primitives.h"

Il2CppObject* MoleMole::SingletonManager::GetSingletonInstance(const char* typeName) {
	auto singletonManager = Il2Cpp::Method::Call<MoleMole::SingletonManager*>("MoleMole", "SingletonManager", "get_Instance", 0);
	return Il2Cpp::Method::Call<Il2CppObject*>("MoleMole", "SingletonManager", "GetSingletonInstance", 1,
		singletonManager, Il2CppString::FromCStr(typeName));
}
