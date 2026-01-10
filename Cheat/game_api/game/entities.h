#pragma once
#include "../unity/unity_objects.h"
#include "molemole_types.h"

#include <vector>

namespace MoleMole {

    class BaseEntity {
    public:
        Unity::GameObject* GetGameObject();
        EntityType GetType();
        Il2CppString* GetName();
    };

    class EntityManager {
    public:
        static EntityManager* Instance();
        std::vector<BaseEntity*> GetEntities();
    };

}
