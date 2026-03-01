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

//Unity::Dictionary<uint32_t, Unity::Dictionary<uint32_t, ScenePointData>*>* MapModule::GetScenePointDics() {
//	return *(Unity::Dictionary<uint32_t, Unity::Dictionary<uint32_t, ScenePointData>*>**)((uintptr_t)this + 0x50);
//}

Unity::Vector3 MoleMole::ConfigScenePoint::GetTranPos() {
	return *(Unity::Vector3*)((uintptr_t)this + 0x18);
}

uint32_t MoleMole::MapManager::GetMapSceneID() {
	return *(uint32_t*)((uintptr_t)this + 0x110);
}

MapManager* MoleMole::MapManager::Instance() {
	return (MapManager*)SingletonManager::GetSingletonInstance(version_constants::beebyte::map_manager_class);
}


UIManager* MoleMole::UIManager::Instance() {
	return (UIManager*)SingletonManager::GetSingletonInstance(version_constants::beebyte::ui_manager_class);
}

Unity::Camera* MoleMole::UIManager::GetUICamera() {
	static int32_t offset = Mem::Signature("48 8B 7E ? 48 85 FF 74 ? 48 83 7F ? 00 74 ? 80 BE ? ? ? ? 00 74 ? 48 8B 8E").FindDisp();
	return *(Unity::Camera**)((uintptr_t)this + offset);
}

MonoInLevelMapPage* MoleMole::InLevelMapPageContext::GetPageMono() {
	static int32_t offset = Mem::Signature("48 8B 8E ? ? ? ? 48 85 C9 0F 84 ? ? ? ? 48 89 DA E8 ? ? ? ? 48 89 F1").FindDisp();
	return *(MonoInLevelMapPage**)((uintptr_t)this + offset);
}

Unity::Rect MoleMole::InLevelMapPageContext::GetMapViewRect() {
	static int32_t offset = Mem::Signature("4C 8D AF ? ? ? ? BD").FindDisp();
	return *(Unity::Rect*)((uintptr_t)this + offset);
}

Unity::Transform* MoleMole::MonoInLevelMapPage::GetMapBackground() {
	return Il2Cpp::Method::Call<Unity::Transform*>("MoleMole", "MonoInLevelMapPage", "get_mapBackground", 0, this);
}

Unity::Rect MoleMole::MonoInLevelMapPage::GetMapRect() {
	return Il2Cpp::Method::Call<Unity::Rect>("MoleMole", "MonoInLevelMapPage", "get_mapRect", 0, this);
}
