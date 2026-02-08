#pragma once
#include "../unity/unity_objects.h"
#include "molemole_types.h"

#include <vector>

namespace MoleMole
{
	struct BaseEntity {
		Unity::GameObject* GetGameObject();
		EntityType GetType();
		Il2CppString* GetName();
		Unity::Vector3 GetRelativePosition();
		Unity::Vector3 GetAbsolutePosition();
		void SetAbsolutePosition(Unity::Vector3 pos);
		Unity::Rigidbody* GetRigidbody();
		uint32_t GetRuntimeID();
	};

	struct AvatarEntity : BaseEntity {};

	struct EntityManager {
		static EntityManager* Instance();
		std::vector<BaseEntity*> GetEntities();
		AvatarEntity* GetAvatar();
		BaseEntity* GetValidEntity(uint32_t runtimeID);
	};

	struct ItemModule {
		static ItemModule* Instance();
		void PickItem(uint32_t entityID);
	};
}
