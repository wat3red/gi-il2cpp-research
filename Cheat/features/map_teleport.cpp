#define IMGUI_DEFINE_MATH_OPERATORS

#include "map_teleport.h"

#include <game_api/include.h>
#include <logger/logger.h>
#include <config/imgui_config.h>
#include <config/config.h>

#include <thread>

#include <imgui/imgui.h>
#include <minhook/include/MinHook.h>
#include <imgui/imgui_internal.h>
#include <game_api/version_constants.h>

extern void* MiHoYo_SDK_Dll_instance;

namespace features
{

	void MapTeleport::DrawUI() {
		ImGuiEx::Checkbox("Enable map teleport", config.map_teleport.enabled);
	}

	void MapTeleport::DrawBackgroundUI() {}

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

		auto waypoints = ((Unity::Dictionary<uint32_t, MoleMole::ScenePointData>*(*)(MoleMole::MapModule*, uint32_t))(g_game_base + 0xF7CED50))(map_module, targetSceneId);
		Log("waypoints %p\n", waypoints);

		//Log("result_0xF7CED50 %p\n", result_0xF7CED50);

		/*auto result_0xF7C5650 = ((Unity::Dictionary<uint32_t, MoleMole::ScenePointData>*(*)(MoleMole::MapModule*, uint32_t))(g_game_base + 0xF7C5650))(map_module, targetSceneId);
		Log("result_0xF7C5650 %p\n", result_0xF7C5650);
		if (result_0xF7C5650)
			Log("result_0xF7C5650->count %d\n", result_0xF7C5650->count);

		auto result_0xF7ECA50 = ((Unity::Dictionary<uint32_t, MoleMole::ScenePointData>*(*)(MoleMole::MapModule*, uint32_t))(g_game_base + 0xF7ECA50))(map_module, targetSceneId);
		Log("result_0xF7ECA50 %p\n", result_0xF7ECA50);
		if (result_0xF7ECA50)
			Log("result_0xF7ECA50->count %d\n", result_0xF7ECA50->count);*/

			/*Unity::Dictionary<uint32_t, Unity::Dictionary<uint32_t, MoleMole::ScenePointData>*>* waypointGroups = map_module->GetScenePointDics();
			if (!waypointGroups) return result;
			Log("targetSceneId %d\n", targetSceneId);
			Log("waypointGroups %d\n", waypointGroups->count);

			for (const auto& [sceneId, waypoints] : waypointGroups->to_vector()) {
				Log("sceneId %d\n", sceneId);
				Log("waypoints %d\n", waypoints->count);

				if (sceneId != targetSceneId)
					continue;*/

		for (const auto& [waypointId, waypoint] : waypoints->to_vector()) {
			if (waypoint.config == nullptr)
				continue;

			//Log("waypointId %d\n", waypointId);
			Unity::Vector3 tran_pos = waypoint.config->GetTranPos();
			//Log("tran_pos %s\n", tran_pos.ToString().c_str());

			if (waypoint.isUnlocked && !waypoint.isGroupLimit && !waypoint.isModelHidden)
				result.push_back(WaypointInfo{ targetSceneId, waypointId, tran_pos, (MoleMole::ScenePointData*)&waypoint });
		}
		//}

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

