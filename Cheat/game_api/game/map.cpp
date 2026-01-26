#include "map.h"
#include "../functions/resolve_funcs.h"
#include "../version_constants.h"

using namespace MoleMole;

MapModule* MapModule::Instance() {
	auto a = SingletonManager::GetSingletonInstance(version_constants::beebyte::map_module_class);
	Log("MapModule::Instance : %p\n", a);
	Log("il2cpp_class_get_name(a->klass) : %s\n", il2cpp_class_get_name(a->klass));

	return (MapModule*)SingletonManager::GetSingletonInstance(version_constants::beebyte::map_module_class);
}

Unity::Dictionary<uint32_t, Unity::Dictionary<uint32_t, ScenePointData>*>* MapModule::GetScenePointDics() {
	return *(Unity::Dictionary<uint32_t, Unity::Dictionary<uint32_t, ScenePointData>*>**)((uintptr_t)this + 0x50);
}

Unity::Vector3 MoleMole::ConfigScenePoint::GetTranPos() {
	return *(Unity::Vector3*)((uintptr_t)this + 0x18); // 0x28, 0x38, 0x4C
}

MapManager* MoleMole::MapManager::Instance() {
	return (MapManager*)SingletonManager::GetSingletonInstance(version_constants::beebyte::map_manager_class);
}

uint32_t MoleMole::MapManager::GetMapSceneID() {
	return *(uint32_t*)((uintptr_t)this + 0x110); // 0xE8
}
