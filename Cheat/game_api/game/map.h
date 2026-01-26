#pragma once

#include "../internal_include.h"

namespace MoleMole
{
	struct ConfigScenePoint {
		Unity::Vector3 GetTranPos();
	};

	struct ScenePointData {
		bool isUnlocked;
		ConfigScenePoint* config;
		bool isGroupLimit;
		bool isModelHidden;
		uint32_t entityId;
		uint32_t level;
	};
	static_assert(offsetof(ScenePointData, level) == 0x18, "asd");

	struct MapModule {
		static MapModule* Instance();
		Unity::Dictionary<uint32_t, Unity::Dictionary<uint32_t, ScenePointData>*>* GetScenePointDics();
	};

	struct MapManager {
		static MapManager* Instance();
		uint32_t GetMapSceneID();
	};
}
