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

Unity::GameObject* BaseEntity::GetGameObject() {
	return BaseEntity_get_gameObject(this);
}

EntityType BaseEntity::GetType() {
	// FJIKAJMIJLC
	// 6.1
	// return *(MoleMole::EntityType*)((uintptr_t)this + 0x438); // DGHCHNBGOPB 
	return *(MoleMole::EntityType*)((uintptr_t)this + 0x470);
}

Il2CppString* MoleMole::BaseEntity::GetName() {
	return BaseEntity_GetName(this);
}

