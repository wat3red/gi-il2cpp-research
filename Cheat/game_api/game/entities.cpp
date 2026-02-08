#include "entities.h"
#include "../functions/resolve_funcs.h"
#include "../version_constants.h"

using namespace MoleMole;

EntityManager* EntityManager::Instance() {
	return (EntityManager*)SingletonManager::GetSingletonInstance(version_constants::beebyte::entity_manager_class);
}

std::vector<BaseEntity*> EntityManager::GetEntities() {
	Unity::List<BaseEntity*>* entityList = EntityManager_GetEntities(this);

	std::vector<BaseEntity*> vector;
	if (entityList) {
		//Log("entityList: %p, size: %d \n", entityList, entityList->size);
		for (int i = 0; i < entityList->size; i++) {
			vector.push_back(entityList->array->items[i]);
		}
	}

	return vector;
}

AvatarEntity* MoleMole::EntityManager::GetAvatar() {
	return EntityManager_GetLocalAvatarEntity(this);
}

BaseEntity* MoleMole::EntityManager::GetValidEntity(uint32_t runtimeID) {
	return EntityManager_GetValidEntity(this, runtimeID);
}

Unity::GameObject* BaseEntity::GetGameObject() {
	return BaseEntity_get_gameObject(this);
}

EntityType BaseEntity::GetType() {
	return *(MoleMole::EntityType*)((uintptr_t)this + version_constants::offsets::entity_type);
}

Il2CppString* MoleMole::BaseEntity::GetName() {
	return BaseEntity_GetName(this);
}

Unity::Vector3 MoleMole::BaseEntity::GetRelativePosition() {
	return BaseEntity_GetRelativePosition(this);
}

Unity::Vector3 MoleMole::BaseEntity::GetAbsolutePosition() {
	return BaseEntity_GetAbsolutePosition(this);
}

void MoleMole::BaseEntity::SetAbsolutePosition(Unity::Vector3 pos) {
	BaseEntity_SetAbsolutePosition(this, pos, true);
}

Unity::Rigidbody* MoleMole::BaseEntity::GetRigidbody() {
	return BaseEntity_GetRigidbody(this);
}

uint32_t MoleMole::BaseEntity::GetRuntimeID() {
	//Log("0x32C %d\n", *(uint32_t*)((uintptr_t)this + 0x32C));
	return *(uint32_t*)((uintptr_t)this + version_constants::offsets::runtime_id);
}

ItemModule* MoleMole::ItemModule::Instance() {
	return (ItemModule*)SingletonManager::GetSingletonInstance(version_constants::beebyte::item_module_class);
}

void MoleMole::ItemModule::PickItem(uint32_t entityID) {
	ItemModule_PickItem(this, entityID);
}
