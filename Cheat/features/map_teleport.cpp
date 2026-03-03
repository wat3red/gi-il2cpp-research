#include "map_teleport.h"

#include <thread>

namespace features
{
	struct WaypointInfo {
		uint32_t sceneId = 0;
		uint32_t waypointId = 0;
		Unity::Vector3 position = {};
		MoleMole::ScenePointData* data = nullptr;
	};
	struct TeleportTaskInfo {
		bool waitingThread = false;
		int currentStage = 0;
		Unity::Vector3 targetPosition = {};
		uint32_t sceneId = 0;
		uint32_t waypointId = 0;
	};
	TeleportTaskInfo taskInfo;
	std::vector<WaypointInfo> GetUnlockedWaypoints(uint32_t targetSceneId) {
		auto result = std::vector<WaypointInfo>();

		MoleMole::MapModule* map_module = MoleMole::MapModule::Instance();
		if (!map_module) return result;

		Unity::Dictionary<uint32_t, MoleMole::ScenePointData>* waypoints = map_module->GetScenePointDics(targetSceneId);
		Log("waypoints %p\n", waypoints);

		for (const auto& [waypointId, waypoint] : waypoints->to_vector()) {
			if (waypoint.config == nullptr)
				continue;

			/*Log("waypoint.config + 0x20 = %s\n", (*(Unity::Vector3*)((uintptr_t)waypoint.config + 0x20)).ToString().c_str());
			Log("waypoint.config + 0x2C = %s\n", (*(Unity::Vector3*)((uintptr_t)waypoint.config + 0x2C)).ToString().c_str());
			Log("waypoint.config + 0x40 = %s\n", (*(Unity::Vector3*)((uintptr_t)waypoint.config + 0x40)).ToString().c_str());
			Log("waypoint.config + 0x50 = %s\n", (*(Unity::Vector3*)((uintptr_t)waypoint.config + 0x50)).ToString().c_str());*/

			Unity::Vector3 tran_pos = waypoint.config->GetTranPos();

			if (waypoint.isUnlocked && !waypoint.isGroupLimit && !waypoint.isModelHidden)
				result.push_back(WaypointInfo{ targetSceneId, waypointId, tran_pos, (MoleMole::ScenePointData*)&waypoint });
		}

		return result;
	}
	WaypointInfo FindNearestWaypoint(Unity::Vector3& position, uint32_t targetSceneId) {
		float minDistance = -1;
		WaypointInfo result{};

		for (const auto& info : GetUnlockedWaypoints(targetSceneId)) {
			float distance = position.Distance(info.position);
			if (minDistance < 0 || distance < minDistance) {
				minDistance = distance;
				result = info;
			}
		}
		return result;
	}
	void TeleportTo(Unity::Vector3 position) {
		MoleMole::PlayerModule* player_module = MoleMole::PlayerModule::Instance();
		if (!player_module) return;

		MoleMole::MapManager* map_manager = MoleMole::MapManager::Instance();
		if (!map_manager) return;

		//Log("targetSceneId = %d\n", targetSceneId);

		Unity::Vector3 avatarPosition = MoleMole::ActorUtils::GetAvatarPos();
		Log("avatarPosition = %s\n", avatarPosition.ToString().c_str());

		WaypointInfo nearestWaypoint = FindNearestWaypoint(avatarPosition, map_manager->GetMapSceneID());

		Log("nearestWaypoint.position = %s, nearestWaypoint.waypointId = %d\n", nearestWaypoint.position.ToString().c_str(), nearestWaypoint.waypointId);

		taskInfo = { true, 3, position, nearestWaypoint.sceneId, nearestWaypoint.waypointId };
	}
	bool IsNeedTransByServer(bool originalResult, Unity::Vector3& position) {
		if (taskInfo.currentStage != 3)
			return originalResult;

		MoleMole::EntityManager* entityManager = MoleMole::EntityManager::Instance();
		bool needServerTrans = entityManager->GetAvatar()->GetRelativePosition().Distance(taskInfo.targetPosition) > 60.0f;
		if (needServerTrans)
			Log("Stage 1. Distance is more than 60m. Performing server tp.\n");
		else
			Log("Stage 1. Distance is less than 60m. Performing fast tp.\n");

		taskInfo.currentStage--;
		return needServerTrans;
	}
	bool (*LoadingManager_NeedTransByServer)(MoleMole::LoadingManager* _this, uint32_t sceneId, Unity::Vector3 position);
	bool hLoadingManager_NeedTransByServer(MoleMole::LoadingManager* _this, uint32_t sceneId, Unity::Vector3 position) {
		//auto result = LoadingManager_NeedTransByServer(_this, sceneId, position);
		//return IsNeedTransByServer(result, position);
		return false;
	}

