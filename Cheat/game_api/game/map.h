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

	struct MapModule {
		static MapModule* Instance();
		//Unity::Dictionary<uint32_t, Unity::Dictionary<uint32_t, ScenePointData>*>* GetScenePointDics();
	};

	struct MapManager {
		static MapManager* Instance();
		uint32_t GetMapSceneID();
	};

	struct UIManager {
		static UIManager* Instance();
		Unity::Camera* GetUICamera();
	};

	struct MonoInLevelMapPage {
		Unity::Transform* GetMapBackground();
		Unity::Rect GetMapRect();
	};

	struct InLevelMapPageContext {
		MonoInLevelMapPage* GetPageMono();
		Unity::Rect GetMapViewRect();
	};
}
