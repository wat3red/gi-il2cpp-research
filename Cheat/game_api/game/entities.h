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
	};

	struct AvatarEntity : BaseEntity {

	};

	struct EntityManager {
		static EntityManager* Instance();
		std::vector<BaseEntity*> GetEntities();
		AvatarEntity* GetAvatar();
	};

}