	void (*LoadingManager_PerformPlayerTransmit)(MoleMole::LoadingManager* _this, Unity::Vector3 position, int32_t someEnum, uint32_t someUint1, int32_t teleportType, uint32_t someUint2, bool someBool);
	void hLoadingManager_PerformPlayerTransmit(MoleMole::LoadingManager* _this, Unity::Vector3 position, int32_t someEnum, uint32_t someUint1, int32_t teleportType, uint32_t someUint2, bool someBool) {
		Log("hLoadingManager_PerformPlayerTransmit, taskInfo.currentStage=%d\n", taskInfo.currentStage);

		if (taskInfo.currentStage == 2) {
			Log("Stage 2. Changing loading location.\n");
			position = taskInfo.targetPosition;
			taskInfo.currentStage--;
			MoleMole::EntityManager* entityManager = MoleMole::EntityManager::Instance();
			LoadingManager_PerformPlayerTransmit(_this, position, someEnum, someUint1, teleportType, someUint2, someBool);

			MoleMole::ActorUtils::SetAvatarPos(position);
			MoleMole::ActorUtils::SyncEntityPos(entityManager->GetAvatar(), 0, 0);
			return;
		}

		LoadingManager_PerformPlayerTransmit(_this, position, someEnum, someUint1, teleportType, someUint2, someBool);
	}









	void MapTeleport::DrawUI() {
		ImGuiEx::Checkbox("Enable map teleport", config.map_teleport.enabled);
	}

	MoleMole::InLevelMapPageContext* last_map_context = nullptr;

	struct PendingTeleport {
		Unity::Vector3 targetPos;
		int retryCount;
		bool active;
	};

	static PendingTeleport pendingTeleport = {};

