#include "map.h"
#include "../functions/resolve_funcs.h"
#include "../version_constants.h"

using namespace MoleMole;

MapModule* MapModule::Instance() {
	//auto a = SingletonManager::GetSingletonInstance(version_constants::beebyte::map_module_class);
	//Log("MapModule::Instance : %p\n", a);
	//Log("il2cpp_class_get_name(a->klass) : %s\n", il2cpp_class_get_name(a->klass));
	return (MapModule*)SingletonManager::GetSingletonInstance(version_constants::beebyte::map_module_class);
}

Unity::Dictionary<uint32_t, ScenePointData>* MoleMole::MapModule::GetScenePointDics(uint32_t sceneID)
{
	return MapModule_GetScenePointDic(this, sceneID);
}

//Unity::Dictionary<uint32_t, Unity::Dictionary<uint32_t, ScenePointData>*>* MapModule::GetScenePointDics() {
//	return *(Unity::Dictionary<uint32_t, Unity::Dictionary<uint32_t, ScenePointData>*>**)((uintptr_t)this + 0x50);
//}

Unity::Vector3& MoleMole::ConfigScenePoint::GetTranPos() {
	static int32_t offset = Mem::Signature("49 8D 57 ? 41 F7 C4 ? ? ? ? 0F 84 ? ? ? ? C7 42 ? 00 00 00 00").FindDisp();
	return *(Unity::Vector3*)((uintptr_t)this + offset);
}

uint32_t MoleMole::MapManager::GetMapSceneID() {
	static int32_t offset = Mem::Signature("3B 88 ? ? ? ? 0F 94 C0 48 83 C4 ? C3 31 C0 48 83 C4 ? C3 48 89 CA 48 8B 80 ? ? ? ? 48 8B 88 ? ? ? ? 48 85 C9 74").FindDisp();
	return *(uint32_t*)((uintptr_t)this + offset);
}

MapManager* MoleMole::MapManager::Instance() {
	return (MapManager*)SingletonManager::GetSingletonInstance(version_constants::beebyte::map_manager_class);
}


UIManager* MoleMole::UIManager::Instance() {
	return (UIManager*)SingletonManager::GetSingletonInstance(version_constants::beebyte::ui_manager_class);
}

Unity::Camera* MoleMole::UIManager::GetUICamera() {
	static int32_t offset = Mem::Signature("49 8B B7 ? ? ? ? 48 85 F6 74 ? 48 83 7E ? 00 74 ? 48 8B 05").FindDisp();
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
