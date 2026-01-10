#include "types.h"
#include "../logger.h"
#include "functions/resolve_funcs.h"

extern uintptr_t g_game_base;

MoleMole::EntityManager* MoleMole::EntityManager::GetEntityManager() {
	//return ((MoleMole::EntityManager * (*)())(g_game_base + 0xC427720))();
	return InLevelDrumPageContext_get_ENTITY();
}

std::vector<MoleMole::BaseEntity*> MoleMole::EntityManager::GetEntities() {
	//auto entityList = ((Unity::List<MoleMole::BaseEntity*>*(*)(MoleMole::EntityManager*))(g_game_base + 0x100132F0))(this);
	Unity::List<MoleMole::BaseEntity*>* entityList = EntityManager_GetEntities(this);

	std::vector<BaseEntity*> vector;
	if (entityList) {
		//Log("entityList: %p, size: %d \n", entityList, entityList->size);
		for (int i = 0; i < entityList->size; i++) {
			vector.push_back(entityList->items->array[i]);
		}
	}

	return vector;
}

Unity::GameObject* MoleMole::BaseEntity::GetGameObject() {
	//return ((Unity::GameObject * (*)(MoleMole::BaseEntity*))(g_game_base + 0xC7F3490))(this);
	return BaseEntity_get_gameObject(this);
}

MoleMole::EntityType MoleMole::BaseEntity::GetType() {
	return *(MoleMole::EntityType*)((uintptr_t)this + 0x438); // DGHCHNBGOPB
}

Unity::String* MoleMole::BaseEntity::GetName() {
	//return *(Unity::String**)((uintptr_t)this + 0xA8); // get alias
	//return ((Unity::String * (*)(MoleMole::BaseEntity*))(g_game_base + 0xC807F00))(this);
	return BaseEntity_GetName(this);
}

Unity::String* Unity::String::FromCString(const char* c_str)
{
	return String_CreateString((char*)c_str);
}

const char* Unity::String::c_str() {
	//return ((const char* (*)(Unity::String*))(g_game_base + 0x3F35E0))(this);
	return Marshal_StringToHGlobalAnsi(this);
}

Unity::Transform* Unity::GameObject::GetTransform() {
	//return ((Unity::Transform * (*)(Unity::GameObject*))(g_game_base + 0x1025420))(this);
	return GameObject_get_transform(this);
}

Unity::Vector3 Unity::Transform::GetPosition() {
	Unity::Vector3 returnValue;
	//((Unity::Vector3(*)(Unity::Vector3*, Unity::Transform*))(g_game_base + 0x14CA9CB0))(&returnValue, this);
	Transform_get_position(&returnValue, this);
	return returnValue;
}

//Unity::Camera* Unity::Camera::get_current() {
//	return Camera_get_current();
//}

Unity::Camera* Unity::Camera::GetMain() {
	return Camera_get_main();
}

Unity::Vector3 Unity::Camera::WorldToScreenPoint(Vector3 position) {
	return Camera_WorldToScreenPoint(this, position);
}

Unity::Vector3 Unity::Camera::WorldToViewportPoint(Vector3 position) {
	return Camera_WorldToViewportPoint(this, position);
}

Unity::Transform* Unity::Component::GetTransform() {
	return Component_get_transform(this);
}

Il2CppObject* MoleMole::SingletonManager::GetSingletonInstance(const char* typeName)
{
	return SingletonManager_GetSingletonInstance(SingletonManager_get_Instance(), Unity::String::FromCString(typeName));
}