	// to find "class MoleMole.BasePageContext " just search for "public virtual Void ClosePage();"
	// "class MoleMole.UIManager " found via "public UIPlatformConfig "
	// "class MoleMole.InLevelMapPageContext " found via "private MonoInLevelMapPage "
	void OnMapClicked_Internal(MoleMole::InLevelMapPageContext* _this, Unity::Vector2 screenPos) {
		// "private Dictionary<Int32,List<Notify>> " or "private MonoBaseCanvas "
		MoleMole::UIManager* ui_manager = MoleMole::UIManager::Instance();
		if (!ui_manager) return;

		Unity::Camera* ui_camera = ui_manager->GetUICamera();
		if (!ui_camera) return;

		MoleMole::MonoInLevelMapPage* page_mono = _this->GetPageMono();
		if (!page_mono) return;

		Unity::Transform* map_background = page_mono->GetMapBackground();
		if (!map_background) return;

		Unity::Vector2 levelPos = { 0,0 };
		if (Il2Cpp::Method::Call<bool>("UnityEngine", "RectTransformUtility", "ScreenPointToLocalPointInRectangle", 4, map_background, screenPos, ui_camera, &levelPos)) {
			Unity::Rect mapRect = page_mono->GetMapRect();
			Unity::Rect mapViewRect = _this->GetMapViewRect();

			levelPos.x = (levelPos.x - mapRect.m_XMin) / mapRect.m_Width;
			levelPos.x = (levelPos.x * mapViewRect.m_Width) + mapViewRect.m_XMin;

			levelPos.y = (levelPos.y - mapRect.m_YMin) / mapRect.m_Height;
			levelPos.y = (levelPos.y * mapViewRect.m_Height) + mapViewRect.m_YMin;

			Unity::Vector3 worldPos = Il2Cpp::Method::Call<Unity::Vector3>("MoleMole", "Miscs", "GenWorldPos", 1, levelPos);
			Log("worldPos 1 %s\n", worldPos.ToString().c_str());



			Unity::Vector3 relativePos = Il2Cpp::Method::Call<Unity::Vector3>("MoleMole", "WorldShiftManager", "GetRelativePosition", 1, worldPos);
			Log("relativePos 2 %s\n", relativePos.ToString().c_str());

			//worldPos.y = Il2Cpp::Method::Call<float>("MoleMole", "Miscs", "CalcCurrentGroundHeight", 2, relativePos.x, relativePos.z) + 5.f;
			
			float groundY = Il2Cpp::Method::Call<float>(
				"MoleMole", "Miscs", "CalcCurrentGroundHeight", 2,
				relativePos.x, relativePos.z);

			if (groundY < -99000000.0f) {
				// Территория не загружена — телепортируем с текущей Y игрока,
				// чтобы движок начал загружать чанки в точке назначения
				Unity::Vector3 avatarPos = MoleMole::ActorUtils::GetAvatarPos();
				worldPos.y = avatarPos.y;

				// Запоминаем цель для корректировки после загрузки
				pendingTeleport = { worldPos, 0, true };

				MoleMole::ActorUtils::SetAvatarPos(worldPos);
				MoleMole::EntityManager* em = MoleMole::EntityManager::Instance();
				MoleMole::ActorUtils::SyncEntityPos(em->GetAvatar(), 0, 0);
			}
			else {
				worldPos.y = groundY + 5.0f;
				MoleMole::ActorUtils::SetAvatarPos(worldPos);
				MoleMole::EntityManager* em = MoleMole::EntityManager::Instance();
				MoleMole::ActorUtils::SyncEntityPos(em->GetAvatar(), 0, 0);
			}


			Log("worldPos 2 %s\n", worldPos.ToString().c_str());

			//TeleportTo(worldPos);

			MoleMole::ActorUtils::SetAvatarPos(worldPos);
			MoleMole::EntityManager* entityManager = MoleMole::EntityManager::Instance();
			MoleMole::ActorUtils::SyncEntityPos(entityManager->GetAvatar(), 0, 0);
		}
	}

	void (*InLevelMapPageContext_OnMapClicked)(MoleMole::InLevelMapPageContext* _this, Unity::Vector2 screenPos);
	void hInLevelMapPageContext_OnMapClicked(MoleMole::InLevelMapPageContext* _this, Unity::Vector2 screenPos) {
		if (config.map_teleport.enabled) {
			last_map_context = _this;
			OnMapClicked_Internal(_this, screenPos);
			return;
		}

		InLevelMapPageContext_OnMapClicked(_this, screenPos);
	}

	void MapTeleport::OnUpdate() {
		static bool last_enabled = false;

		if (last_enabled && !config.map_teleport.enabled) {
			if (last_map_context)
				BasePageContext_ClosePage(last_map_context);
		}

		last_enabled = config.map_teleport.enabled;

		if (taskInfo.waitingThread) {
			Log("taskInfo.waitingThread\n");
			taskInfo.waitingThread = false;
			auto loading_manager = MoleMole::LoadingManager::Instance();
			LoadingManager_RequestSceneTransToPoint(loading_manager, taskInfo.sceneId, taskInfo.waypointId, nullptr);
		}
	}


