#include "sdk_types.h"
#include "logger.h"

extern uintptr_t g_base;

MoleMole::EntityManager* MoleMole::EntityManager::get_EntityManager() {
	return ((MoleMole::EntityManager * (*)())(g_base + 0xC427720))();
}

std::vector<MoleMole::BaseEntity*> MoleMole::EntityManager::entities() {
	// 0x100132F0 - 149
	// 0x100258B0 - 1 // team
	// 0x1002C300 - 0
	auto entityList = ((Unity::List<MoleMole::BaseEntity*>*(*)(MoleMole::EntityManager*))(g_base + 0x100132F0))(this);

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
	return ((Unity::String * (*)(MoleMole::BaseEntity*))(g_base + 0xC807F00))(this);
}

const char* Unity::String::ToCString() {
	return ((const char* (*)(Unity::String*))(g_base + 0x3F35E0))(this);
}

//const char* Unity::String::ToCString() {
//	static char buf[512];
//	int len = this->m_StringLength;
//	for (int i = 0; i < len && i < 511; i++)
//		buf[i] = (char)this->m_FirstChar[i];
//	buf[len] = 0;
//	return buf;
//}