	void TeleportTo1(Unity::Vector3 position) {
		auto loadingManager = MoleMole::LoadingManager::Instance();
		if (!loadingManager) return;

		auto player_module = MoleMole::PlayerModule::Instance();
		if (!player_module) return;

		uint32_t sceneId = player_module->GetCurSceneID();

		auto ActorManager = MoleMole::ActorManager::Instance();
		if (!ActorManager) return;

		auto list = *(Unity::List<MoleMole::BaseActor*>**)((uintptr_t)ActorManager + 0x68);
		Log("list->array->length() = %d\n", list->array->length());
		Log("list->size = %d\n", list->size);
		for (const auto& actor : list->to_vector()) Log("actor: %p, alias: %s\n", actor, (*(Il2CppString**)((uintptr_t)actor + 0xC8))->ToCStr());

		auto actor_dic = *(Unity::Dictionary<Il2CppString*, MoleMole::BaseActor*>**)((uintptr_t)ActorManager + 0x1D0);

		//Log("actor_dic->entries = %p\n", actor_dic->entries);
		//Log("actor_dic->entries->length() = %d\n", actor_dic->entries->length());

		auto vector = actor_dic->to_vector();
		Log("vector.size() = %d\n", vector.size());

		for (const auto& actor : vector)
			Log("%s : %p\n", actor.first->ToCStr(), actor.second);

		//auto GlobalActor = ActorManager->GetGlobalActor();
		//if (!GlobalActor) return;

		MoleMole::EntityManager* entity_manager = MoleMole::EntityManager::Instance();
		if (!entity_manager) return;

		MoleMole::AvatarEntity* avatar_entity = entity_manager->GetAvatar();
		Log("avatar_entity: %p\n", avatar_entity);
		Log("avatar_entity->GetName: %s\n", avatar_entity->GetName()->ToCStr());

		if (!avatar_entity) return;

		auto actor = ActorManager->GetActor(avatar_entity);
		Log("actor: %p\n", actor);
		//if (!actor) return;

		auto global_actor = ActorManager->GetGlobalActor();
		Log("global_actor: %p\n", global_actor);
		if (!global_actor) return;

		MoleMole::TransmitRequest req{};
		memset(&req, 0, sizeof(req));

		req.actor = global_actor;   // ќЅя«ј“≈Ћ№Ќќ
		req.sceneId = sceneId;            // текуща€ сцена
		req.targetPos = position;
		req.targetEuler = { 0.f, 0.f, 0.f };

		// важные флаги
		req.showBlackScreen = false;
		req.useWhiteScreen = false;
		req.disableTPAudio = true;

		// callbacks можно NULL
		req.transFinishCallback = nullptr;
		req.transPreCallback = nullptr;

		bool ok = Il2Cpp::Method::Call<bool>(
			"MoleMole",
			"BaseActor",
			"TryTransmitPlayer",
			1,
			global_actor,
			req
		);

		Log("OK - %d\n", ok);

		//auto GlobalActorLua = GlobalActor->GetLuaActor();
		//if (!GlobalActorLua) return;

		//Unity::Vector3 euler = { 0.f, 0.f, 0.f };
		//int reason = 1; // Teleport / GM

		//// public Void DIHNBGPCMGA(
		//// UInt32 DJBKJLBDLON, Vector3 ODCOBMFEIDF, Vector3 NILDECNCCBP, 
		//// PEDLGFFAGFG LCJACFBPFOA, ILuaActor MCPPINHNCEG, Action<ILuaActor> NLHEDJJFGFG, 
		//// Action<ILuaActor> OINEKHIOHKA, UInt32 JJHMAHOGNEK, UInt32 FBHAKJIECMG, 
		//// UInt32 PHFCIMKGAJA, MDHHOCCKMMH[] FOJOACGMOCK, Boolean CCDGGNEJHAJ
		//// ); // RVA: 0xD9FAF90

		//Il2Cpp::Method::Call<void>(
		//	"",
		//	version_constants::beebyte::loading_manager_class,
		//	"DIHNBGPCMGA",
		//	12,
		//	loadingManager,
		//	sceneId,
		//	position,
		//	euler,
		//	reason,
		//	GlobalActorLua,
		//	nullptr,
		//	nullptr,
		//	0, 0, 0,
		//	nullptr,
		//	false
		//);
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

	// to find "class MoleMole.BasePageContext " just search for "public virtual Void ClosePage();"
	// "class MoleMole.UIManager " found via "public UIPlatformConfig "
	// "class MoleMole.InLevelMapPageContext " found via "private MonoInLevelMapPage "
	void OnMapClicked_Internal(void* _this, Unity::Vector2 screenPos) {
		// "private Dictionary<Int32,List<Notify>> " or "private MonoBaseCanvas "
		Il2CppObject* uiManager = MoleMole::SingletonManager::GetSingletonInstance(version_constants::beebyte::ui_manager_class);
		if (!uiManager) return;

		Unity::Camera* ui_camera = *reinterpret_cast<Unity::Camera**>((uintptr_t)uiManager + version_constants::offsets::ui_camera);
		if (!ui_camera) return;

		void* mono_in_level_map_page = *reinterpret_cast<void**>((uintptr_t)_this + version_constants::offsets::mono_in_level_map_page);
		if (!mono_in_level_map_page) return;

		auto map_background = Il2Cpp::Method::Call<Unity::Transform*>("MoleMole", "MonoInLevelMapPage", "get_mapBackground", 0, mono_in_level_map_page);
		if (!map_background) return;

		Unity::Vector2 levelPos = { 0,0 };
		if (Il2Cpp::Method::Call<bool>("UnityEngine", "RectTransformUtility", "ScreenPointToLocalPointInRectangle", 4, map_background, screenPos, ui_camera, &levelPos)) {

			auto mapRect = Il2Cpp::Method::Call<Unity::Rect>("MoleMole", "MonoInLevelMapPage", "get_mapRect", 0, mono_in_level_map_page);
			Unity::Rect mapViewRect = *reinterpret_cast<Unity::Rect*>((uintptr_t)_this + version_constants::offsets::map_view_rect);

			levelPos.x = (levelPos.x - mapRect.m_XMin) / mapRect.m_Width;
			levelPos.x = (levelPos.x * mapViewRect.m_Width) + mapViewRect.m_XMin;

			levelPos.y = (levelPos.y - mapRect.m_YMin) / mapRect.m_Height;
			levelPos.y = (levelPos.y * mapViewRect.m_Height) + mapViewRect.m_YMin;

			auto worldPos = Il2Cpp::Method::Call<Unity::Vector3>("MoleMole", "Miscs", "GenWorldPos", 1, levelPos);
			auto relativePos = Il2Cpp::Method::Call<Unity::Vector3>("MoleMole", "WorldShiftManager", "GetRelativePosition", 1, worldPos);
			worldPos.y = Il2Cpp::Method::Call<float>("MoleMole", "Miscs", "CalcCurrentGroundHeight", 2, relativePos.x, relativePos.z) + 10.f;

			Log("worldPos: %s\n", worldPos.ToString().c_str());

			//TeleportTo(worldPos);

			MoleMole::ActorUtils::SetAvatarPos(worldPos);
			MoleMole::EntityManager* entityManager = MoleMole::EntityManager::Instance();
			MoleMole::ActorUtils::SyncEntityPos(entityManager->GetAvatar(), 0, 0);

			BasePageContext_ClosePage(_this);
		}
	}

	void (*InLevelMapPageContext_OnMapClicked)(void* _this, Unity::Vector2 screenPos);
	void hInLevelMapPageContext_OnMapClicked(void* _this, Unity::Vector2 screenPos) {
		OnMapClicked_Internal(_this, screenPos);

		InLevelMapPageContext_OnMapClicked(_this, screenPos);
	}

	bool IsNeedTransByServer(bool originalResult, Unity::Vector3& position) {
		if (taskInfo.currentStage != 3)
			return originalResult;

		MoleMole::EntityManager* entityManager = MoleMole::EntityManager::Instance();
		bool needServerTrans = entityManager->GetAvatar()->GetGameObject()->GetTransform()->GetPosition().Distance(taskInfo.targetPosition) > 60.0f;
		if (needServerTrans)
			Log("Stage 1. Distance is more than 60m. Performing server tp.\n");
		else
			Log("Stage 1. Distance is less than 60m. Performing fast tp.\n");

		taskInfo.currentStage--;
		return needServerTrans;
	}

	bool (*LoadingManager_NeedTransByServer)(MoleMole::LoadingManager* _this, uint32_t sceneId, Unity::Vector3 position);
	bool hLoadingManager_NeedTransByServer(MoleMole::LoadingManager* _this, uint32_t sceneId, Unity::Vector3 position) {
		auto result = LoadingManager_NeedTransByServer(_this, sceneId, position);
		return IsNeedTransByServer(result, position);
	}
	// 	public virtual Vector3 GetRelativePosition() { }

	void (*LoadingManager_PerformPlayerTransmit)(MoleMole::LoadingManager* _this, Unity::Vector3 position, int32_t someEnum, uint32_t someUint1, int32_t teleportType, uint32_t someUint2, bool someBool);
	void hLoadingManager_PerformPlayerTransmit(MoleMole::LoadingManager* _this, Unity::Vector3 position, int32_t someEnum, uint32_t someUint1, int32_t teleportType, uint32_t someUint2, bool someBool) {
		if (taskInfo.currentStage == 2) {
			Log("Stage 2. Changing loading location.\n");
			position = taskInfo.targetPosition;
			taskInfo.currentStage--;
			MoleMole::EntityManager* entityManager = MoleMole::EntityManager::Instance();
			LoadingManager_PerformPlayerTransmit(_this, position, someEnum, someUint1, teleportType, someUint2, someBool);
			BaseEntity_SetAbsolutePosition(entityManager->GetAvatar(), position, true);
			return;
		}

		LoadingManager_PerformPlayerTransmit(_this, position, someEnum, someUint1, teleportType, someUint2, someBool);
	}

	/*void (*ActorUtils_SetAvatarPos)(Unity::Vector3 pos);
	void hActorUtils_SetAvatarPos(Unity::Vector3 pos) {
		Log("hActorUtils_SetAvatarPos pos = %s\n", pos.ToString().c_str());

		if (taskInfo.currentStage == 1) {
			pos = taskInfo.targetPosition;
			Log("Finish.  Teleport to mark finished.\n");
			taskInfo.currentStage--;
		}

		ActorUtils_SetAvatarPos(pos);
	}*/

	// Last event in teleportation is avatar teleport, we just change avatar position from
	// waypoint location to teleport location.
	void (*BaseEntity_SetAbsolutePosition)(MoleMole::BaseEntity* _this, Unity::Vector3 position, bool forceSyncToRigidbody);
	void hBaseEntity_SetAbsolutePosition(MoleMole::BaseEntity* _this, Unity::Vector3 position, bool forceSyncToRigidbody) {
		MoleMole::EntityManager* entityManager = MoleMole::EntityManager::Instance();
		Log("entityManager->GetAvatar pos %s\n", entityManager->GetAvatar()->GetGameObject()->GetTransform()->GetPosition().ToString().c_str());
		Log("_this->GetName()->ToCStr() %s\n", _this->GetName()->ToCStr());
		Log("position.ToString %s\n", position.ToString().c_str());


		if (entityManager->GetAvatar() == _this) {
			Log("1\n");

			if (taskInfo.currentStage == 1) {
				Log("2\n");

				position = taskInfo.targetPosition;
				Log("Finish.  Teleport to mark finished.\n");
				taskInfo.currentStage--;
			}
		}

		BaseEntity_SetAbsolutePosition(_this, position, forceSyncToRigidbody);
	}

	//	public Void .*\(Vector3 .*, .* .*, UInt .*, UInt .*, Bool .*\)

	// private Void .*\(\)

	void MapTeleport::OnInit() {
		MH_CreateHook((LPVOID)(Mem::Signature("41 57 41 56 56 57 53 48 81 EC ? ? ? ? 44 0F 29 44 24 ? 0F 29 7C 24 ? 0F 29 74 24 ? 49 89 D6").Scan()),
			(LPVOID)hInLevelMapPageContext_OnMapClicked, (LPVOID*)&InLevelMapPageContext_OnMapClicked);

		//public bool NeedTransByServer(uint sceneId, Vector3 targetPos) { }
		//public boolean [A-Z]{11}\(uint32 [A-Z]{11}, Vector3 [A-Z]{11}\)
		// 0xD9F3190
		// Stage 1
		//MH_CreateHook((LPVOID)(Mem::Signature("56 57 55 53 48 83 EC ? 4C 89 C7 89 D3 48 89 CE 80 3D ? ? ? ? 00 48 8B 05 ? ? ? ? 0F 85 ? ? ? ? 48 8B 90").Scan()),
			//(LPVOID)hLoadingManager_NeedTransByServer, (LPVOID*)&LoadingManager_NeedTransByServer);

		//	private void PerformPlayerTransmit(Vector3 targetPos, Proto.EnterType enterType, uint token, MoleMole.EvtTransmitAvatar.MoleMole.EvtTransmitAvatar/TransmitType transType = 0, uint enterReason = 0) { }
		// 	private Void [A-Z]{11}\(Vector3 [A-Z]{11}, [A-Z]{11} [A-Z]{11}, UInt32 [A-Z]{11}, [A-Z]{11} [A-Z]{11}, UInt32 [A-Z]{11}, Boolean [A-Z]{11}\);
		// 0xD9FC660
		// Stage 2
		//MH_CreateHook((LPVOID)(Mem::Signature("41 57 41 56 41 54 56 57 55 53 48 81 EC ? ? ? ? 45 89 CC 45 89 C6").Scan()),
			//(LPVOID)hLoadingManager_PerformPlayerTransmit, (LPVOID*)&LoadingManager_PerformPlayerTransmit);

		// public void SetAbsolutePosition(Vector3 abpos, bool forceSyncToRigidbody = False) { }
		// public void [A-Z]{11}\(Vector3 [A-Z]{11}, boolean [A-Z]{11}\)
		// in class that can be found by "public static List<RuntimePlatform>"
		// Stage 3
		//MH_CreateHook((LPVOID)(Mem::Signature("56 57 53 48 81 EC ? ? ? ? 44 0F 29 BC 24 ? ? ? ? 44 0F 29 B4 24 ? ? ? ? 44 0F 29 AC 24 ? ? ? ? 44 0F 29 A4 24 ? ? ? ? 44 0F 29 9C 24 ? ? ? ? 44 0F 29 94 24 ? ? ? ? 44 0F 29 8C 24 ? ? ? ? 44 0F 29 84 24 ? ? ? ? 0F 29 BC 24 ? ? ? ? 0F 29 B4 24 ? ? ? ? 44 89 C3 48 89 D7 48 89 CE 80 3D ? ? ? ? 00 0F 85").Scan()),
			//(LPVOID)hBaseEntity_SetAbsolutePosition, (LPVOID*)&BaseEntity_SetAbsolutePosition);

		//MH_CreateHook((LPVOID)(Mem::Signature("56 48 83 EC ? 48 89 CE 80 3D ? ? ? ? 00 48 8B 0D ? ? ? ? 75 ? 48 8B 81 ? ? ? ? 48 85 C0 74 ? 80 3D ? ? ? ? 00 75 ? 8B 90 ? ? ? ? 48 89 C1 E8 ? ? ? ? 48 85 C0 74 ? 8B 4E").Scan()),
			//(LPVOID)hActorUtils_SetAvatarPos, (LPVOID*)&ActorUtils_SetAvatarPos);
	}

	void MapTeleport::OnUpdate() {
		if (taskInfo.waitingThread) {
			taskInfo.waitingThread = false;
			auto loading_manager = MoleMole::LoadingManager::Instance();
			LoadingManager_RequestSceneTransToPoint(loading_manager, taskInfo.sceneId, taskInfo.waypointId, nullptr);
		}
	}
}