	void (*SetPos)(void* _this, Unity::Vector3 targetPos, Unity::Vector3 lookAtPos, void* action);
	void hSetPos(void* _this, Unity::Vector3 targetPos, Unity::Vector3 lookAtPos, void* action) {
		if (taskInfo.waitingThread || config.map_teleport.enabled) {
			// Подменяем координаты прямо перед выполнением перемещения
			targetPos.x = taskInfo.targetPosition.x;
			targetPos.y = taskInfo.targetPosition.y;
			targetPos.z = taskInfo.targetPosition.z;

			Log("Intercepted: Position modified to %f, %f, %f\n", targetPos.x, targetPos.y, targetPos.z);

			// Сбрасываем флаги, чтобы не зациклиться
			taskInfo.waitingThread = false;
		}
		SetPos(_this, targetPos, lookAtPos, action);
	}

	typedef MoleMole::ScenePointData* (*PJEKOIAKMJM__OHDMJANMLLP_t)(MoleMole::ScenePointData*, MoleMole::LoadingManager*, unsigned int, unsigned int);
	PJEKOIAKMJM__OHDMJANMLLP_t Original = (PJEKOIAKMJM__OHDMJANMLLP_t)0;

	MoleMole::ScenePointData* My_PJEKOIAKMJM__OHDMJANMLLP(MoleMole::ScenePointData* retstr, MoleMole::LoadingManager* _this, unsigned int sceneId, unsigned int pointId) {
		// Вызываем оригинал, чтобы получить исходные данные (включая OKGFCEMNLNP)
		MoleMole::ScenePointData result;
		Original(&result, _this, sceneId, pointId);

		// Подменяем позицию, если нужно
		if (sceneId == taskInfo.sceneId) {
			result.config->GetTranPos() = taskInfo.targetPosition;
		}

		// Копируем результат в выходной параметр (как это делает оригинал)
		*retstr = result;
		return retstr;
	}

	void MapTeleport::OnInit() {
		//MH_CreateHook((LPVOID)(g_game_base + 0x10C09490),
		//	(LPVOID)My_PJEKOIAKMJM__OHDMJANMLLP, (LPVOID*)&Original);

		MH_CreateHook((LPVOID)(Mem::Signature("41 57 41 56 56 57 53 48 81 EC ? ? ? ? 44 0F 29 44 24 ? 0F 29 7C 24 ? 0F 29 74 24 ? 49 89 D6").Scan()),
			(LPVOID)hInLevelMapPageContext_OnMapClicked, (LPVOID*)&InLevelMapPageContext_OnMapClicked);

		//MH_CreateHook((LPVOID)(g_game_base + 0x8CEDA40),
		//	(LPVOID)hSetPos, (LPVOID*)&SetPos);


		/*MH_CreateHook((LPVOID)(Mem::Signature("41 57 41 56 41 54 56 57 55 53 48 81 EC ? ? ? ? 45 89 CC 45 89 C6").Scan()),
			(LPVOID)hLoadingManager_PerformPlayerTransmit, (LPVOID*)&LoadingManager_PerformPlayerTransmit);*/
		
			//MH_CreateHook((LPVOID)(Mem::Signature("56 57 55 53 48 83 EC ? 4C 89 C7 89 D3 48 89 CE 80 3D ? ? ? ? 00 48 8B 05 ? ? ? ? 0F 85 ? ? ? ? 48 8B 90").Scan()),
		//	(LPVOID)hLoadingManager_NeedTransByServer, (LPVOID*)&LoadingManager_NeedTransByServer);

		//todo: hook InLevelMapPageContext_OnMarkClicked
		//private void [A-Z]{11}\(MonoMapMark IDLDOMJBBEK)
	}

	void MapTeleport::UpdateHotkeys() {
		config.map_teleport.enabled =
			ImGui::IsKeyDown(config.map_teleport.enable_hotkey);
	}
}