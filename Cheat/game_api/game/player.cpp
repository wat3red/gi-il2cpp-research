#include "player.h"
#include "../functions/resolve_funcs.h"
#include "../version_constants.h"

using namespace MoleMole;

PlayerModule* MoleMole::PlayerModule::Instance() {
	return (PlayerModule*)SingletonManager::GetSingletonInstance(version_constants::beebyte::player_module_class);
}

uint32_t MoleMole::PlayerModule::GetCurSceneID() {
	// class "private static MonoUIWaterMask ", "public UInt32 "
	return *(uint32_t*)((uintptr_t)this + 0x1F8);
}

LoadingManager* MoleMole::LoadingManager::Instance() {
	return (LoadingManager*)SingletonManager::GetSingletonInstance(version_constants::beebyte::loading_manager_class);
}

ActorManager* MoleMole::ActorManager::Instance() {
	return (ActorManager*)SingletonManager::GetSingletonInstance("MoleMole.ActorManager");
}

BaseActor* MoleMole::ActorManager::GetGlobalActor() {
	return Il2Cpp::Method::Call<BaseActor*>("MoleMole", "ActorManager", "GetGlobalActor", 0, this);
}

BaseActor* MoleMole::ActorManager::GetActor(MoleMole::BaseEntity* entity) {
	return ActorManager_GetActor(this, entity);
}

Unity::Vector3 MoleMole::ActorUtils::GetAvatarPos() {
	return Il2Cpp::Method::Call<Unity::Vector3>("MoleMole", "ActorUtils", "GetAvatarPos", 0);
}

void MoleMole::ActorUtils::SetAvatarPos(const Unity::Vector3& position) {
	Il2Cpp::Method::Call<void>("MoleMole", "ActorUtils", "SetAvatarPos", 1, position);
}

void MoleMole::ActorUtils::SyncEntityPos(MoleMole::BaseEntity* entity, int32_t state, uint32_t mainQuestId) {
	Il2Cpp::Method::Call<void>("MoleMole", "ActorUtils", "SyncEntityPos", 3, entity, state, mainQuestId);
}

ILuaActor* MoleMole::BaseActor::GetLuaActor() {
	return Il2Cpp::Method::Call<ILuaActor*>("MoleMole", "BaseActor", "get_luaActor", 0, this);
}


