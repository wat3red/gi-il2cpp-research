#include "sdk_types.h"
#include "logger.h"

extern uintptr_t g_Base;

MoleMole::EntityManager* MoleMole::EntityManager::get_EntityManager() {
	return ((MoleMole::EntityManager * (*)())(g_Base + 0xC427720))();
}

std::vector<MoleMole::BaseEntity*> MoleMole::EntityManager::entities() {
	// 0x100258B0 - 1 (team entities)
	// 0x1002C300 - 0 (idk)
	auto entityList = ((Unity::List<MoleMole::BaseEntity*>*(*)(MoleMole::EntityManager*))(g_Base + 0x100132F0))(this);

	std::vector<BaseEntity*> vector;
	if (entityList) {
		Log("entityList: %p, size: %d \n", entityList, entityList->size);

		for (int i = 0; i < entityList->size; i++) {
			vector.push_back(entityList->items->array[i]);
		}
	}

	return vector;
}

Unity::String* MoleMole::BaseEntity::name() {
	//return *(Unity::String**)((uintptr_t)this + 0xA8); // get alias
	return ((Unity::String * (*)(MoleMole::BaseEntity*))(g_Base + 0xC807F00))(this);
}

const char* Unity::String::ToCString() {
	return ((const char* (*)(Unity::String*))(g_Base + 0x3F35E0))(this);
}
