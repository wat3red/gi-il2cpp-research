#include "proto.h"

uint32_t* Proto::AvatarInfo::ConfigID() {
	static int32_t offset = Mem::Signature("8B 96 ? ? ? ? 85 D2 0F 85 ? ? ? ? 31 DB").FindDisp();
	return (uint32_t*)((uintptr_t)this + offset);
}

uint32_t* Proto::AvatarInfo::CostumeID() {
	static int32_t offset = Mem::Signature("41 89 84 24 ? ? ? ? E9 ? ? ? ? 49 8B 9C 24").FindDisp();
	return (uint32_t*)((uintptr_t)this + offset);
}

uint32_t* Proto::AvatarInfo::FlycloakID() {
	static int32_t offset = Mem::Signature("41 89 84 24 ? ? ? ? 49 8B 4C 24 ? 48 8B 56 ? 4C 8B 0D ? ? ? ? 4D 89 E0 E8 ? ? ? ? 8B 86").FindDisp();
	return (uint32_t*)((uintptr_t)this + offset);
}

Protobuf::RepeatedMessageField<Proto::AvatarInfo*>* Proto::AvatarDataNotify::GetAvatarList() {
	static int32_t offset = Mem::Signature("48 8B 4E ? 48 85 C9 0F 84 ? ? ? ? 89 C7 48 8B 05 ? ? ? ? 48 8B 90 ? ? ? ? 4C 8B 05 ? ? ? ? E8 ? ? ? ? 48 8B 4E ? 48 85 C9 0F 84 ? ? ? ? 48 8B 15 ? ? ? ? 48 8B 92 ? ? ? ? 44 01 F3 01 EB 01 FB 01 C3 45 31 C0").FindDisp();
	return *(Protobuf::RepeatedMessageField<Proto::AvatarInfo*>**)((uintptr_t)this + offset);
}

Il2CppObject* Proto::SceneEntityInfo::Entity() {
	static int32_t offset = Mem::Signature("8B 56 ? 48 89 F9 E8 ? ? ? ? 8B 46 ? F3 48 0F 2A D8").FindDisp();
	return *(Il2CppObject**)((uintptr_t)this + offset);
}

uint32_t* Proto::SceneAvatarInfo::ConfigID() {
	static int32_t offset = Mem::Signature("41 8B 96 ? ? ? ? 85 D2 75 ? EB").FindDisp();
	return (uint32_t*)((uintptr_t)this + offset);
}

uint32_t* Proto::SceneAvatarInfo::CostumeID() {
	static int32_t offset = Mem::Signature("41 89 87 ? ? ? ? E9 ? ? ? ? 49 8B 7F ? 48 8B 0D ? ? ? ? 80 B9 ? ? ? ? 00 0F 84 ? ? ? ? 48 85 FF 0F 84 ? ? ? ? 48 8B 05 ? ? ? ? 4C 8B 80 ? ? ? ? 4C 8B 0D ? ? ? ? EB").FindDisp();
	return (uint32_t*)((uintptr_t)this + offset);
}

uint32_t* Proto::SceneAvatarInfo::FlycloakID() {
	static int32_t offset = Mem::Signature("48 C7 86 ? ? ? ? 00 00 00 00 C7 86 ? ? ? ? 00 00 00 00 48 C7 86 ? ? ? ? 00 00 00 00 48 83 C4 ? 5E C3 E8").FindDisp();
	return (uint32_t*)((uintptr_t)this + offset);
}
